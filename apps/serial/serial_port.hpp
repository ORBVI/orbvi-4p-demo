#pragma once
#include <string>

namespace orbvi::serial {
enum class Mode { Tx, Rx, TxRx, Echo };
struct Options {
    std::string port = "/dev/ttyS1";
    Mode mode = Mode::TxRx;
    unsigned baud = 115200;
    unsigned count = 0;
    unsigned interval_ms = 1000;
    unsigned timeout_ms = 200;
    std::string text = "uart-demo";
    bool append_newline = true;
};
int parse_options(int argc, const char *const *argv, Options &options);
void print_usage();
int run(const Options &options, int output_fd);
}
