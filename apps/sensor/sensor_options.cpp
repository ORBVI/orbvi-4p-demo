#include "sensor_options.hpp"
#include <arpa/inet.h>
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <set>
#include <vector>

namespace orbvi::sensor {
namespace {
std::string trim(std::string text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}
bool boolean(const std::string &text, bool &value) {
    if (text == "true") { value = true; return true; }
    if (text == "false") { value = false; return true; }
    return false;
}
bool unsigned_number(const std::string &text, unsigned min, unsigned max, unsigned &value) {
    if (text.empty() || text.front() == '-' || text.front() == '+') return false;
    char *end = nullptr; errno = 0;
    const unsigned long parsed = std::strtoul(text.c_str(), &end, 10);
    if (errno || *end || parsed < min || parsed > max) return false;
    value = static_cast<unsigned>(parsed); return true;
}
bool absolute(const std::string &path) { return !path.empty() && path.front() == '/'; }
bool udp_destination(const std::string &text) {
    const auto separator = text.find(':');
    if (separator == std::string::npos || separator == 0 || separator != text.rfind(':')) return false;
    const std::string host = text.substr(0, separator);
    const std::string port_text = text.substr(separator + 1);
    unsigned port = 0;
    in_addr address{};
    if (inet_pton(AF_INET, host.c_str(), &address) != 1) return false;
    const unsigned first_octet = ntohl(address.s_addr) >> 24;
    return first_octet >= 1 && first_octet <= 223 &&
           unsigned_number(port_text, 1, 65535, port);
}
}

int load_yaml(const std::string &path, Options &options) {
    std::ifstream input(path);
    if (!input) {
        std::error_code error;
        const std::filesystem::path config_path(path);
        if (!config_path.parent_path().empty())
            std::filesystem::create_directories(config_path.parent_path(), error);
        if (error) return -1;
        std::ofstream output(path);
        if (!output) return -1;
        output << "# sensor_demo YAML config. Command-line options override this file.\n"
                  "camera:\n  width: 1280\n  height: 1088\n  fps: 30\n  rotate: 0\n"
                  "rtsp:\n  bps: 4000\n  codec: h264\n  url: /live/PRR\n"
                  "imu:\n  sample_rate_hz: 1000\n  print_rate_hz: 10\n  print_metrics: false\n"
                  "  # udp_dest: 192.168.1.100:5600\n"
                  "save_data:\n  save: false\n  format: mp4\n"
                  "  save_path: /root/demo/save_mp4/\n  skip: false\n";
        output.close(); if (!output) return -1;
        input.open(path); if (!input) return -1;
    }
    std::string section, line, save_format = "mp4", save_path = "/root/demo/save_mp4/";
    bool save = false;
    while (std::getline(input, line)) {
        const auto comment = line.find('#');
        if (comment != std::string::npos) line.erase(comment);
        if (trim(line).empty()) continue;
        const bool nested = !line.empty() && std::isspace(static_cast<unsigned char>(line.front()));
        const auto colon = line.find(':');
        if (colon == std::string::npos) return -1;
        std::string key = trim(line.substr(0, colon));
        std::string value = trim(line.substr(colon + 1));
        if (value.empty() && !nested) { section = key; continue; }
        if (nested) {
            if (section.empty()) return -1;
            key = section + "." + key;
        } else section.clear();
        if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                                  (value.front() == '\'' && value.back() == '\'')))
            value = value.substr(1, value.size() - 2);
        bool flag = false;
        if (key == "camera.width") {
            if (!unsigned_number(value, 1, 1280, options.camera.width)) return -1;
        } else if (key == "camera.height") {
            if (!unsigned_number(value, 1, 1280, options.camera.height)) return -1;
        } else if (key == "camera.fps") {
            if (!unsigned_number(value, 1, 60, options.camera.fps)) return -1;
        } else if (key == "camera.rotate") {
            if (!unsigned_number(value, 0, 270, options.camera.rotate)) return -1;
        } else if (key == "rtsp.bps") {
            if (!unsigned_number(value, 1, 100000, options.camera.bitrate_kbps)) return -1;
        } else if (key == "rtsp.codec") {
            if (value == "h264") options.camera.codec = camera::Codec::H264;
            else if (value == "h265") options.camera.codec = camera::Codec::H265;
            else return -1;
        } else if (key == "rtsp.url") {
            if (value.empty() || value.front() != '/') return -1;
            options.camera.url = value;
        } else if (key == "imu.sample_rate_hz") {
            if (!unsigned_number(value, 1, 2000, options.imu_sample_rate_hz)) return -1;
        } else if (key == "imu.print_rate_hz") {
            if (!unsigned_number(value, 0, 2000, options.imu_print_rate_hz)) return -1;
        } else if (key == "imu.print_metrics") {
            if (!boolean(value, options.imu_print_metrics)) return -1;
        } else if (key == "imu.udp_dest") {
            if (!udp_destination(value)) return -1;
            options.imu_udp_dest = value;
        } else if (key == "save_data.save") {
            if (!boolean(value, save)) return -1;
        } else if (key == "save_data.format") {
            if (value != "mp4" && value != "rosbag") return -1;
            save_format = value;
        } else if (key == "save_data.save_path") {
            if (!absolute(value)) return -1;
            save_path = value;
        } else if (key == "save_data.skip") {
            if (!boolean(value, flag)) return -1;
            options.record_frame_skip = flag ? 1 : 0;
        } else return -1;
    }
    if (save) {
        if (save_format == "mp4") options.record_mp4_directory = save_path;
        else options.record_bag_path = save_path;
    }
    return 0;
}

