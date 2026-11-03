# ORBVI 4P Demo

运行在 ORBVI 4P 板卡上的示例程序，用于相机采集、传感器读取和数据发送，可配合 PC SDK 与 ROS 2 Bridge 使用。

## 功能

- 相机图像：单路 / 四路 RAW 采集、H.264 RTSP 视频及 NV12 / JPEG 图像发送。
- IMU 数据：读取加速度和角速度，通过 UDP 发送到电脑。
- CPU 温度：读取板端温度，通过 UDP 发送到电脑。
- 时钟同步：PPS 与串口 ZDA/RMC 时间报文自动接收、NTP 校时。
- 串口通信：串口收发示例。

## 使用文档

编译、部署、板内演示及与 SDK / ROS 2 的配合使用，请按 [ORBVI 4P 使用指南](https://huanshizhineng.feishu.cn/wiki/GoeUwlTiRiaY5skses2crmPFnCg?fromScene=spaceOverview) 操作。

## UART/PPS 授时

`timesync_demo` 0.1.1 默认自动接收 ZDA/RMC，无需按工装选择协议。默认只读，`--apply` 才允许调钟。接线与使用方法见 [授时 Demo 说明](docs/TIMESYNC_DEMO.md)。

## 外触发测试开发

四路 Trig-Readout 60fps 采集及 4P 与接收设备直连的 RTSP 演示，见 [60fps 使用说明](docs/CAMERA_TRIGGER_60FPS.md)。

## 硬件资料

网口板（4PET A01）是 ORBVI 4P 的以太网接口扩展板，将板卡的以太网信号引出为 RJ45 网口，方便通过网线连接电脑或交换机。

- [网口板原理图（4PET A01，PDF）](hardware/02-4PET-HWA01-20260827-SCH.pdf)

## 源码目录

- [`apps/`](apps/)：板端示例程序。
- [`src/`](src/)：RAW 采集实现。
- [`scripts/`](scripts/)：构建与运行脚本。
- [`build_deps/`](build_deps/)：板端交叉编译依赖。
- [`tests/`](tests/)：测试代码。

## 曝光时间读取

使用 `--read-exposure` 按需打印每路相机的 传感器寄存器换算的曝光总时长、ISP 对照值和估算开始时间，默认关闭，
每路最多每秒查询一次。NV12、分路 RTSP、拼接 RTSP 和 `sensor_demo` 均支持。
开始时间为未经过物理校准的估算值，详细字段、运行示例及时间戳边界见
[曝光时间读取说明](docs/EXPOSURE.md)。
