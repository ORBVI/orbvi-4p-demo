#pragma once
#include <array>
#include <cstdint>
#include <iosfwd>
#include <string>

namespace orbvi::imu {
struct Options {
    unsigned sample_rate_hz = 1000;
    unsigned print_rate_hz = 10;
    uint64_t count = 0;
    bool print_metrics = false;
    std::string csv_path;
    std::string udp_dest;
};
struct Sample {
    int64_t timestamp_ns = 0;
    std::array<double, 3> accel{};
    std::array<double, 3> gyro{};
    // Raw IIO timestamps retained for the OIMU v1 wire format.
    int64_t accel_timestamp_ns = 0;
    int64_t gyro_timestamp_ns = 0;
};
int parse_options(int argc, const char *const *argv, Options &options);
void print_usage();
void print_sample(std::ostream &output, const Sample &sample);
int run(const Options &options);
}