int parse_options(int argc, const char *const *argv, const std::string &config_path,
                  Options &options) {
    options = Options{};
    if (load_yaml(config_path, options)) return 2;
    std::vector<std::string> camera_storage{"cam_demo"};
    bool cli_bag = false, cli_mp4 = false;
    const std::set<std::string> camera_values{
        "--channels", "--camera-id", "--camera-mask", "--width", "--height", "--fps",
        "--bps", "--codec", "--url", "--rtsp-base-port", "--rotate",
        "--diag-interval-ms", "--max-skew-ns", "--frame-timeout-ms", "--trigger-mode"};
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") return 1;
        if (arg == "--diagnostics") { camera_storage.push_back(arg); continue; }
        if (camera_values.count(arg)) {
            if (++i == argc) return 2;
            camera_storage.push_back(arg); camera_storage.emplace_back(argv[i]); continue;
        }
        if (arg == "--print-metrics") { options.imu_print_metrics = true; continue; }
        if (++i == argc) return 2;
        const std::string value = argv[i]; unsigned number = 0;
        if (arg == "--sample-rate-hz") {
            if (!unsigned_number(value, 1, 2000, number)) return 2;
            options.imu_sample_rate_hz = number;
        } else if (arg == "--print-rate-hz") {
            if (!unsigned_number(value, 0, 2000, number)) return 2;
            options.imu_print_rate_hz = number;
        } else if (arg == "--udp-dest") {
            if (!udp_destination(value)) return 2;
            options.imu_udp_dest = value;
        } else if (arg == "--record-bag") {
            if (!absolute(value) || cli_mp4) return 2;
            cli_bag = true; options.record_bag_path = value; options.record_mp4_directory.clear();
        } else if (arg == "--record-mp4-dir") {
            if (!absolute(value) || cli_bag) return 2;
            cli_mp4 = true; options.record_mp4_directory = value; options.record_bag_path.clear();
        } else if (arg == "--record-frame-skip") {
            if (!unsigned_number(value, 0, 1, options.record_frame_skip)) return 2;
        } else return 2;
    }
    std::vector<const char *> camera_argv;
    for (const auto &item : camera_storage) camera_argv.push_back(item.c_str());
    camera::Options parsed_camera = options.camera;
    // camera parser initializes defaults, so replay YAML fields as synthetic CLI first.
    std::vector<std::string> merged{"cam_demo", "--width", std::to_string(options.camera.width),
        "--height", std::to_string(options.camera.height), "--fps", std::to_string(options.camera.fps),
        "--rotate", std::to_string(options.camera.rotate), "--bps", std::to_string(options.camera.bitrate_kbps),
        "--codec", options.camera.codec == camera::Codec::H264 ? "h264" : "h265",
        "--url", options.camera.url};
    merged.insert(merged.end(), camera_storage.begin() + 1, camera_storage.end());
    camera_argv.clear(); for (const auto &item : merged) camera_argv.push_back(item.c_str());
    if (camera::parse_options(static_cast<int>(camera_argv.size()), camera_argv.data(), parsed_camera)) return 2;
    options.camera = std::move(parsed_camera);
    const std::set<unsigned> imu_rates{25, 50, 100, 200, 500, 1000, 2000};
    if (!imu_rates.count(options.imu_sample_rate_hz) || options.imu_print_rate_hz > options.imu_sample_rate_hz)
        return 2;
    if (!options.record_bag_path.empty() && !options.record_mp4_directory.empty()) return 2;
    if (!options.record_bag_path.empty() && options.camera.codec != camera::Codec::H264) return 2;
    if (!options.record_mp4_directory.empty() &&
        (options.camera.codec != camera::Codec::H264 || options.camera.camera_mask != 0xf || options.record_frame_skip))
        return 2;
    if (options.record_frame_skip && options.record_bag_path.empty()) return 2;
    return 0;
}

void print_usage() {
    camera::print_usage();
    std::puts("sensor_demo additions:\n"
              "  --sample-rate-hz HZ\n"
              "  --print-rate-hz HZ\n"
              "  --print-metrics\n"
              "  --udp-dest IPv4:PORT (disabled by default)\n"
              "  --record-bag ABSOLUTE_PATH\n"
              "  --record-mp4-dir ABSOLUTE_DIR\n"
              "  --record-frame-skip 0|1");
}
}
