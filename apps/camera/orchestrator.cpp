#include "camera_options.hpp"
#include "process_group.hpp"
#include "../version.hpp"
#include <cerrno>
#include <atomic>
#include <csignal>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {
volatile sig_atomic_t stop_requested = 0;
void stop(int) { stop_requested = 1; }
struct Gpio417 {
    int fd = -1; bool exported = false;
    std::atomic<bool> stopping{false};
    std::atomic<int> failure{0};
    std::atomic<uint64_t> pulses{0};
    std::thread worker;
    int open_gpio() {
        const std::filesystem::path path("/sys/class/gpio/gpio417");
        if (std::filesystem::exists(path)) { errno = EBUSY; return -1; }
        { std::ofstream out("/sys/class/gpio/export"); if (!out) return -1; out << "417"; if (!out) return -1; }
        exported = true;
        for (unsigned attempt = 0; attempt < 100 && !std::filesystem::exists(path); ++attempt)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        { std::ofstream out(path / "direction"); if (!out) return -1; out << "low"; if (!out) return -1; }
        fd = ::open((path / "value").c_str(), O_WRONLY);
        return fd < 0 ? -1 : 0;
    }
    int set(char value) {
        return lseek(fd, 0, SEEK_SET) < 0 || write(fd, &value, 1) != 1 ? -1 : 0;
    }
    static uint64_t monotonic_ns() {
        timespec value{};
        if (clock_gettime(CLOCK_MONOTONIC, &value)) return 0;
        return static_cast<uint64_t>(value.tv_sec) * 1000000000ULL + value.tv_nsec;
    }
    static int sleep_until(uint64_t value) {
        timespec target{static_cast<time_t>(value / 1000000000ULL),
                        static_cast<long>(value % 1000000000ULL)};
        int status;
        do { status = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &target, nullptr); }
        while (status == EINTR);
        return status;
    }
    void run_periodic() {
        sched_param parameter{}; parameter.sched_priority = 80;
        if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &parameter))
            std::fputs("GPIO417: SCHED_FIFO unavailable\n", stderr);
        const uint64_t start = monotonic_ns() + 10000000ULL;
        uint64_t sequence = 0;
        while (!stopping.load()) {
            const uint64_t target = start + sequence * 1000000000ULL / 30;
            const int error = sleep_until(target);
            if (error) { failure.store(error); break; }
            if (stopping.load()) break;
            if (monotonic_ns() > target + 2000000ULL) { failure.store(ETIMEDOUT); break; }
            if (set('1')) { failure.store(errno ? errno : EIO); break; }
            const int pulse_error = sleep_until(monotonic_ns() + 1000000ULL);
            if (set('0')) { failure.store(errno ? errno : EIO); break; }
            if (pulse_error) { failure.store(pulse_error); break; }
            ++pulses;
            ++sequence;
        }
        set('0');
    }
    void start_periodic() { worker = std::thread(&Gpio417::run_periodic, this); }
    void stop_periodic() {
        stopping.store(true);
        if (worker.joinable()) worker.join();
    }
    void close_gpio() {
        stop_periodic();
        if (fd >= 0) {
            const char low = '0';
            const off_t seek_result = lseek(fd, 0, SEEK_SET);
            const ssize_t write_result = write(fd, &low, 1);
            const int close_result = close(fd);
            (void)seek_result; (void)write_result; (void)close_result; fd = -1;
        }
        if (exported) { std::ofstream out("/sys/class/gpio/unexport"); if (out) out << "417"; exported = false; }
    }
    ~Gpio417() { close_gpio(); }
};

pid_t spawn(const std::filesystem::path &program, unsigned camera_id,
            const std::string &group_name, int trigger_fd, int argc, char **argv) {
    const pid_t child = fork();
    if (child != 0) return child;
    std::vector<std::string> storage{"--camera-id", std::to_string(camera_id)};
    for (int i = 1; i < argc; ++i) storage.emplace_back(argv[i]);
    storage.emplace_back("--group-shm"); storage.push_back(group_name);
    if (trigger_fd >= 0) { storage.emplace_back("--trigger-fd"); storage.push_back(std::to_string(trigger_fd)); }
    std::vector<char *> arguments; std::string executable = program.string();
    arguments.push_back(executable.data());
    for (auto &item : storage) arguments.push_back(item.data());
    arguments.push_back(nullptr);
    execv(executable.c_str(), arguments.data());
    std::perror(executable.c_str()); _exit(127);
}
}

