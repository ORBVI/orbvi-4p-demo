#pragma once

#include <string>

namespace orbvi::sensor {
int write_rosbag(const std::string &bag_path, const std::string &spool_directory,
                 unsigned camera_mask, const std::string &imu_csv_path);
}
