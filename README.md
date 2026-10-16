# ORBVI 4P Demo

运行在 ORBVI 4P 板卡上的示例程序，用于相机采集、传感器读取和数据发送，可配合 PC SDK 与 ROS 2 Bridge 使用。

## 功能

- 相机图像：单路 / 四路 RAW 采集、H.264 RTSP 视频及 NV12 / JPEG 图像发送。
- IMU 数据：读取加速度和角速度，通过 UDP 发送到电脑。
- CPU 温度：读取板端温度，通过 UDP 发送到电脑。
- 时钟同步：PPS 与串口时间报文接收、NTP 校时。
- 串口通信：串口收发示例。

## 使用文档

编译、部署、板内演示及与 SDK / ROS 2 的配合使用，请按 [ORBVI 4P 使用指南](https://huanshizhineng.feishu.cn/wiki/GoeUwlTiRiaY5skses2crmPFnCg?fromScene=spaceOverview) 操作。

## 硬件资料

网口板（4PET A01）是 ORBVI 4P 的以太网接口扩展板，将板卡的以太网信号引出为 RJ45 网口，方便通过网线连接电脑或交换机。

- [网口板原理图（4PET A01，PDF）](hardware/02-4PET-HWA01-20260827-SCH.pdf)

## 源码目录

- [`apps/`](apps/)：板端示例程序。
- [`src/`](src/)：RAW 采集实现。
- [`scripts/`](scripts/)：构建与运行脚本。
- [`build_deps/`](build_deps/)：板端交叉编译依赖。
- [`tests/`](tests/)：测试代码。