int main(int argc, char **argv) {
    if (argc == 2 && std::string(argv[1]) == "--version") {
        std::printf("cam_demo %s\n", orbvi::kDemoVersion); return 0;
    }
    orbvi::camera::Options options;
    const int parsed = orbvi::camera::parse_options(argc, argv, options);
    if (parsed) { orbvi::camera::print_usage(); return parsed == 1 ? 0 : 2; }
    if (options.camera_id >= 0) return 2;
    std::ifstream model_input("/proc/device-tree/model", std::ios::binary);
    std::string model((std::istreambuf_iterator<char>(model_input)), {});
    while (!model.empty() && model.back() == '\0') model.pop_back();
    if (model != "HS ORBVI 4P V1.0") {
        std::fputs("cam_demo: expected HS ORBVI 4P V1.0 device tree\n", stderr); return 2;
    }
    Gpio417 trigger;
    if (options.trigger_mode == "software_gpio" && trigger.open_gpio()) {
        std::perror("GPIO417"); return 1;
    }
    const std::string group_name = "/orbvi-camera-group-" + std::to_string(getpid());
    const int fd = shm_open(group_name.c_str(), O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd < 0) { std::perror("shm_open frame group"); return 1; }
    if (ftruncate(fd, sizeof(orbvi::camera::SharedFrameGroup))) {
        std::perror("ftruncate frame group"); close(fd); shm_unlink(group_name.c_str()); return 1;
    }
    void *mapping = mmap(nullptr, sizeof(orbvi::camera::SharedFrameGroup),
                         PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapping == MAP_FAILED) {
        std::perror("mmap frame group"); close(fd); shm_unlink(group_name.c_str()); return 1;
    }
    auto &group = *static_cast<orbvi::camera::SharedFrameGroup *>(mapping);
    if (orbvi::camera::initialize_group(group, options.camera_mask, options.max_skew_ns)) {
        std::fputs("initialize frame group failed\n", stderr);
        munmap(mapping, sizeof(group)); close(fd); shm_unlink(group_name.c_str()); return 1;
    }
    struct sigaction action{}; action.sa_handler = stop; sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, nullptr); sigaction(SIGTERM, &action, nullptr);
    const auto self = std::filesystem::canonical("/proc/self/exe");
    const auto camera_program = self.parent_path() / "cam_demo";
    std::vector<pid_t> children;
    for (unsigned camera = 0; camera < 4; ++camera) {
        if (!(options.camera_mask & (1u << camera))) continue;
        const pid_t child = spawn(camera_program, camera, group_name,
                                  options.external_sync ? -1 : trigger.fd, argc, argv);
        if (child < 0) { std::perror("fork cam_demo"); stop_requested = 1; break; }
        children.push_back(child);
    }
    int result = stop_requested ? 1 : 0;
    size_t running = children.size();
    bool trigger_started = false;
    const auto startup_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
    while (running) {
        if (options.external_sync && !trigger_started && orbvi::camera::all_cameras_ready(group)) {
            trigger.start_periodic();
            trigger_started = true;
            orbvi::camera::release_start(group);
            std::puts("GPIO417: external synchronization started at 30 Hz");
        }
        if (options.external_sync && !trigger_started && std::chrono::steady_clock::now() >= startup_deadline) {
            std::fputs("GPIO417: timed out waiting for cameras to prepare\n", stderr);
            result = 1; stop_requested = 1;
        }
        if (options.external_sync && trigger.failure.load()) {
            std::fprintf(stderr, "GPIO417: periodic trigger failed (%d)\n", trigger.failure.load());
            result = 1; stop_requested = 1;
        }
        if (stop_requested) {
            orbvi::camera::stop_group(group);
            for (pid_t child : children) if (child > 0) kill(child, SIGTERM);
            stop_requested = 0;
        }
        int status = 0; const pid_t child = waitpid(-1, &status, options.external_sync ? WNOHANG : 0);
        if (child == 0) { std::this_thread::sleep_for(std::chrono::milliseconds(2)); continue; }
        if (child < 0) {
            if (errno == EINTR) continue;
            result = 1; break;
        }
        --running;
        for (auto &entry : children) if (entry == child) entry = -1;
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) result = 1;
        if (running) {
            orbvi::camera::stop_group(group);
            for (pid_t entry : children) if (entry > 0) kill(entry, SIGTERM);
        }
    }
    if (options.external_sync) {
        trigger.stop_periodic();
        std::printf("GPIO417: pulses=%llu\n", static_cast<unsigned long long>(trigger.pulses.load()));
    }
    orbvi::camera::stop_group(group);
    orbvi::camera::destroy_group(group);
    munmap(mapping, sizeof(group)); close(fd); shm_unlink(group_name.c_str());
    return result;
}
