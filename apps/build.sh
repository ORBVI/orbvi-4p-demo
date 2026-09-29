#!/bin/sh
set -eu
compiler=${CXX:-aarch64-linux-gnu-g++}
flags="-std=c++17 -O2 -Wall -Wextra -Werror -pthread"
mkdir -p dist/bin
$compiler $flags -Iapps/serial apps/serial/main.cpp apps/serial/serial_port.cpp -o dist/bin/serial_port_demo
$compiler $flags -Iapps/timesync apps/timesync/main.cpp apps/timesync/time_sync.cpp -o dist/bin/timesync_demo
$compiler $flags -Iapps/imu -Itools apps/imu/main.cpp apps/imu/imu_reader.cpp apps/imu/imu_wire.cpp tools/validation.cpp -o dist/bin/imu_reader_demo
$compiler $flags -Iapps/sensor -Iapps/camera apps/sensor/main.cpp apps/sensor/sensor_options.cpp apps/sensor/bag_session.cpp apps/sensor/rosbag/rosbag_v2_writer.cpp apps/camera/camera_options.cpp -o dist/bin/sensor_demo
install -m 0755 apps/launchers/serial_port_demo dist/serial_port_demo
install -m 0755 apps/launchers/timesync_demo dist/timesync_demo
install -m 0755 apps/launchers/imu_reader_demo dist/imu_reader_demo
install -m 0755 apps/launchers/sensor_demo dist/sensor_demo
mkdir -p dist/config
cp config/sensor_config.yaml dist/config/sensor_config.yaml
cp VERSION dist/VERSION
install -m 0755 scripts/sync_time.sh dist/sync_time.sh
install -m 0755 scripts/wifi_example.sh dist/wifi_example.sh
mkdir -p dist/licenses
cp apps/sensor/rosbag/LICENSE dist/licenses/Apache-2.0-rosbag-writer.txt
cp apps/sensor/rosbag/SOURCE.md dist/licenses/rosbag-writer-SOURCE.md
apps/camera/build.sh
(cd dist && sha256sum VERSION sync_time.sh wifi_example.sh cam_demo mosaic_rtsp_demo sensor_demo imu_reader_demo serial_port_demo timesync_demo bin/timesync_demo bin/cam_demo bin/cam_group bin/mosaic_rtsp_demo bin/sensor_demo bin/imu_reader_demo bin/serial_port_demo config/sensor_config.yaml licenses/Apache-2.0-rosbag-writer.txt licenses/rosbag-writer-SOURCE.md > manifest.sha256)
