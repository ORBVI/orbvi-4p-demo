#include "imu_reader.hpp"
#include "imu_wire.hpp"
#include "validation.hpp"
#include <algorithm>
#include <cerrno>
#include <cmath>
#include <csignal>
#include <cstring>
#include <filesystem>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <poll.h>
#include <set>
#include <unistd.h>
#include <ctime>
#include <vector>

namespace orbvi::imu {
namespace fs = std::filesystem;
using orbvi::validation::Layout;
using orbvi::validation::Row;
using orbvi::validation::ScanEntry;
namespace {
volatile sig_atomic_t stopped = 0;
void stop(int) { stopped = 1; }
bool number(const char *text, uint64_t min, uint64_t max, uint64_t &value) {
    if (!*text || *text == '-' || *text == '+') return false;
    char *end = nullptr; errno = 0;
    unsigned long long parsed = strtoull(text, &end, 10);
    if (errno || *end || parsed < min || parsed > max) return false;
    value = parsed; return true;
}
std::string read_text(const fs::path &path) {
    std::ifstream input(path); if (!input) throw std::runtime_error("cannot read " + path.string());
    std::string value; std::getline(input, value); return value;
}
void write_text(const fs::path &path, const std::string &value) {
    std::ofstream output(path); if (!output) throw std::runtime_error("cannot write " + path.string());
    output << value; output.close(); if (!output) throw std::runtime_error("cannot update " + path.string());
}
struct Device {
    fs::path path;
    std::string id, name;
    bool accel = false, gyro = false, touched = false;
    Layout layout;
    std::array<double, 3> accel_scale{}, gyro_scale{}, accel_offset{}, gyro_offset{};
    std::vector<fs::path> frequencies;
    int fd = -1;
    std::vector<uint8_t> pending;
    std::vector<Row> rows;
    std::map<fs::path, std::string> saved;
};
int64_t field(const Row &row, const std::string &name) {
    auto found = row.find(name);
    if (found == row.end()) throw std::runtime_error("missing IIO field " + name);
    return found->second;
}
void restore(Device &device) {
    if (device.fd >= 0) { close(device.fd); device.fd = -1; }
    if (!device.touched) return;
    try { write_text(device.path / "buffer/enable", "0"); } catch (...) {}
    for (const auto &saved : device.saved) {
        try { write_text(saved.first, saved.second); } catch (...) {}
    }
}
}

int parse_options(int argc, const char *const *argv, Options &options) {
    options = Options{};
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (!strcmp(arg, "--help") || !strcmp(arg, "-h")) return 1;
        if (!strcmp(arg, "--print-metrics")) { options.print_metrics = true; continue; }
        if (++i == argc) return 2;
        if (!strcmp(arg, "--csv")) { options.csv_path = argv[i]; continue; }
        if (!strcmp(arg, "--udp-dest")) {
            UdpEndpoint endpoint;
            if (!parse_udp_endpoint(argv[i], endpoint)) return 2;
            options.udp_dest = argv[i];
            continue;
        }
        if (!strcmp(arg, "--sample-rate-hz") && !strcmp(argv[i], "auto")) {
            options.sample_rate_hz = 0;
            continue;
        }
        uint64_t value = 0;
        const uint64_t minimum = !strcmp(arg, "--print-rate-hz") ? 0 : 1;
        if (!number(argv[i], minimum, UINT64_MAX, value)) return 2;
        if (!strcmp(arg, "--sample-rate-hz")) {
            if (value > 2000) return 2;
            options.sample_rate_hz = static_cast<unsigned>(value);
        } else if (!strcmp(arg, "--print-rate-hz")) {
            if (value > 2000) return 2;
            options.print_rate_hz = static_cast<unsigned>(value);
        } else if (!strcmp(arg, "--count")) options.count = value;
        else return 2;
    }
    if (options.sample_rate_hz && options.print_rate_hz > options.sample_rate_hz) return 2;
    return 0;
}

void print_usage() {
    std::puts("IIO IMU reader\n"
              "Usage: imu_reader_demo [options]\n"
              "  --sample-rate-hz auto|HZ (default: auto, closest to 1000 Hz)\n"
              "  --print-rate-hz N      Printed samples per second (default: 10)\n"
              "  --count N              Stop after N paired samples (default: unlimited)\n"
              "  --print-metrics        Print final continuity/rate metrics\n"
              "  --csv ABSOLUTE_PATH    Save every paired sample as CSV\n"
              "  --udp-dest IPv4:PORT   Send every sample as OIMU v1 UDP unicast");
}

void print_sample(std::ostream &output, const Sample &sample) {
    output << "ts_ns=" << sample.timestamp_ns << " accel_mps2=[" << sample.accel[0] << ','
           << sample.accel[1] << ',' << sample.accel[2] << "] gyro_rps=[" << sample.gyro[0]
           << ',' << sample.gyro[1] << ',' << sample.gyro[2] << "]\n";
}

int run(const Options &options) {
    return run(options, "/sys/bus/iio/devices", "/dev");
}

