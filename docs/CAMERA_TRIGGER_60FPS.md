# 四路 60fps 采集与 RTSP 直连演示

60fps 使用 SC132GS Trig-Readout：GPIO417 以 60Hz 输出 1ms 高脉冲，
Camera/MIPI 配置 60fps，sensor VTS=1400，0x3231 bit5 置1。
模式设置在 SDK 初始化后、启动采集前完成，并检查寄存器读回。
程序不写曝光和增益参数，沿用原 AE。

## 构建

已验证构建环境为 Ubuntu 22.04 x86_64、Docker Engine 29.1.3。
宿主机需要 Bash、Git 和可直接访问的 Docker daemon；首次构建镜像需联网
获取 Ubuntu 22.04 及工具链软件包。镜像内为 AArch64 GCC 11.4.0、Make 4.3、
Python 3.10.12，SDK 头文件和链接库来自仓库内的 `build_deps/`。

在仓库根目录执行：

```sh
./build-all.sh
```

生成完整板端 Demo，包含 `dist/mosaic_rtsp_demo`、
`dist/bin/mosaic_rtsp_demo`、`dist/orbvi-4p-demo` 和 `dist/orbvi-verify`。
独立 NV12 入口另在同一镜像中构建，不依赖宿主机安装交叉编译器：

```sh
docker run --rm --platform linux/amd64 --network none \
  --user "$(id -u):$(id -g)" -v "$PWD:/work:rw" -w /work \
  -e HBN_ROOT=/opt/x5 orbvi-4p-demo-builder:standalone sh nv12/build.sh
```

只部署 `dist/` 内可执行文件与启动脚本，保持 RTSP 启动脚本与 `bin/` 的相对位置。
板端使用 HS ORBVI 4P V1.0 对应的 AArch64 BSP，保留 `/usr/hobot/lib`、
`/usr/hobot/lib/sensor` 中的运行库和 sensor 插件；RTSP 还依赖 BSP 的
`libmultimedia.so.1` 及系统 OpenSSL 1.1。可在板端检查依赖解析：

```sh
LD_LIBRARY_PATH=/usr/hobot/lib:/usr/hobot/lib/sensor ldd ./bin/mosaic_rtsp_demo
```

本次目标板检查无缺失库。`build_deps/` 是交叉链接输入，不能覆盖板端运行库。

## 4P 与接收设备直连

用网线连接 4P 和接收设备，给两端配置同一网段的不同地址。
本次实测地址为 4P `192.168.1.10/24`，接收机 `192.168.1.111/24`。
无需 RTSP 转发服务器，接收设备直接访问 4P。

板端保留 `cam-service`，停止占用相机或 GPIO417 的其他采集进程，
在部署目录执行：

```sh
export LD_LIBRARY_PATH=/usr/hobot/lib:/usr/hobot/lib/sensor
./mosaic_rtsp_demo --external --fps 60 --bps 12000 --diagnostics
```

`--external --fps 60` 自动选择 Readout、VTS1400 和 1ms 配组容差，
程序在四路初始化完成后、启动流前自行产生触发，退出时停止脉冲并释放 GPIO。
保留 `--vts`、`--trigger-readout` 和 `--max-skew-ns` 显式设置；
外触发配组容差必须小于触发周期的一半。

接收设备需安装 FFmpeg（本次验证版本 4.4.2），并能够直连板端 TCP 558 端口。
有图形桌面的设备可用 FFplay 打开；无桌面设备使用下方录制命令：

```sh
ffplay -rtsp_transport tcp rtsp://192.168.1.10:558/live/PRR
```

录制命令需在可写目录执行。保存一段视频：

```sh
ffmpeg -rtsp_transport tcp -i rtsp://192.168.1.10:558/live/PRR \
  -t 30 -an -c:v copy preview.mkv
```

录制完成后，检查并完整解码：

```sh
ffprobe -v error -select_streams v:0 -show_entries stream=codec_name,width,height \
  -of default=noprint_wrappers=1 preview.mkv
ffmpeg -v error -i preview.mkv -vsync 0 -f null -
```

输出为四路旋转后 2×2 拼接的 2560×2176 H.264 视频。
`--diagnostics` 输出实际编码帧率、队列丢帧和耗时；RTSP 标称 60fps
不等于接收端实际收到 60fps，需分别检查采集与编码输出。

## RAW / NV12 采集验收

以下限时验证独占 GPIO417，不与 RTSP 演示同时运行。
验证器在 `--fps 60` 时自动选择 Readout/VTS1400，无需实验包装脚本。
输出目录必须可写，运行结束后保存逐帧记录与结果。

```sh
./orbvi-verify trigger ./orbvi-4p-demo --fps 60 --seconds 60
./orbvi-verify trigger-nv12 ./orbvi-nv12-demo --fps 60 --seconds 60
```

## 实板验证（2026-10-26）

设备为 HS ORBVI 4P V1.0，使用板端原有 sensor 插件。

| 路径 | 验证结果 |
|---|---|
| 四路 RAW，60 秒 | 每路 3600 帧，约 60fps，帧号无缺口 |
| 四路 NV12，60 秒 | 3600 个完整四路组，约 60fps，各路帧号无缺口 |
| 直连 RTSP，30 秒录制 | 接收 1318 帧，实际约 44fps；整段解码通过，四路画面可见 |

当前全尺寸拼接的平均耗时约 21.6ms，拼接 RTSP 尚未达到完整 60fps。
RTSP 流头中的 60fps 是配置值，实际输出以接收记录和运行统计为准。

## 时序边界

Trig-Readout 的触发对应读出相关事件，不能直接作为曝光开始时间。
SDK 帧时间戳接近不能替代四路实际曝光窗口测量；相机与 IMU 融合需
单独确认曝光时间与时间戳语义。
