#include "validation.hpp"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>

using namespace orbvi::validation;
namespace fs = std::filesystem;

template<class F> void rejects(F fn) {
    bool caught = false;
    try { fn(); } catch (const std::exception &) { caught = true; }
    assert(caught);
}

void device(const fs::path &path, const std::string &name, bool accel, bool gyro) {
    fs::create_directories(path / "scan_elements");
    std::ofstream(path / "name") << name;
    for (const auto &channel : {std::pair{"in_accel_", accel}, std::pair{"in_anglvel_", gyro}}) {
        if (!channel.second) continue;
        for (const auto *axis : {"x", "y", "z"})
            std::ofstream(path / "scan_elements" / (std::string(channel.first) + axis + "_en")) << '0';
    }
}

int main() {
    char pattern[] = "/tmp/orbvi-iio-discovery-XXXXXX";
    const char *created = mkdtemp(pattern);
    assert(created);
    fs::path root(created);
    auto split = root / "split";
    device(split / "iio:device4", "new-accel", true, false);
    device(split / "iio:device8", "new-gyro", false, true);
    auto devices = find_imu_devices(split);
    assert(devices.size() == 2 && devices[0].accel && devices[1].gyro);
    auto combined = root / "combined";
    device(combined / "iio:device2", "new-imu", true, true);
    devices = find_imu_devices(combined);
    assert(devices.size() == 1 && devices[0].accel && devices[0].gyro);
    device(split / "iio:device9", "extra-accel", true, false);
    rejects([&]{ find_imu_devices(split); });
    auto missing = root / "missing";
    device(missing / "iio:device1", "accel-only", true, false);
    rejects([&]{ find_imu_devices(missing); });
    auto separate = root / "separate";
    fs::create_directories(separate);
    device(root / "chip-a" / "iio:device1", "accel", true, false);
    device(root / "chip-b" / "iio:device2", "gyro", false, true);
    fs::create_directory_symlink(root / "chip-a" / "iio:device1", separate / "iio:device1");
    fs::create_directory_symlink(root / "chip-b" / "iio:device2", separate / "iio:device2");
    rejects([&]{ find_imu_devices(separate); });
    auto path = combined / "iio:device2";
    std::ofstream(path / "in_accel_scale") << "0.1\n";
    std::ofstream(path / "in_accel_x_scale") << "0.2\n";
    assert(iio_axis_value(path, "in_accel", "x", "scale", NAN) == 0.2);
    assert(iio_axis_value(path, "in_accel", "y", "scale", NAN) == 0.1);
    assert(iio_axis_value(path, "in_accel", "x", "offset", 0.0) == 0.0);
    rejects([&]{ iio_axis_value(path, "in_anglvel", "x", "scale", NAN); });
    std::ofstream(path / "in_accel_x_scale") << "0.2invalid\n";
    rejects([&]{ iio_axis_value(path, "in_accel", "x", "scale", NAN); });
    auto frequency = path / "sampling_frequency";
    std::ofstream(frequency) << "800\n";
    std::ofstream(path / "sampling_frequency_available") << "12.5 25 800 1600 3200\n";
    assert(choose_iio_rate({frequency}, 0) == 800);
    assert(choose_iio_rate({frequency}, 1600) == 1600);
    rejects([&]{ choose_iio_rate({frequency}, 1000); });
    std::ofstream(path / "sampling_frequency_available") << "12.5 -10 800\n";
    rejects([&]{ choose_iio_rate({frequency}, 0); });
}
