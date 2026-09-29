#include "serial_port.hpp"
#include <cassert>
#include <fcntl.h>
#include <poll.h>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>

using namespace orbvi::serial;
namespace {
struct Pty {
    int master = posix_openpt(O_RDWR | O_NOCTTY);
    std::string slave;
    Pty() {
        assert(master >= 0 && grantpt(master) == 0 && unlockpt(master) == 0);
        char *name = ptsname(master); assert(name); slave = name;
    }
    ~Pty() { close(master); }
};
std::string read_exact(int fd, size_t length) {
    std::string result;
    while (result.size() < length) {
        pollfd event{fd, POLLIN, 0};
        assert(poll(&event, 1, 1000) == 1);
        char data[64];
        ssize_t size = read(fd, data, sizeof(data)); assert(size > 0);
        result.append(data, static_cast<size_t>(size));
    }
    return result;
}
void wait_raw(const std::string &path) {
    for (unsigned attempt = 0; attempt < 1000; ++attempt) {
        int fd = open(path.c_str(), O_RDONLY | O_NOCTTY | O_NONBLOCK); assert(fd >= 0);
        termios config{}; assert(tcgetattr(fd, &config) == 0); close(fd);
        if (!(config.c_lflag & (ICANON | ECHO))) return;
        usleep(1000);
    }
    assert(false);
}
}

int main() {
    Options options;
    const char *defaults[]{"serial_port_demo"};
    assert(parse_options(1, defaults, options) == 0);
    assert(options.count == 0 && options.timeout_ms == 200);
    assert(options.interval_ms == 1000 && options.text == "uart-demo");
    assert(options.append_newline);
    const char *args[]{"serial_port_demo", "--port", "/dev/ttyS7", "--mode", "echo",
                       "--baud", "9600", "--count", "2", "--timeout-ms", "100",
                       "--interval-ms", "0", "--no-newline"};
    assert(parse_options(14, args, options) == 0);
    assert(options.port == "/dev/ttyS7" && options.mode == Mode::Echo);
    assert(options.baud == 9600 && options.count == 2 && options.timeout_ms == 100);
    assert(options.interval_ms == 0 && !options.append_newline);
    const char *short_port[]{"demo", "--port", "ttyS1"};
    assert(parse_options(3, short_port, options) == 0 && options.port == "/dev/ttyS1");
    const char *bad[]{"demo", "--baud", "12345"};
    assert(parse_options(3, bad, options) == 2);

    Pty tx;
    options = Options{}; options.port = tx.slave; options.mode = Mode::Tx;
    options.text = "hello-x5"; options.count = 1; options.interval_ms = 0; options.append_newline = false;
    assert(run(options, STDOUT_FILENO) == 0);
    assert(read_exact(tx.master, 14) == "hello-x5 seq=0");

    Pty exchange;
    options.port = exchange.slave; options.mode = Mode::TxRx;
    options.text = "ping"; options.timeout_ms = 500; options.interval_ms = 0; options.append_newline = false;
    int output[2]; assert(pipe(output) == 0);
    int status = -1;
    std::thread worker([&] { status = run(options, output[1]); close(output[1]); });
    wait_raw(exchange.slave);
    assert(read_exact(exchange.master, 10) == "ping seq=0");
    assert(write(exchange.master, "pong", 4) == 4);
    worker.join();
    assert(status == 0 && read_exact(output[0], 4) == "pong"); close(output[0]);

    Pty echo;
    options.port = echo.slave; options.mode = Mode::Echo; options.count = 1;
    std::thread echo_worker([&] { status = run(options, STDOUT_FILENO); });
    wait_raw(echo.slave);
    assert(write(echo.master, "abc", 3) == 3);
    assert(read_exact(echo.master, 3) == "abc");
    echo_worker.join(); assert(status == 0);

    Pty timeout;
    options.port = timeout.slave; options.mode = Mode::TxRx; options.count = 1;
    options.text = "timeout"; options.timeout_ms = 20; options.interval_ms = 0;
    assert(run(options, STDOUT_FILENO) == 0);
}