int run(const Options &options, const fs::path &sys, const fs::path &dev) {
    std::vector<Device> devices;
    try {
        stopped = 0;
        struct sigaction action{}; action.sa_handler = stop; sigemptyset(&action.sa_mask);
        sigaction(SIGINT, &action, nullptr); sigaction(SIGTERM, &action, nullptr);
        std::unique_ptr<UdpPublisher> udp;
        if (!options.udp_dest.empty()) {
            UdpEndpoint endpoint;
            std::string error;
            if (!parse_udp_endpoint(options.udp_dest, endpoint, &error))
                throw std::runtime_error("invalid --udp-dest: " + error);
            udp = std::make_unique<UdpPublisher>(endpoint);
        }

        std::ofstream csv;
        if (!options.csv_path.empty()) {
            csv.open(options.csv_path, std::ios::out | std::ios::trunc);
            if (!csv) throw std::runtime_error("cannot open IMU CSV " + options.csv_path);
            csv << "sensor_timestamp_ns,host_timestamp_ns,accel_x_mps2,accel_y_mps2,accel_z_mps2,gyro_x_rps,gyro_y_rps,gyro_z_rps\n";
        }
        for (const auto &found : orbvi::validation::find_imu_devices(sys)) {
            Device device; device.path = found.path; device.id = found.id; device.name = found.name;
            device.accel = found.accel; device.gyro = found.gyro;
            if (read_text(device.path / "buffer/enable") != "0") throw std::runtime_error(device.name + " buffer is busy");
            std::vector<ScanEntry> entries;
            for (const auto &scan : fs::directory_iterator(device.path / "scan_elements")) {
                std::string filename = scan.path().filename();
                if (filename.size() < 3 || filename.substr(filename.size() - 3) != "_en") continue;
                std::string field_name = filename.substr(0, filename.size() - 3);
                entries.push_back({field_name, read_text(device.path / "scan_elements" / (field_name + "_type")),
                                   std::stoi(read_text(device.path / "scan_elements" / (field_name + "_index")))});
                device.saved[scan.path()] = read_text(scan.path());
            }
            device.layout = orbvi::validation::make_layout(entries);
            if (std::none_of(entries.begin(), entries.end(), [](const ScanEntry &item) { return item.name == "in_timestamp"; }))
                throw std::runtime_error("missing IIO timestamp for " + device.name);
            for (unsigned axis = 0; axis < 3; ++axis) {
                const std::string suffix(1, "xyz"[axis]);
                for (const auto &channel : {std::pair{device.accel, std::string("in_accel")},
                                            std::pair{device.gyro, std::string("in_anglvel")}}) {
                    if (!channel.first) continue;
                    auto &scale = channel.second == "in_accel" ? device.accel_scale : device.gyro_scale;
                    auto &offset = channel.second == "in_accel" ? device.accel_offset : device.gyro_offset;
                    scale[axis] = orbvi::validation::iio_axis_value(device.path, channel.second, suffix,
                        "scale", std::numeric_limits<double>::quiet_NaN());
                    offset[axis] = orbvi::validation::iio_axis_value(device.path, channel.second, suffix, "offset", 0.0);
                    if (scale[axis] <= 0) throw std::runtime_error("invalid IIO scale for " + device.name);
                }
            }
            if (fs::exists(device.path / "sampling_frequency")) {
                device.frequencies.push_back(device.path / "sampling_frequency");
            } else {
                if (device.accel) device.frequencies.push_back(device.path / "in_accel_sampling_frequency");
                if (device.gyro) device.frequencies.push_back(device.path / "in_anglvel_sampling_frequency");
            }
            for (const auto &frequency : device.frequencies) device.saved[frequency] = read_text(frequency);
            for (const char *setting : {"buffer/length", "buffer/watermark", "buffer/enable"}) {
                fs::path path = device.path / setting;
                if (fs::exists(path)) device.saved[path] = read_text(path);
            }
            devices.push_back(std::move(device));
        }
        std::vector<fs::path> frequencies;
        for (const auto &device : devices)
            frequencies.insert(frequencies.end(), device.frequencies.begin(), device.frequencies.end());
        const unsigned sample_rate = orbvi::validation::choose_iio_rate(frequencies, options.sample_rate_hz);
        for (auto &device : devices) {
            device.touched = true;
            for (const auto &frequency : device.frequencies) {
                write_text(frequency, std::to_string(sample_rate));
                if (std::stod(read_text(frequency)) != sample_rate)
                    throw std::runtime_error("IIO sampling frequency did not match request: " + frequency.string());
            }
            write_text(device.path / "buffer/length", "256");
            if (fs::exists(device.path / "buffer/watermark")) write_text(device.path / "buffer/watermark", "1");
            for (const auto &entry : device.layout.fields)
                write_text(device.path / "scan_elements" / (entry.name + "_en"), "1");
            device.fd = open((dev / device.id).c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
            if (device.fd < 0) throw std::runtime_error("cannot open /dev/" + device.id);
        }
        for (auto &device : devices) write_text(device.path / "buffer/enable", "1");
        std::cout << "SENSOR_IMU_CONFIG sample_rate_hz=" << sample_rate << '\n';
        uint64_t emitted = 0, next_print = 0;
        int64_t first_timestamp = 0, last_timestamp = 0;
        const size_t accel_index = 0;
        const size_t gyro_index = devices[0].gyro ? 0 : 1;
        while (!stopped && (!options.count || emitted < options.count)) {
            std::vector<pollfd> descriptors;
            for (const auto &device : devices) descriptors.push_back({device.fd, POLLIN, 0});
            int ready = poll(descriptors.data(), descriptors.size(), 1000);
            if (ready < 0 && errno == EINTR) continue;
            if (ready <= 0) throw std::runtime_error("IMU read timeout");
            for (size_t i = 0; i < devices.size(); ++i) {
                if (!(descriptors[i].revents & POLLIN)) continue;
                uint8_t bytes[65536]; ssize_t size = read(devices[i].fd, bytes, sizeof(bytes));
                if (size <= 0) continue;
                auto &pending = devices[i].pending;
                pending.insert(pending.end(), bytes, bytes + size);
                size_t complete = pending.size() / devices[i].layout.size * devices[i].layout.size;
                for (size_t offset = 0; offset < complete; offset += devices[i].layout.size)
                    devices[i].rows.push_back(devices[i].layout.decode(pending.data() + offset));
                pending.erase(pending.begin(), pending.begin() + complete);
            }
            size_t pairs = gyro_index == accel_index ? devices[accel_index].rows.size() :
                std::min(devices[accel_index].rows.size(), devices[gyro_index].rows.size());
            while (pairs--) {
                Device &accel_device = devices[accel_index];
                Device &gyro_device = devices[gyro_index];
                Row accel = std::move(accel_device.rows.front());
                accel_device.rows.erase(accel_device.rows.begin());
                Row gyro;
                if (gyro_index == accel_index) gyro = accel;
                else {
                    gyro = std::move(gyro_device.rows.front());
                    gyro_device.rows.erase(gyro_device.rows.begin());
                }
                const int64_t accel_timestamp_ns = field(accel, "in_timestamp");
                const int64_t gyro_timestamp_ns = field(gyro, "in_timestamp");
                Sample sample;
                sample.timestamp_ns = std::min(accel_timestamp_ns, gyro_timestamp_ns);
                sample.accel_timestamp_ns = accel_timestamp_ns;
                sample.gyro_timestamp_ns = gyro_timestamp_ns;
                for (unsigned axis = 0; axis < 3; ++axis) {
                    const std::string suffix(1, "xyz"[axis]);
                    sample.accel[axis] = (field(accel, "in_accel_" + suffix) + accel_device.accel_offset[axis]) * accel_device.accel_scale[axis];
                    sample.gyro[axis] = (field(gyro, "in_anglvel_" + suffix) + gyro_device.gyro_offset[axis]) * gyro_device.gyro_scale[axis];
                }
                if (!first_timestamp) first_timestamp = sample.timestamp_ns;
                timespec host_now{};
                if (clock_gettime(CLOCK_REALTIME, &host_now)) throw std::runtime_error("clock_gettime failed");
                const uint64_t host_timestamp_ns = static_cast<uint64_t>(host_now.tv_sec) * 1000000000ULL + host_now.tv_nsec;
                last_timestamp = sample.timestamp_ns;
                if (udp) udp->publish(sample, host_timestamp_ns);
                // An unopened ofstream can still have a good stream state.
                // Only write when --csv explicitly opened a file.
                if (csv.is_open()) {
                    csv << sample.timestamp_ns << ',' << host_timestamp_ns;
                    for (double value : sample.accel) csv << ',' << value;
                    for (double value : sample.gyro) csv << ',' << value;
                    csv << '\n';
                    if (!csv) throw std::runtime_error("write IMU CSV failed");
                }
                if (options.print_rate_hz && emitted >= next_print) {
                    print_sample(std::cout, sample);
                    next_print += std::max(1u, sample_rate / options.print_rate_hz);
                }
                ++emitted;
                if (options.count && emitted >= options.count) break;
            }
        }
        if (options.print_metrics) {
            double hz = emitted > 1 && last_timestamp > first_timestamp
                ? double(emitted - 1) * 1e9 / double(last_timestamp - first_timestamp) : 0;
            std::cout << "SENSOR_IMU_RESULT samples=" << emitted << " effective_hz=" << hz << '\n';
            if (udp) {
                const UdpMetrics &metrics = udp->metrics();
                std::cout << "SENSOR_IMU_UDP_RESULT samples=" << metrics.samples
                          << " sent=" << metrics.sent << " dropped=" << metrics.dropped << '\n';
            }
        }
        for (auto &device : devices) restore(device);
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "imu_reader_demo: " << error.what() << '\n';
        for (auto &device : devices) restore(device);
        return 1;
    }
}
}
