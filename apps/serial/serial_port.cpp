#include "serial_port.hpp"
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

namespace orbvi::serial {
namespace {
volatile sig_atomic_t stopped = 0;
void stop(int) { stopped = 1; }
bool number(const char *text, unsigned min, unsigned max, unsigned &value) {
    if (!*text || *text == '-' || *text == '+') return false;
    char *end = nullptr; errno = 0;
    unsigned long parsed = strtoul(text, &end, 10);
    if (errno || *end || parsed < min || parsed > max) return false;
    value = static_cast<unsigned>(parsed); return true;
}
speed_t speed(unsigned baud) {
    switch (baud) {
    case 9600: return B9600; case 19200: return B19200; case 38400: return B38400;
    case 57600: return B57600; case 115200: return B115200;
#ifdef B230400
    case 230400: return B230400;
#endif
#ifdef B460800
    case 460800: return B460800;
#endif
#ifdef B921600
    case 921600: return B921600;
#endif
    default: return 0;
    }
}
bool write_all(int fd, const void *data, size_t size) {
    const auto *next = static_cast<const unsigned char *>(data);
    while (size) {
        ssize_t written = write(fd, next, size);
        if (written < 0 && errno == EINTR) continue;
        if (written <= 0) return false;
        next += written; size -= static_cast<size_t>(written);
    }
    return true;
}
int receive(int fd, unsigned timeout_ms, std::string &data) {
    pollfd event{fd, POLLIN, 0};
    int ready;
    do { ready = poll(&event, 1, static_cast<int>(timeout_ms)); } while (ready < 0 && errno == EINTR);
    if (ready <= 0) return -1;
    char buffer[1024];
    ssize_t size;
    do { size = read(fd, buffer, sizeof(buffer)); } while (size < 0 && errno == EINTR);
    if (size <= 0) return -1;
    data.assign(buffer, static_cast<size_t>(size)); return 0;
}
}

int parse_options(int argc, const char *const *argv, Options &options) {
    options = Options{};
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (!strcmp(arg, "--help") || !strcmp(arg, "-h")) return 1;
        if (!strcmp(arg, "--no-newline")) { options.append_newline = false; continue; }
        if (++i == argc) return 2;
        const char *value = argv[i];
        if (!strcmp(arg, "--port")) {
            if (!*value) return 2;
            options.port = value;
            if (options.port.rfind("tty", 0) == 0 || options.port.rfind("TTY", 0) == 0)
                options.port = "/dev/" + options.port;
        } else if (!strcmp(arg, "--mode")) {
            if (!strcmp(value, "tx")) options.mode = Mode::Tx;
            else if (!strcmp(value, "rx")) options.mode = Mode::Rx;
            else if (!strcmp(value, "txrx")) options.mode = Mode::TxRx;
            else if (!strcmp(value, "echo")) options.mode = Mode::Echo;
            else return 2;
        } else if (!strcmp(arg, "--baud")) {
            if (!number(value, 1, 4000000, options.baud) || !speed(options.baud)) return 2;
        } else if (!strcmp(arg, "--count")) {
            if (!number(value, 0, 1000000, options.count)) return 2;
        } else if (!strcmp(arg, "--interval-ms")) {
            if (!number(value, 0, 60000, options.interval_ms)) return 2;
        } else if (!strcmp(arg, "--timeout-ms")) {
            if (!number(value, 1, 60000, options.timeout_ms)) return 2;
        } else if (!strcmp(arg, "--text")) options.text = value;
        else return 2;
    }
    return 0;
}

void print_usage() {
    puts("UART1/UART7 serial demo\n"
         "Usage: serial_port_demo [options]\n"
         "  --port DEVICE          /dev/ttyS1 or /dev/ttyS7 (default: /dev/ttyS1)\n"
         "  --mode tx|rx|txrx|echo (default: txrx)\n"
         "  --baud RATE            9600..921600 standard rate (default: 115200)\n"
         "  --count N              Operations; 0 runs until signal (default: 0)\n"
         "  --interval-ms N        Delay between operations (default: 1000)\n"
         "  --text TEXT            Transmit payload prefix (default: uart-demo)\n"
         "  --no-newline           Do not append newline to TX payload\n"
         "  --timeout-ms N         Receive timeout (default: 200)\n"
         "This demo is not for the 1.8V DEBUG_UART.");
}

int run(const Options &options, int output_fd) {
    int fd = open(options.port.c_str(), O_RDWR | O_NOCTTY | O_CLOEXEC);
    if (fd < 0) { perror(options.port.c_str()); return 1; }
    termios config{};
    if (tcgetattr(fd, &config)) { perror("tcgetattr"); close(fd); return 1; }
    cfmakeraw(&config);
    speed_t baud = speed(options.baud);
    cfsetispeed(&config, baud); cfsetospeed(&config, baud);
    config.c_cflag |= CLOCAL | CREAD;
    config.c_cflag &= ~(CSTOPB | CRTSCTS | PARENB | CSIZE);
    config.c_cflag |= CS8;
    config.c_cc[VMIN] = 0; config.c_cc[VTIME] = 0;
    if (tcsetattr(fd, TCSANOW, &config)) { perror("tcsetattr"); close(fd); return 1; }
    stopped = 0;
    struct sigaction action{}; action.sa_handler = stop; sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, nullptr); sigaction(SIGTERM, &action, nullptr);
    int result = 0;
    unsigned operations = 0;
    while (!stopped && (!options.count || operations < options.count)) {
        std::string payload = options.text + " seq=" + std::to_string(operations);
        if (options.append_newline) payload.push_back('\n');
        if (options.mode == Mode::Tx || options.mode == Mode::TxRx) {
            if (!write_all(fd, payload.data(), payload.size()) || tcdrain(fd)) { result = 1; break; }
        }
        if (options.mode != Mode::Tx) {
            std::string data;
            if (receive(fd, options.timeout_ms, data)) {
                if (options.mode == Mode::TxRx) ++operations;
                if (options.interval_ms) usleep(static_cast<useconds_t>(options.interval_ms) * 1000);
                continue;
            }
            if (options.mode == Mode::Echo) {
                if (!write_all(fd, data.data(), data.size()) || tcdrain(fd)) { result = 1; break; }
            } else if (!write_all(output_fd, data.data(), data.size())) { result = 1; break; }
        }
        ++operations;
        if (options.interval_ms) usleep(static_cast<useconds_t>(options.interval_ms) * 1000);
    }
    if (close(fd)) result = 1;
    return result;
}
}
