#!/bin/sh
set -eu
mkdir -p .build
# A CR on the launcher exec line corrupts the last CLI argument on Buildroot.
if LC_ALL=C grep -q "$(printf '\r')" apps/launchers/mosaic_rtsp_demo; then
  echo 'mosaic launcher must use LF line endings' >&2
  exit 1
fi
sh -n apps/launchers/mosaic_rtsp_demo
./tests/test_sync_time.sh
cxx=${HOST_CXX:-c++}
flags="-std=c++17 -Wall -Wextra -Werror"
$cxx $flags -Iapps/serial tests/test_serial_demo.cpp apps/serial/serial_port.cpp -pthread -o .build/test-serial-demo
.build/test-serial-demo
$cxx $flags -Iapps/timesync tests/test_timesync_demo.cpp apps/timesync/time_sync.cpp -o .build/test-timesync-demo
.build/test-timesync-demo
$cxx $flags -Iapps/imu -Itools tests/test_imu_demo.cpp apps/imu/imu_reader.cpp apps/imu/imu_wire.cpp tools/validation.cpp -o .build/test-imu-demo
.build/test-imu-demo
$cxx $flags -Iapps/imu tests/test_imu_wire.cpp apps/imu/imu_wire.cpp -o .build/test-imu-wire
.build/test-imu-wire
$cxx $flags -Iapps/sensor -Iapps/camera tests/test_sensor_options.cpp apps/sensor/sensor_options.cpp apps/camera/camera_options.cpp -o .build/test-sensor-options
.build/test-sensor-options
$cxx $flags -Iapps/camera tests/test_camera_demo.cpp apps/camera/camera_options.cpp -o .build/test-camera-demo
.build/test-camera-demo
$cxx $flags -Iapps/camera tests/test_nv12_copy.cpp apps/camera/nv12_copy.cpp -o .build/test-nv12-copy
.build/test-nv12-copy
$cxx $flags -Iapps/sensor -Iapps/sensor/rosbag tests/test_bag_session.cpp apps/sensor/bag_session.cpp apps/sensor/rosbag/rosbag_v2_writer.cpp -pthread -o .build/test-bag-session
.build/test-bag-session
$cxx $flags -Iapps/camera tests/test_mosaic_nv12.cpp apps/camera/mosaic_nv12.cpp -pthread -o .build/test-mosaic-nv12
.build/test-mosaic-nv12
$cxx $flags -Iapps/camera tests/test_mosaic_options.cpp apps/camera/mosaic_options.cpp -o .build/test-mosaic-options
.build/test-mosaic-options
$cxx $flags -Iapps/camera tests/test_process_group.cpp apps/camera/process_group.cpp -pthread -o .build/test-process-group
.build/test-process-group

# Camera-only regression: fake media-kit registration/MP4 startup.
$cxx $flags -Iapps/camera -Itests/stubs tests/test_rtsp_recorder.cpp apps/camera/rtsp_server.cpp -pthread -o .build/test-rtsp-recorder
.build/test-rtsp-recorder
