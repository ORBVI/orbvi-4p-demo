#include "imu_reader.hpp"
#include "validation.hpp"
#include <cassert>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;
using namespace orbvi::imu;
using namespace orbvi::validation;

void write(const fs::path &path, const std::string &value) {
    std::ofstream output(path);
    assert(bool(output));
    output << value;
}

std::string read(const fs::path &path) {
    std::ifstream input(path);
    std::string value;
    std::getline(input, value);
    return value;
}

int create_device(const fs::path &sys, const fs::path &dev, int number, bool accel, bool gyro) {
    const std::string id = "iio:device" + std::to_string(number);
    auto path = sys / id;
    fs::create_directories(path / "scan_elements");
    fs::create_directories(path / "buffer");
    fs::create_directories(dev);
    write(path / "name", "generic-imu-" + std::to_string(number));
    write(path / "buffer/enable", "0");
    write(path / "buffer/length", "77");
    write(path / "buffer/watermark", "5");
    write(path / "sampling_frequency", "500");
    std::vector<ScanEntry> entries;
    for (const auto &channel : {std::pair{std::string("in_accel"), accel},
                                std::pair{std::string("in_anglvel"), gyro}}) {
        if (!channel.second) continue;
        write(path / (channel.first + "_scale"), accel && channel.first == "in_accel" ? "0.5" : "0.25");
        if (channel.first == "in_anglvel") write(path / "in_anglvel_y_offset", "2");
        for (const auto *axis : {"x", "y", "z"}) {
            std::string field = channel.first + "_" + axis;
            entries.push_back({field, "le:s16/16>>0", int(entries.size())});
        }
    }
    entries.push_back({"in_timestamp", "le:s64/64>>0", int(entries.size())});
    for (const auto &entry : entries) {
        write(path / "scan_elements" / (entry.name + "_en"), "0");
        write(path / "scan_elements" / (entry.name + "_type"), entry.type);
        write(path / "scan_elements" / (entry.name + "_index"), std::to_string(entry.index));
    }
    assert(mkfifo((dev / id).c_str(), 0600) == 0);
    int writer = ::open((dev / id).c_str(), O_RDWR | O_NONBLOCK | O_CLOEXEC);
    assert(writer >= 0);
    auto layout = make_layout(entries);
    for (int sequence = 0; sequence < 2; ++sequence) {
        std::vector<uint8_t> bytes(layout.size, 0);
        for (const auto &field : layout.fields) {
            int value = field.name == "in_timestamp" ? 10 + sequence :
                field.name == "in_accel_x" ? 2 : field.name == "in_accel_y" ? 4 :
                field.name == "in_accel_z" ? 6 : field.name == "in_anglvel_x" ? 1 :
                field.name == "in_anglvel_y" ? 2 : 3;
            bytes[field.offset] = static_cast<uint8_t>(value);
        }
        assert(::write(writer, bytes.data(), bytes.size()) == static_cast<ssize_t>(bytes.size()));
    }
    return writer;
}

void capture(const fs::path &root, bool combined) {
    auto sys = root / "sys";
    auto dev = root / "dev";
    int accel = create_device(sys, dev, 0, true, combined);
    int gyro = combined ? -1 : create_device(sys, dev, 1, false, true);
    if (combined) write(sys / "iio:device0/sampling_frequency_available", "100 200 400 800 1600 3200");
    Options options;
    options.count = 2;
    options.print_rate_hz = 1000;
    options.print_metrics = true;
    std::ostringstream output;
    auto previous = std::cout.rdbuf(output.rdbuf());
    int result = run(options, sys, dev);
    std::cout.rdbuf(previous);
    assert(result == 0);
    assert(output.str().find("accel_mps2=[1,2,3] gyro_rps=[0.25,1,0.75]") != std::string::npos);
    assert(output.str().find("SENSOR_IMU_RESULT samples=2") != std::string::npos);
    assert(output.str().find(combined ? "SENSOR_IMU_CONFIG sample_rate_hz=800" :
                                      "SENSOR_IMU_CONFIG sample_rate_hz=500") != std::string::npos);
    for (int number = 0; number < (combined ? 1 : 2); ++number) {
        auto path = sys / ("iio:device" + std::to_string(number));
        assert(read(path / "buffer/enable") == "0");
        assert(read(path / "buffer/length") == "77");
        assert(read(path / "buffer/watermark") == "5");
        assert(read(path / "sampling_frequency") == "500");
        assert(read(path / "scan_elements/in_timestamp_en") == "0");
    }
    ::close(accel);
    if (gyro >= 0) ::close(gyro);
}

int main() {
    char pattern[] = "/tmp/orbvi-imu-iio-XXXXXX";
    const char *created = mkdtemp(pattern);
    assert(created);
    capture(fs::path(created) / "split", false);
    capture(fs::path(created) / "combined", true);
    auto sys = fs::path(created) / "busy/sys";
    auto dev = fs::path(created) / "busy/dev";
    int accel = create_device(sys, dev, 0, true, false);
    int gyro = create_device(sys, dev, 1, false, true);
    write(sys / "iio:device1/buffer/enable", "1");
    Options options;
    assert(run(options, sys, dev) == 1);
    assert(read(sys / "iio:device0/buffer/length") == "77");
    assert(read(sys / "iio:device1/buffer/enable") == "1");
    ::close(accel);
    ::close(gyro);
    sys = fs::path(created) / "unsupported/sys";
    dev = fs::path(created) / "unsupported/dev";
    accel = create_device(sys, dev, 0, true, true);
    write(sys / "iio:device0/sampling_frequency_available", "100 200 400 800 1600");
    options.sample_rate_hz = 1000;
    assert(run(options, sys, dev) == 1);
    assert(read(sys / "iio:device0/sampling_frequency") == "500");
    assert(read(sys / "iio:device0/buffer/enable") == "0");
    ::close(accel);
    options.sample_rate_hz = 0;
    sys = fs::path(created) / "partial/sys";
    dev = fs::path(created) / "partial/dev";
    accel = create_device(sys, dev, 0, true, false);
    gyro = create_device(sys, fs::path(created) / "partial/other-dev", 1, false, true);
    assert(run(options, sys, dev) == 1);
    for (int index = 0; index < 2; ++index) {
        auto path = sys / ("iio:device" + std::to_string(index));
        assert(read(path / "buffer/enable") == "0");
        assert(read(path / "buffer/length") == "77");
        assert(read(path / "sampling_frequency") == "500");
    }
    ::close(accel);
    ::close(gyro);
}
