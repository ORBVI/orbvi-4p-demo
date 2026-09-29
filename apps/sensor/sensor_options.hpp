#pragma once

#include "camera_options.hpp"
#include <cstdint>
#include <string>

namespace orbvi::sensor {
struct Options {
    camera::Options camera;
    unsigned imu_sample_rate_hz = 1000;
    unsigned imu_print_rate_hz = 10;
    bool imu_print_metrics = false;
    std::string imu_udp_dest;
    std::string record_bag_path;
    std::string record_mp4_directory;
    unsigned record_frame_skip = 0;
};

int load_yaml(const std::string &path, Options &options);
int parse_options(int argc, const char *const *argv, const std::string &config_path,
                  Options &options);
void print_usage();
}
