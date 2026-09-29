#include "time_sync.hpp"
#include "../version.hpp"
#include <atomic>
#include <cerrno>
#include <climits>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <deque>
#include <dirent.h>
#include <fcntl.h>
#include <linux/pps.h>
#include <mutex>
#include <poll.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace orbvi::timesync;
namespace {
volatile sig_atomic_t interrupted = 0;
void stop(int) { interrupted = 1; }
int64_t clock_ns(clockid_t clock_id) {
    timespec ts{};
    if (clock_gettime(clock_id, &ts)) return -1;
    return int64_t(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
}
std::string read_file(const std::string &path) {
    int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) return {};
    char data[256]{};
    ssize_t n = read(fd, data, sizeof(data) - 1);
    close(fd);
    return n > 0 ? std::string(data, static_cast<size_t>(n)) : std::string{};
}
bool time_daemon_running() {
    DIR *dir = opendir("/proc");
    if (!dir) return true;
    bool running = false;
    while (auto *ent = readdir(dir)) {
        if (ent->d_name[0] < '0' || ent->d_name[0] > '9') continue;
        const std::string name = read_file(std::string("/proc/") + ent->d_name + "/comm");
        if (name == "ntpd\n" || name == "phc2sys\n" || name == "chronyd\n" ||
            name == "systemd-timesyn\n" || name == "ntpdate\n") { running = true; break; }
    }
    closedir(dir);
    return running;
}
bool gpio_pps(const std::string &node) {
    const std::string name = read_file("/sys/class/pps/" + node + "/name");
    // This BSP registers its DT pps-gpio source as "pps.-1", not "pps-gpio".
    return name.find("pps-gpio") != std::string::npos ||
           (name == "pps.-1\n" && read_file("/sys/bus/platform/devices/pps/uevent").find("DRIVER=pps-gpio") != std::string::npos);
}
std::string choose_pps(std::string pps) {
    if (!pps.empty()) {
        if (pps.rfind("/dev/pps", 0) != 0 || pps.substr(8).find_first_not_of("0123456789") != std::string::npos || pps.size() < 9)
            return {};
        return gpio_pps(pps.substr(5)) ? pps : std::string{};
    }
    for (int i = 0; i < 16; ++i) {
        auto candidate = "/dev/pps" + std::to_string(i);
        if (gpio_pps("pps" + std::to_string(i))) {
            if (!pps.empty()) return {};
            pps = candidate;
        }
    }
    return pps;
}
bool configure_uart(int fd) {
    termios t{};
    if (tcgetattr(fd, &t)) return false;
    cfmakeraw(&t);
    cfsetispeed(&t, B115200); cfsetospeed(&t, B115200);
    t.c_cflag = (t.c_cflag & ~(CSIZE | CSTOPB | PARENB | CRTSCTS)) | CS8 | CLOCAL | CREAD;
    t.c_cc[VMIN] = 0; t.c_cc[VTIME] = 0;
    return tcsetattr(fd, TCSANOW, &t) == 0;
}
void report(const Match &m, bool apply) {
    const int64_t before = clock_ns(CLOCK_REALTIME);
    const auto target = target_now_ns(m, before);
    if (!target) { std::fprintf(stderr, "PPS stale or system clock jumped; LOST\n"); return; }
    const int64_t offset = *target - before;
    time_t utc = static_cast<time_t>(m.utc_seconds);
    tm stamp{};
    gmtime_r(&utc, &stamp);
    char text[32]{};
    strftime(text, sizeof(text), "%Y-%m-%dT%H:%M:%SZ", &stamp);
    std::printf("PPS#%u UTC=%s matches=%u state=%s offset_ms=%.3f\n",
                m.sequence, text, m.consecutive, m.consecutive >= 3 ? "LOCKED" : "ACQUIRING",
                static_cast<double>(offset) / 1e6);
    if (!apply || m.consecutive < 3) return;
    if (time_daemon_running()) {
        std::fprintf(stderr, "Refusing to set clock: ntpd/phc2sys/chronyd/other time daemon running\n");
        return;
    }
    if (offset > -5000000 && offset < 5000000) return; // already within 5ms
    // /proc daemon check and logging consume milliseconds: recalculate immediately before the step.
    const int64_t fresh = clock_ns(CLOCK_REALTIME);
    const auto fresh_target = target_now_ns(m, fresh);
    if (!fresh_target) { std::fprintf(stderr, "Clock changed before apply; skipping\n"); return; }
    const int64_t start = clock_ns(CLOCK_MONOTONIC);
    timespec ts{static_cast<time_t>(*fresh_target / 1000000000LL), static_cast<long>(*fresh_target % 1000000000LL)};
    if (clock_settime(CLOCK_REALTIME, &ts)) { std::perror("clock_settime"); interrupted = 1; return; }
    const int64_t end = clock_ns(CLOCK_MONOTONIC);
    const int64_t after = clock_ns(CLOCK_REALTIME);
    std::printf("APPLIED PPS#%u residual_ms=%.3f\n", m.sequence,
                static_cast<double>(after - *fresh_target - (end - start)) / 1e6);
}
} // namespace
int main(int argc, char **argv) {
    std::string uart = "/dev/ttyS1", selected_pps;
    bool apply = false;
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--version")) { std::printf("timesync_demo %s\n", orbvi::kDemoVersion); return 0; }
        if (!std::strcmp(argv[i], "--apply")) { apply = true; continue; }
        if ((!std::strcmp(argv[i], "--uart") || !std::strcmp(argv[i], "--pps")) && i + 1 < argc) {
            const bool is_uart = !std::strcmp(argv[i], "--uart");
            (is_uart ? uart : selected_pps) = argv[++i]; continue;
        }
        std::fprintf(stderr, "Usage: timesync_demo [--uart /dev/ttyS1] [--pps /dev/ppsN] [--apply]\n"
                             "Default is receive-only; --apply SETS CLOCK_REALTIME from TEST UTC!\n");
        return 2;
    }
    if (apply && time_daemon_running()) {
        std::fprintf(stderr, "Refusing --apply: a time daemon is running; stop it deliberately first\n"); return 1;
    }
    auto pps_path = choose_pps(selected_pps);
    if (pps_path.empty()) { std::fprintf(stderr, "No unique pps-gpio source found (check /sys/class/pps/*/name)\n"); return 1; }
    const int pps_fd = open(pps_path.c_str(), O_RDONLY | O_CLOEXEC);
    if (pps_fd < 0) { std::perror(pps_path.c_str()); return 1; }
    int cap = 0;
    if (ioctl(pps_fd, PPS_GETCAP, &cap) || !(cap & PPS_CAPTUREASSERT)) {
        std::fprintf(stderr, "PPS source has no assert timestamps\n"); close(pps_fd); return 1;
    }
    const int uart_fd = open(uart.c_str(), O_RDONLY | O_NOCTTY | O_NONBLOCK | O_CLOEXEC);
    if (uart_fd < 0) { std::perror(uart.c_str()); close(pps_fd); return 1; }
    if (ioctl(uart_fd, TIOCEXCL) || !configure_uart(uart_fd)) {
        std::perror("exclusive UART/termios"); close(uart_fd); close(pps_fd); return 1;
    }
    int notify[2]{};
    if (pipe2(notify, O_CLOEXEC | O_NONBLOCK)) { std::perror("pipe2"); close(uart_fd); close(pps_fd); return 1; }
    pps_fdata snapshot{};
    snapshot.timeout.sec = 0; snapshot.timeout.nsec = 0;
    // Do not treat the last PPS recorded before startup as a fresh pulse.
    uint32_t baseline = 0;
    if (!ioctl(pps_fd, PPS_FETCH, &snapshot)) baseline = snapshot.info.assert_sequence;
    std::mutex mutex;
    std::deque<Edge> edges;
    std::atomic<bool> done{false}, pps_error{false};
    std::thread worker([&] {
        uint32_t last = baseline;
        while (!done) {
            pps_fdata data{};
            data.timeout.sec = 1;
            if (ioctl(pps_fd, PPS_FETCH, &data)) {
                if (errno != ETIMEDOUT && errno != EINTR) { pps_error = true; break; }
                continue;
            }
            if (data.info.assert_sequence == last) continue;
            if (data.info.assert_tu.nsec < 0 || data.info.assert_tu.nsec >= 1000000000) { pps_error = true; break; }
            last = data.info.assert_sequence;
            auto received = Steady::now();
            {
                std::lock_guard<std::mutex> lock(mutex);
                edges.push_back(Edge{last, data.info.assert_tu.sec * 1000000000LL + data.info.assert_tu.nsec, received});
            }
            const char byte = 'p';
            if (write(notify[1], &byte, 1) < 0 && errno != EAGAIN) { pps_error = true; break; }
        }
    });
    struct sigaction action{}; action.sa_handler = stop; sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, nullptr); sigaction(SIGTERM, &action, nullptr);
    std::printf("Listening UART=%s PPS=%s mode=%s (source is simulated UTC)\n", uart.c_str(), pps_path.c_str(), apply ? "APPLY" : "observe");
    std::fflush(stdout);
    Pairer pairer;
    std::string line;
    unsigned previous_lock = 0;
    while (!interrupted && !pps_error) {
        pollfd fds[2]{{notify[0], POLLIN, 0}, {uart_fd, POLLIN, 0}};
        int ready = poll(fds, 2, 100);
        if (ready < 0) { if (errno == EINTR) continue; std::perror("poll"); break; }
        if (fds[0].revents & POLLIN) {
            char tmp[128]; while (read(notify[0], tmp, sizeof(tmp)) > 0) {}
            std::deque<Edge> queue;
            { std::lock_guard<std::mutex> lock(mutex); queue.swap(edges); }
            for (const auto &e : queue) if (auto matched = pairer.on_pps(e)) report(*matched, apply);
        }
        if (fds[1].revents & POLLIN) {
            char bytes[256];
            ssize_t n = read(uart_fd, bytes, sizeof(bytes));
            if (n == 0) { std::fprintf(stderr, "UART disconnected\n"); break; }
            if (n < 0 && errno != EAGAIN && errno != EINTR) { std::perror("UART read"); break; }
            for (ssize_t j = 0; j < (n > 0 ? n : 0); ++j) {
                const char c = bytes[j];
                if (c == '\n') {
                    if (!line.empty() && line.back() == '\r') line.pop_back();
                    if (auto utc = parse_rmc(line)) {
                        if (auto matched = pairer.on_rmc(*utc, Steady::now())) report(*matched, apply);
                    } else if (!line.empty()) std::fprintf(stderr, "Invalid RMC discarded\n");
                    line.clear();
                } else if (line.size() < 127) line.push_back(c);
                else line.clear();
            }
        }
        if ((fds[0].revents | fds[1].revents) & (POLLERR | POLLHUP | POLLNVAL)) break;
        pairer.tick(Steady::now());
        if (previous_lock >= 3 && pairer.consecutive() == 0) std::fprintf(stderr, "LOST PPS/RMC lock\n");
        previous_lock = pairer.consecutive();
    }
    done = true; worker.join();
    close(notify[0]); close(notify[1]); close(uart_fd); close(pps_fd);
    return pps_error || !interrupted ? 1 : 0;
}
