# BSP-native C++ demos

The `apps/` programs mirror the public RoboBaton 4P non-ROS demo entry points
while using the ORBVI X5 BSP directly. They do not link RoboBaton's
`libsc132`, `libicm42688`, or `libprrtsp` binaries.

## Build

Set `RTSP_ROOT` to the BSP `sunrise_camera/Transport/media_server/mk_api`
directory and `HBN_ROOT` to the matching X5 SDK sysroot, then run:

```sh
./apps/test.sh
./apps/build.sh
```

The build creates ARM64 executables under `dist/bin` and launchers plus the
default YAML under `dist/`. ZLMediaKit is statically linked from the BSP;
camera capture and encoding dynamically use the board's `libcam`, `libvpf`,
`libhbmem`, and `libmultimedia`. The board's own `libsc132gs.so` sensor plugin
is selected indirectly by `hbn_camera_create("sc132gs")`.

## Run

```sh
./cam_demo
./mosaic_rtsp_demo --diagnostics
./imu_reader_demo
./serial_port_demo --port /dev/ttyS1 --mode txrx
./sensor_demo
./sensor_demo --record-mp4-dir /data/mp4_session
./sensor_demo --record-bag /data/run.bag
```

`mosaic_rtsp_demo` independently captures four NV12 camera frames, matches timestamps within a configurable window, rotates each portrait frame clockwise into a 2x2 2560x2176 NV12 canvas (camera0/camera1 above camera2/camera3), and publishes one H.264 RTSP stream at `rtsp://<board-ip>:558/live/PRR`. It uses the board BSP encoder and ZLMediaKit, without the Hessian binary libraries. The default grouping window is 40 ms for free-running visual preview, **not** a 1 ms sync guarantee. Use `--max-skew-ns` to tighten it. The BSP ZLMediaKit path contains an application component (`/live/PRR`), unlike Hessian's `/PRR`. Do not run it concurrently with `cam_demo`, `sensor_demo`, or the production camera pipeline. The 2560x2176 encoder throughput, latency, orientation and four-camera stability still require board acceptance. The mosaic path uses three reusable NV12 buffers and a dedicated encode/RTSP worker. Inspired by the production UVC pipeline's per-camera transformation, four portrait tiles are rotated concurrently by the caller plus three persistent CPU workers; `--serial-compose` falls back to the original sequential CPU implementation for A/B comparison. A single pending frame is kept, replacing an older pending frame (never the encoder's in-flight buffer); when encoding is slower than composition, recently redundant groups can be discarded *before* the costly rotation. The SDK camera leases are held until all four tile workers complete. **This does not use UVC's Nano2D/DMA/JPEG path**: the production implementation warns that direct full-size NV12 Nano2D rotation may hang X5, so we do not enable it without separate board validation. `--diagnostics` prints measured `actual_fps`, cumulative `dropped_queue`, `dropped_precompose`, `compose_avg_ms` and `encode_push_avg_ms` every ~2 seconds and at exit. The encode/RTSP metric includes input-buffer copy and codec wait; compare the same scene, FPS, bitrate and RTSP client before claiming improvement. Lower queue latency can increase frame drops without increasing encode throughput.

`cam_demo` publishes four H.264 streams at ports 554 through 557 and path
`/live/PRR` by default. ZLMediaKit RTSP paths must contain both an application
and a stream component (`/APP/STREAM`). Camera frames enter the application as two-plane NV12 from
the BSP ISP/VSE. A process-shared timestamp barrier holds one borrowed VSE frame per
enabled camera, drops only the oldest frame when skew exceeds `--max-skew-ns`,
and releases a complete NV12 frame group to the per-camera encoders together.
Supported public frame rates are 25/30/40/50/60 and supported
output canvases are 1280x1088, 640x480, 720x480, and 1280x720.

MP4 mode stores one BSP ZLMediaKit recording per camera, per-frame timestamp
indexes, and `imu.csv`. Bag mode produces one indexed ROSBAG V2.0 containing
four `sensor_msgs/CompressedImage` topics and `/imu/data`. In this BSP-native
implementation the compressed image `format` is `h264`; the reference package
uses JPEG for bag images. `--record-frame-skip 1` applies only to bag recording.

For free-running four-camera capture, use `--trigger-mode none --max-skew-ns 40000000` when the default 1 ms grouping window rejects frames. The 40 ms tolerance is **not** a 1 ms hardware synchronization guarantee. Validate all four decoded RTSP streams (`:554`–`:557/live/PRR`), not merely open ports. MP4 recording may need a short media-registration wait; a requested recording now fails visibly rather than continuing without a recorder.

The new NV12/RTSP/recording path has host regression and ARM64 cross-build
coverage but still requires board acceptance for visible image orientation,
all four RTSP URLs, hardware encoder rotation, long-run frame continuity,
MP4 finalization, bag replay, IMU timestamp pairing, shutdown, and reopen.

## IMU UDP 单播（可选）

板端 `imu_reader_demo` 可用 `--udp-dest <接收机IPv4>:<端口>` 发送 IMU；
`sensor_demo` 接受相同参数，也可在 `config/sensor_config.yaml` 中设置
`imu.udp_dest`。未设置时不创建 UDP socket，终端打印、CSV、ROSBAG 维持原行为。
启用 UDP 时这些已有输出仍可同时使用，例如：

```sh
./imu_reader_demo --udp-dest 192.168.1.10:5600 --print-metrics
./sensor_demo --udp-dest 192.168.1.10:5600 --print-metrics
```

每个配对采样一个固定 72 字节的 `OIMU/1` datagram，包含序号、加速度及陀螺仪原始时间戳、发送时间、六轴 SI 数据及 CRC32；字段布局和解析约束见 `docs/IMU_UDP_V1.md`。网络发送不阻塞采集；发送统计仅表示本机发送系统调用结果，不能证明 PC 已收到。实际端到端接收及板端 IIO 独占需上板联调验证。
