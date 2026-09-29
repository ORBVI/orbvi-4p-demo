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
    Layout layout;
    double scale = 0;
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
    try { write_text(device.path / "buffer/enable", "0"); } catch (...) {}
    for (const auto &saved : device.saved) {
        try { write_text(saved.first, saved.second); } catch (...) {}
    }
}
}

int parse_options(int argc, const char *const *argv, Options &options) {
    options = Options{};
    const std::set<unsigned> rates{25, 50, 100, 200, 500, 1000, 2000};
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
        uint64_t value = 0;
        const uint64_t minimum = !strcmp(arg, "--print-rate-hz") ? 0 : 1;
        if (!number(argv[i], minimum, UINT64_MAX, value)) return 2;
        if (!strcmp(arg, "--sample-rate-hz")) {
            if (!rates.count(static_cast<unsigned>(value))) return 2;
            options.sample_rate_hz = static_cast<unsigned>(value);
        } else if (!strcmp(arg, "--print-rate-hz")) {
            if (value > 2000) return 2;
            options.print_rate_hz = static_cast<unsigned>(value);
        } else if (!strcmp(arg, "--count")) options.count = value;
        else return 2;
    }
    if (options.print_rate_hz > options.sample_rate_hz) return 2;
    return 0;
}

void print_usage() {
    std::puts("ICM-42688 IIO reader\n"
              "Usage: imu_reader_demo [options]\n"
              "  --sample-rate-hz 25|50|100|200|500|1000|2000\n"
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
    std::vector<Device> devices;
    try {
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
        for (const auto &entry : fs::directory_iterator("/sys/bus/iio/devices")) {
            if (entry.path().filename().string().rfind("iio:device", 0)) continue;
            std::string name = read_text(entry.path() / "name");
            if (name != "icm42688-accel" && name != "icm42688-gyro") continue;
            Device device; device.path = entry.path(); device.id = entry.path().filename(); device.name = name;
            if (read_text(device.path / "buffer/enable") != "0") throw std::runtime_error(name + " buffer is busy");
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
            for (const char *axis : {"x", "y", "z"}) {
                const std::string prefix = name == "icm42688-gyro" ? "in_anglvel_" : "in_accel_";
                if (std::none_of(entries.begin(), entries.end(), [&](const ScanEntry &item) { return item.name == prefix + axis; }))
                    throw std::runtime_error("missing IIO axis " + prefix + axis);
            }
            const fs::path scale_path = device.path / (name == "icm42688-gyro" ? "in_anglvel_scale" : "in_accel_scale");
            device.scale = std::stod(read_text(scale_path));
            for (const char *setting : {"sampling_frequency", "buffer/length", "buffer/watermark", "buffer/enable"}) {
                fs::path path = device.path / setting;
                if (fs::exists(path)) device.saved[path] = read_text(path);
            }
            devices.push_back(std::move(device));
        }
        if (devices.size() != 2) throw std::runtime_error("expected separate ICM42688 accel and gyro devices");
        for (auto &device : devices) {
            write_text(device.path / "sampling_frequency", std::to_string(options.sample_rate_hz));
            write_text(device.path / "buffer/length", "256");
            if (fs::exists(device.path / "buffer/watermark")) write_text(device.path / "buffer/watermark", "1");
            for (const auto &entry : device.layout.fields)
                write_text(device.path / "scan_elements" / (entry.name + "_en"), "1");
            device.fd = open((fs::path("/dev") / device.id).c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
            if (device.fd < 0) throw std::runtime_error("cannot open /dev/" + device.id);
        }
        for (auto &device : devices) write_text(device.path / "buffer/enable", "1");
        struct sigaction action{}; action.sa_handler = stop; sigemptyset(&action.sa_mask);
        sigaction(SIGINT, &action, nullptr); sigaction(SIGTERM, &action, nullptr);
        uint64_t emitted = 0, next_print = 0;
        int64_t first_timestamp = 0, last_timestamp = 0;
        while (!stopped && (!options.count || emitted < options.count)) {
            pollfd descriptors[2]{{devices[0].fd, POLLIN, 0}, {devices[1].fd, POLLIN, 0}};
            int ready = poll(descriptors, 2, 1000);
            if (ready < 0 && errno == EINTR) continue;
            if (ready <= 0) throw std::runtime_error("IMU read timeout");
            for (unsigned i = 0; i < 2; ++i) {
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
            size_t pairs = std::min(devices[0].rows.size(), devices[1].rows.size());
            while (pairs--) {
                Row accel = std::move(devices[0].name == "icm42688-accel" ? devices[0].rows.front() : devices[1].rows.front());
                Row gyro = std::move(devices[0].name == "icm42688-gyro" ? devices[0].rows.front() : devices[1].rows.front());
                devices[0].rows.erase(devices[0].rows.begin()); devices[1].rows.erase(devices[1].rows.begin());
                Device &accel_device = devices[0].name == "icm42688-accel" ? devices[0] : devices[1];
                Device &gyro_device = devices[0].name == "icm42688-gyro" ? devices[0] : devices[1];
                const int64_t accel_timestamp_ns = field(accel, "in_timestamp");
                const int64_t gyro_timestamp_ns = field(gyro, "in_timestamp");
                Sample sample;
                sample.timestamp_ns = std::min(accel_timestamp_ns, gyro_timestamp_ns);
                sample.accel_timestamp_ns = accel_timestamp_ns;
                sample.gyro_timestamp_ns = gyro_timestamp_ns;
                for (unsigned axis = 0; axis < 3; ++axis) {
                    const std::string suffix(1, "xyz"[axis]);
                    sample.accel[axis] = field(accel, "in_accel_" + suffix) * accel_device.scale;
                    sample.gyro[axis] = field(gyro, "in_anglvel_" + suffix) * gyro_device.scale;
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
                    next_print += std::max(1u, options.sample_rate_hz / options.print_rate_hz);
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
