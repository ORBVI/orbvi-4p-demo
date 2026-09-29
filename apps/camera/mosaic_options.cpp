#include "mosaic_options.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace orbvi::mosaic {
namespace {
bool parse_number(const char *text, uint64_t low, uint64_t high, uint64_t &value) {
    if (!text || !*text || *text == '-' || *text == '+') return false;
    char *end = nullptr;
    errno = 0;
    const auto number = strtoull(text, &end, 10);
    if (errno || *end || number < low || number > high) return false;
    value = number;
    return true;
}
bool rtsp_path(const char *path) {
    if (!path || path[0] != '/') return false;
    const char *slash = std::strchr(path + 1, '/');
    return slash && slash != path + 1 && slash[1] != '\0';
}
}
int parse_options(int argc, const char *const *argv, Options &options) {
    options = Options{};
    for (int index = 1; index < argc; ++index) {
        const char *arg = argv[index];
        if (!std::strcmp(arg, "--help") || !std::strcmp(arg, "-h")) return 1;
        if (!std::strcmp(arg, "--diagnostics")) { options.diagnostics = true; continue; }
        if (!std::strcmp(arg, "--serial-compose")) { options.serial_compose = true; continue; }
        if (++index == argc) return 2;
        const char *value = argv[index];
        uint64_t number = 0;
        if (!std::strcmp(arg, "--fps")) {
            if (!parse_number(value, 25, 60, number) ||
                (number != 25 && number != 30 && number != 40 && number != 50 && number != 60)) return 2;
            options.fps = static_cast<unsigned>(number);
        } else if (!std::strcmp(arg, "--bps")) {
            if (!parse_number(value, 1, 100000, number)) return 2;
            options.bitrate_kbps = static_cast<unsigned>(number);
        } else if (!std::strcmp(arg, "--port")) {
            if (!parse_number(value, 1, 65535, number)) return 2;
            options.port = static_cast<unsigned>(number);
        } else if (!std::strcmp(arg, "--url")) {
            if (!rtsp_path(value)) return 2;
            options.url = value;
        } else if (!std::strcmp(arg, "--max-skew-ns")) {
            if (!parse_number(value, 1, 1000000000, number)) return 2;
            options.max_skew_ns = number;
        } else if (!std::strcmp(arg, "--frame-timeout-ms")) {
            if (!parse_number(value, 1, 60000, number)) return 2;
            options.frame_timeout_ms = static_cast<unsigned>(number);
        } else return 2;
    }
    return 0;
}
void print_usage() {
    std::puts("ORBVI 4P mosaic RTSP demo (camera0 camera1 / camera2 camera3)\n"
              "Usage: mosaic_rtsp_demo [options]\n"
              "  --fps 25|30|40|50|60     Output frame rate (default: 30)\n"
              "  --bps KBPS               H.264 bitrate (default: 8000)\n"
              "  --port PORT              RTSP port (default: 558)\n"
              "  --url /APP/STREAM        RTSP path (default: /live/PRR)\n"
              "  --max-skew-ns N          Max group timestamp span (default: 40000000)\n"
              "  --frame-timeout-ms N     Group timeout (default: 3000)\n"
              "  --diagnostics            Print FPS, stage times, queue drops and skew\n"
              "  --serial-compose         CPU fallback: compose tiles sequentially");
}
}
