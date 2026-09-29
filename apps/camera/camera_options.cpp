#include "camera_options.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>

namespace orbvi::camera {
namespace {
bool number(const char *text, uint64_t min, uint64_t max, uint64_t &value) {
    if (!*text || *text == '-' || *text == '+') return false;
    char *end = nullptr; errno = 0;
    unsigned long long parsed = strtoull(text, &end, 10);
    if (errno || *end || parsed < min || parsed > max) return false;
    value = parsed; return true;
}
bool rtsp_path(const char *text) {
    if (!text || text[0] != '/' || text[1] == '\0') return false;
    const char *separator = std::strchr(text + 1, '/');
    return separator && separator != text + 1 && separator[1] != '\0';
}
}
int parse_options(int argc, const char *const *argv, Options &options) {
    options = Options{};
    const std::set<unsigned> rates{25, 30, 40, 50, 60};
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (!strcmp(arg, "--help") || !strcmp(arg, "-h")) return 1;
        if (!strcmp(arg, "--diagnostics")) { options.diagnostics = true; continue; }
        if (!strcmp(arg, "--trigger-mode")) {
            if (++i == argc || (strcmp(argv[i], "software_gpio") && strcmp(argv[i], "none"))) return 2;
            options.trigger_mode = argv[i];
            continue;
        }
        if (++i == argc) return 2;
        const char *text = argv[i]; uint64_t value = 0;
        if (!strcmp(arg, "--camera-id")) {
            if (!number(text, 0, 3, value)) return 2;
            options.camera_id = static_cast<int>(value);
            options.camera_mask = 1u << value;
        } else if (!strcmp(arg, "--channels")) {
            if (!number(text, 1, 4, value)) return 2;
            options.channels = static_cast<unsigned>(value);
            options.camera_mask = (1u << value) - 1u;
        } else if (!strcmp(arg, "--camera-mask")) {
            if (!number(text, 1, 15, value)) return 2;
            options.camera_mask = static_cast<unsigned>(value);
        } else if (!strcmp(arg, "--width")) {
            if (!number(text, 1, 1280, value)) return 2;
            options.width = static_cast<unsigned>(value);
        } else if (!strcmp(arg, "--height")) {
            if (!number(text, 1, 1280, value)) return 2;
            options.height = static_cast<unsigned>(value);
        } else if (!strcmp(arg, "--rtsp-base-port")) {
            if (!number(text, 1, 65532, value)) return 2;
            options.rtsp_base_port = static_cast<unsigned>(value);
        } else if (!strcmp(arg, "--diag-interval-ms")) {
            if (!number(text, 1, 60000, value)) return 2;
            options.diag_interval_ms = static_cast<unsigned>(value);
        } else if (!strcmp(arg, "--fps")) {
            if (!number(text, 1, 60, value) || !rates.count(static_cast<unsigned>(value))) return 2;
            options.fps = static_cast<unsigned>(value);
        } else if (!strcmp(arg, "--rotate")) {
            if (!number(text, 0, 270, value) || (value != 0 && value != 90 && value != 180 && value != 270)) return 2;
            options.rotate = static_cast<unsigned>(value);
        } else if (!strcmp(arg, "--bps")) {
            if (!number(text, 1, 100000, value)) return 2;
            options.bitrate_kbps = static_cast<unsigned>(value);
        } else if (!strcmp(arg, "--codec")) {
            if (!strcmp(text, "h264")) options.codec = Codec::H264;
            else if (!strcmp(text, "h265")) options.codec = Codec::H265;
            else return 2;
        } else if (!strcmp(arg, "--url")) {
            if (!rtsp_path(text)) return 2;
            options.url = text;
        } else if (!strcmp(arg, "--record-mp4-dir")) {
            if (*text != '/') return 2;
            options.record_mp4_directory = text;
        } else if (!strcmp(arg, "--record-h264")) {
            if (*text != '/') return 2;
            options.record_h264_path = text;
        } else if (!strcmp(arg, "--record-h264-dir")) {
            if (*text != '/') return 2;
            options.record_h264_directory = text;
        } else if (!strcmp(arg, "--record-frame-skip")) {
            if (!number(text, 0, 1, value)) return 2;
            options.record_frame_skip = static_cast<unsigned>(value);
        } else if (!strcmp(arg, "--group-shm")) {
            if (*text != '/') return 2;
            options.group_shm = text;
        } else if (!strcmp(arg, "--trigger-fd")) {
            if (!number(text, 0, 1048576, value)) return 2;
            options.trigger_fd = static_cast<int>(value);
        } else if (!strcmp(arg, "--frame-timeout-ms")) {
            if (!number(text, 1, 60000, value)) return 2;
            options.frame_timeout_ms = static_cast<unsigned>(value);
        } else if (!strcmp(arg, "--max-skew-ns")) {
            if (!number(text, 1, 1000000000, value)) return 2;
            options.max_skew_ns = value;
        } else return 2;
    }
    const bool supported_size =
        (options.width == 1280 && options.height == 1088) ||
        (options.width == 640 && options.height == 480) ||
        (options.width == 720 && options.height == 480) ||
        (options.width == 1280 && options.height == 720) ||
        (options.width == 1088 && options.height == 1280) ||
        (options.width == 480 && options.height == 640) ||
        (options.width == 480 && options.height == 720) ||
        (options.width == 720 && options.height == 1280);
    if (!supported_size || (options.rotate == 180 && options.fps != 30)) return 2;
    return 0;
}
void print_usage() {
    puts("ORBVI 4P NV12/RTSP camera demo\n"
         "Usage: cam_demo [options]\n"
         "  --channels 1|2|3|4      First N cameras (default: 4)\n"
         "  --camera-id 0|1|2|3   Select one camera\n"
         "  --camera-mask 1..15   Camera bit mask\n"
         "  --width PIXELS        Output width (default: 1280)\n"
         "  --height PIXELS       Output height (default: 1088)\n"
         "  --fps 25|30|40|50|60  Output frame rate (default: 30)\n"
         "  --codec h264|h265      RTSP codec (default: h264)\n"
         "  --bps KBPS             Bit rate (default: 4000)\n"
         "  --url /APP/STREAM      RTSP path (default: /live/PRR)\n"
         "  --rotate 0|90|180|270  Encoded-stream rotation\n"
         "  --rtsp-base-port PORT  First RTSP port (default: 554)\n"
         "  --diagnostics          Print counters\n"
         "  --diag-interval-ms N   Diagnostics interval (default: 1000)\n"
         "  --max-skew-ns N        Four-camera grouping threshold\n"
         "  --trigger-mode software_gpio|none  GPIO417 IMU FSYNC output\n"
         "  --frame-timeout-ms N   Frame timeout");
}
}
