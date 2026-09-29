#include "options.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace orbvi::nv12 {
namespace {
bool parse_number(const char *text, uint64_t min, uint64_t max, uint64_t &value) {
    if (!*text || *text == '-' || *text == '+') return false;
    char *end = nullptr;
    errno = 0;
    unsigned long long parsed = strtoull(text, &end, 10);
    if (errno || *end || parsed < min || parsed > max) return false;
    value = parsed;
    return true;
}
}

int parse_options(int argc, const char *const *argv, Options &options) {
    options = Options{};
    for (int index = 1; index < argc; ++index) {
        const char *arg = argv[index];
        if (!std::strcmp(arg, "--help") || !std::strcmp(arg, "-h")) return 1;
        if (!std::strcmp(arg, "--discard")) { options.discard = true; continue; }
        if (++index == argc) return 2;
        const char *value = argv[index];
        uint64_t number = 0;
        if (!std::strcmp(arg, "--frames")) {
            if (!parse_number(value, 1, 1000000, number)) return 2;
            options.groups = static_cast<unsigned>(number);
        } else if (!std::strcmp(arg, "--timeout-ms")) {
            if (!parse_number(value, 1, 60000, number)) return 2;
            options.timeout_ms = static_cast<unsigned>(number);
        } else if (!std::strcmp(arg, "--max-skew-ns")) {
            if (!parse_number(value, 1, 1000000000, number)) return 2;
            options.max_skew_ns = number;
        } else if (!std::strcmp(arg, "--output")) {
            if (!*value) return 2;
            options.output = value;
        } else return 2;
    }
    return 0;
}

void print_usage() {
    std::puts("ORBVI 4P NV12 frame sets (X5 BSP)\n"
              "Usage: orbvi-nv12-demo [options]\n"
              "  --frames N             Complete four-camera sets (default: 10)\n"
              "  --output DIR           New directory (default: nv12-capture)\n"
              "  --max-skew-ns N        Maximum SDK timestamp spread (default: 1000000)\n"
              "  --timeout-ms N         Complete-set timeout (default: 3000)\n"
              "  --discard              Save metadata without NV12 pixels\n"
              "  --help                 Show help");
}
}
