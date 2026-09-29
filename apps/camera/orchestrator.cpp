#include "camera_options.hpp"
#include "process_group.hpp"
#include "../version.hpp"
#include <cerrno>
#include <csignal>
#include <cstdio>
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
    void close_gpio() {
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
        const pid_t child = spawn(camera_program, camera, group_name, trigger.fd, argc, argv);
        if (child < 0) { std::perror("fork cam_demo"); stop_requested = 1; break; }
        children.push_back(child);
    }
    int result = 0; size_t running = children.size();
    while (running) {
        if (stop_requested) {
            orbvi::camera::stop_group(group);
            for (pid_t child : children) if (child > 0) kill(child, SIGTERM);
            stop_requested = 0;
        }
        int status = 0; const pid_t child = waitpid(-1, &status, 0);
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
    orbvi::camera::stop_group(group);
    orbvi::camera::destroy_group(group);
    munmap(mapping, sizeof(group)); close(fd); shm_unlink(group_name.c_str());
    return result;
}
