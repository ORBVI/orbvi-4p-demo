#pragma once
#include <cstdint>
#include <string>

namespace orbvi::camera {
enum class Codec { H264, H265 };
struct Options {
    int camera_id = -1;
    unsigned fps = 30;
    unsigned width = 1280;
    unsigned height = 1088;
    unsigned channels = 4;
    unsigned camera_mask = 0xf;
    unsigned rtsp_base_port = 554;
    unsigned diag_interval_ms = 1000;
    unsigned rotate = 0;
    unsigned bitrate_kbps = 4000;
    unsigned frame_timeout_ms = 100;
    uint64_t max_skew_ns = 1000000;
    Codec codec = Codec::H264;
    std::string url = "/live/PRR";
    std::string record_mp4_directory;
    std::string record_h264_path;
    std::string record_h264_directory;
    std::string group_shm;
    std::string trigger_mode = "software_gpio";
    int trigger_fd = -1;
    unsigned record_frame_skip = 0;
    bool diagnostics = false;
};
int parse_options(int argc, const char *const *argv, Options &options);
void print_usage();
}
