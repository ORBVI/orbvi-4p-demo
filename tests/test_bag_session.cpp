#include "bag_session.hpp"
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

int main() {
    const auto base = std::filesystem::temp_directory_path() / "orbvi-bag-session-test";
    std::filesystem::remove_all(base); std::filesystem::create_directories(base);
    {
        std::ofstream out(base / "camera0.h264spool", std::ios::binary);
        const uint64_t source = 10, host = 1000000001; const uint32_t size = 4;
        out.write(reinterpret_cast<const char *>(&source), sizeof(source));
        out.write(reinterpret_cast<const char *>(&host), sizeof(host));
        out.write(reinterpret_cast<const char *>(&size), sizeof(size)); out.write("h264", 4);
    }
    {
        std::ofstream out(base / "imu.csv");
        out << "sensor_timestamp_ns,host_timestamp_ns,accel_x_mps2,accel_y_mps2,accel_z_mps2,gyro_x_rps,gyro_y_rps,gyro_z_rps\n"
               "11,1000000000,1,2,3,4,5,6\n";
    }
    const auto bag = base / "run.bag";
    assert(orbvi::sensor::write_rosbag(bag, base, 1, base / "imu.csv") == 0);
    std::ifstream input(bag, std::ios::binary); std::string magic(13, '\0'); input.read(magic.data(), 13);
    assert(magic == "#ROSBAG V2.0\n");
    assert(std::filesystem::file_size(bag) > 4096);
    std::filesystem::remove_all(base);
}
