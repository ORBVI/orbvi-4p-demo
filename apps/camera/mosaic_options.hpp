#pragma once
#include <cstdint>
#include <string>

namespace orbvi::mosaic {
struct Options {
    unsigned fps = 30;
    unsigned bitrate_kbps = 8000;
    unsigned port = 558;
    unsigned frame_timeout_ms = 3000;
    uint64_t max_skew_ns = 40000000;
    std::string url = "/live/PRR";
    bool diagnostics = false;
    bool serial_compose = false;
};
int parse_options(int argc, const char *const *argv, Options &options);
void print_usage();
}
