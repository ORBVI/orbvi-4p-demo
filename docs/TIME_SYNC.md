# 板端 NTP 单次校时示例

`dist/sync_time.sh` 是**可选的单次校时示例**：板卡向指定 NTP 服务器查询，
执行一次 `ntpdate -u -b -t 5`，校准板端系统时钟 `CLOCK_REALTIME`。
它不配置网络、不开 NTP 服务、不启停 `ntpd`/PTP、不改时区、不写 RTC、
不设开机任务，也不保证后续持续同步。源码为 `scripts/sync_time.sh`，
构建后复制为 `dist/sync_time.sh`；也可直接部署源码。
校时会跳变板端系统时间，演示前请停止采集和其他依赖绝对时间的程序。

## 本地电脑给板卡授时（推荐演示）

1. 确保电脑与板卡直连网口互通。此前电脑的网口为 `192.168.1.111`、
   板卡为 `192.168.1.10`；以下地址只是示例，以现场为准。
   先在电脑上用 `date -u` 核对其系统时间：NTP 交换表示时间点的时间戳，
   **电脑与板卡不需要使用相同时区**；时区只影响本地时间的显示，
   不影响 NTP 授时的绝对时间。电脑时间不准，板卡也会跟随错误的时间。
2. **在电脑上运行 NTP 服务器**，只对板卡所在网口开放 UDP 123。
   例如在 Linux `chrony` 配置中加入 `allow 192.168.1.10/32`，启动/重载服务，
   检查电脑网口上的 UDP 123、防火墙以及 NTP 服务状态。电脑已有可靠上游时，
   无需本地时钟源；完全离线且仅想让板卡跟随电脑时，可选配置
   `local stratum 10`，但这不保证电脑时间准确。不要对公网开放 NTP 服务。
   电脑在这里是 **NTP 服务器，不是公网 NTP 转发器**。
3. 在演示板上用 `adb shell` 或串口检查环境，并由现场人员决定是否暂停
   当前负责调整系统时钟的服务；以下示例仅在允许中断采集时执行：

```sh
# 宿主机上：先部署示例（首次），再进入板端 shell；也可使用串口
adb -s <4P序列号> shell 'mkdir -p /userdata/orbvi-4p-demo'
adb -s <4P序列号> push dist/sync_time.sh /userdata/orbvi-4p-demo/sync_time.sh
adb -s <4P序列号> shell
# 以下命令在板端执行；先查询偏差，不修改时钟
ntpdate -q -u -t 5 192.168.1.111
# 若 ntpd/phc2sys 正在调系统时钟，按现场方案自行暂停（本 BSP 服务名仅供参考）：
# /etc/init.d/S49ntp stop
# /etc/init.d/S66phc2sys stop
cd /userdata/orbvi-4p-demo
./sync_time.sh 192.168.1.111
date -u
ntpdate -q -u -t 5 192.168.1.111
```

脚本需要板端 root 权限及 `ntpdate`、`pidof`；检测到 `ntpd` 或 `phc2sys`
在运行时会拒绝校时，但不会私自停止或重启服务。若只想查看偏差，
仅运行 `ntpdate -q -u`。恢复 `phc2sys` 后，它可能重新调整系统时钟；
单次校时不代表持续同步。不要让 PTP 与 NTP 同时争夺系统时钟。

## 公网 NTP（客户自行配网）

通过 `adb shell` **在板端**按现场网络环境配置 IP 和网关；使用域名时还需配置 DNS。
可先检查 `ip addr`、`ip route`、`cat /etc/resolv.conf`，然后在板端执行
`ntpdate -q -u -t 5 <公网NTP服务器地址>`，验证目标 NTP 服务器可通过 UDP 123
响应。ADB 是配置通道，**不是**板端通往公网的 NTP 数据通道。
确认网络通畅、时钟主从关系和采集中断安排后，按上例暂停冲突服务，执行
`./sync_time.sh <公网NTP服务器地址>`。本示例不写 RTC；断电后的时间保持和
板端 `ntpd` 长期同步均不在示例范围内。

