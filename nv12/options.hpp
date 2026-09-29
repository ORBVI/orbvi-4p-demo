#pragma once
#include <cstdint>
#include <string>

namespace orbvi::nv12 {
struct Options {
    unsigned groups = 10;
    unsigned timeout_ms = 3000;
    uint64_t max_skew_ns = 1000000;
    std::string output = "nv12-capture";
    bool discard = false;
};
// 0: parsed, 1: help, 2: invalid options.
int parse_options(int argc, const char *const *argv, Options &options);
void print_usage();
}
