#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace orbvi::cpu_temp {

// OCPU/1 is independent of the OIMU/1 accelerometer/gyroscope protocol.
constexpr std::size_t kPacketSize = 32;
struct Sample {
    float celsius;
    std::uint64_t timestamp_ns; // CLOCK_MONOTONIC, captured after successful read.
};
struct Options {
    std::uint32_t samples = 0;
    std::uint32_t period_ms = 0;
    std::string udp_dest;
    std::string thermal_root = "/sys/class/thermal";
};

// 0 = valid, 1 = help, 2 = invalid. All three required options must be present.
int parse_options(int argc, const char *const argv[], Options &options);
void print_usage();
bool read_cpu_temperature(const std::string &thermal_root, Sample &sample,
                          std::string &error);
std::uint32_t crc32_iso_hdlc(const std::uint8_t *data, std::size_t size) noexcept;
bool encode_packet(const Sample &sample, std::uint64_t sequence,
                   std::array<std::uint8_t, kPacketSize> &packet) noexcept;
int run(const Options &options);

} // namespace orbvi::cpu_temp