**时间戳边界：** NTP 只校准板端 `CLOCK_REALTIME`；`imu_reader_demo` UDP 的
`send_timestamp_ns` 使用该时钟，`received_monotonic_ns` 使用
`CLOCK_MONOTONIC`。相机 CSV 的 `sdk_timestamp`/`sdk_sys_timestamp` 保留 SDK 原值，
IMU `in_timestamp` 保留 IIO 原值；不要把这些原始值直接解释为 UTC，或把 NTP
当作相机曝光/IMU FSYNC 硬同步。精密跨设备同步需另行确认时钟域和 PTP/PPS/硬触发。

宿主机无特权回归：`./tests/test_sync_time.sh`（mock NTP，不改系统时间）。

## 实板验证边界

**新版脚本：电脑直连 NTP 实板测试（2026-09-24）。** 板卡为
`192.168.1.10`，电脑直连网口为 `192.168.1.111`。电脑临时在隔离的
Docker 容器中运行 `chronyd -x`（不调整电脑时钟），绑定
`192.168.1.111`，只允许 `192.168.1.10/32`，并配置 `local stratum 10`
提供电脑本地时间。板卡直接通过 UDP 123 向电脑查询，**没有公网转发**；
这验证的是板卡跟随电脑的时间，不证明电脑时间本身准确。

将 `dist/sync_time.sh` 部署为板端
`/userdata/orbvi-4p-demo/sync_time.sh`，部署文件 SHA256 为
`b004fe918212805526af05e64d5959a995e079be94ac1c6e9f3923ca7300214a`。
在 `ntpd`/`phc2sys` 运行时，脚本按预期拒绝校时并退出 1。暂停生产相机/IMU
服务及 `ntpd`、`phc2sys` 后（`ptp4l`、`cam-service` 保持运行），
脚本执行一次校时并退出 0。校时前只读查询偏差为 `+2.020088 s`，
执行校时的 `ntpdate` 报告偏差为 `+2.020326 s`，校时后只读查询为
`+0.000158 s`。脚本没有写 RTC，校时后 RTC 比系统时钟慢约 2 秒。

随后恢复 `phc2sys`、`ntpd` 及生产相机/数据服务，并确认服务状态和
设备信息本地 HTTP API；恢复后短时间再次只读查询的偏差为
`+0.001190 s`。这些只是测试时的查询结果，**不代表长期稳定精度**。
板端没有 `ntpq`，未验证持续 peer lock；也未验收帧内容或录像。
本次日志在板端 `/userdata/orbvi-4p-demo/ntp-local-test.log`。
临时 `chronyd`、HTTP 文件服务及测试 runner 均已停止或移除；
新版脚本保留在板上。电脑 NTP 服务已停止，不能据此认为板卡现在持续同步。

**历史旧版测试（同日，非本次电脑直连测试）：**旧版脚本曾经通过临时公网
NTP 转发完成单次校时及 RTC 写入；该转发已关闭，**不是推荐产品路径**。
旧版日志在板端 `/userdata/orbvi-4p-demo/ntp-test.log`，当时未验证持续
peer lock。旧版测试不作为新版脚本的验收依据。

## N32 串口 + PPS 模拟 GPS 接收

独立的 `timesync_demo` 用 `/dev/ttyS1` 和 pps-gpio 从 N32G4 接收模拟 UTC（`GPRMC` 的 V/N 无定位报文仅用于实验配对，非可信外部时间源），支持只读匹配和显式改系统钟实验；与本页的 NTP 单次校时示例互不依赖。即使只读也须先确认 `ntpd` 未占用 UART/PPS、避免争用；`--apply` 不得与 `ntpd` 并行。接线、测试时间风险、构建和旧版板端结果见 [TIMESYNC_DEMO.md](TIMESYNC_DEMO.md)。
