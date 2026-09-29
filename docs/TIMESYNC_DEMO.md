# orbvi-4p 从 N32 模拟 GPS 授时（timesync_demo）

本演示中 **N32G4 是模拟 GPS 时间源，orbvi-4p 是接收端**。N32 PA2 TX → 板端 UART1 RX（`/dev/ttyS1`）；N32 PB1 PPS → 板端 pps-gpio。此 BSP 在本次实板上将该 GPIO 注册为 `/dev/pps1`、`/sys/class/pps/pps1/name=pps.-1`；`/dev/pps0` 是 `ptp0`，程序按 sysfs 来源选择/校验，不假定设备号固定。不得让串口 demo 的 TX 模式同时占用该串口。**即使仅运行默认只读模式，启动前也必须确认 `ntpd` 等进程没有占用 `/dev/ttyS1`、`/dev/pps1`，避免 UART/PPS 多进程争用和误判；若已由 `ntpd` 使用，应先停止本演示，协调设备使用和板钟恢复方案，不要直接抢占设备。**

构建独立程序：`./apps/timesync/build.sh`（`CXX` 可指定交叉编译器，不依赖相机 SDK），或运行完整的 `./apps/build.sh`（还需配置相机/RTSP 构建依赖）；产生 `dist/bin/timesync_demo`、`dist/timesync_demo`，前者或连同后者都可部署。历史 ZDA 版本曾将完整 `apps/build.sh` 构建的 ELF（SHA256 `66018ca1bb6f959d7e99bed32075849d5c5722215b34efa768f8188e31c8ec45`）部署到了板端 `/userdata/orbvi-4p-demo/timesync_demo`；以下命令仅供经设备所有者确认、隔离竞争进程后的实验使用，不应在目前 `ntpd` 正占用串口/PPS 的实板上直接执行；进入该目录后运行：

```sh
# 默认只读，不修改 CLOCK_REALTIME；确认输出连续 LOCKED，先排查源与接线。
./timesync_demo --uart /dev/ttyS1 --pps /dev/pps1
# 只有明确允许测试系统时钟跳变、拥有 CAP_SYS_TIME 且停止竞争的校时服务时才执行：
./timesync_demo --uart /dev/ttyS1 --pps /dev/pps1 --apply
```

预期 N32 从 **2025-01-01 00:00:00 UTC** 起模拟计时，每秒在对应 PPS 后发送带 XOR 校验的 `$GPRMC,hhmmss.00,V,,,,,,,ddmmyy,,,N*XX\r\n`。RMC 字段标准顺序为 `time,status,lat,NS,lon,EW,speed,course,date,magvar,magvarEW,mode`；其中 `V` 是无有效定位，纬度/经度、方向、速度、航向、磁偏角字段均为空，末尾 `N` 是 no fix。**V 不表示可信的外部时间源**；此程序只在实验中把模拟 UTC 与 PPS 配对，不证明其时间准确，也不应以它维持生产板钟。部署时必须先确认 N32 固件实际上已改为输出此格式：旧版 `$GPZDA` 不再接收。`--apply` 会把板端 `CLOCK_REALTIME` 跳到模拟 UTC（例如把 2026 年跳回 2025 年），影响正在运行的采集和依赖绝对时间的服务。测试前暂停相关业务并保存恢复计划。**`--apply` 严禁与 `ntpd` 并行**；程序拒绝在 `ntpd`/`phc2sys`/`chronyd` 等调钟进程运行时启动或继续执行改时，不自行启停服务，不写 RTC，也不向 N32 回传时间。结束测试后恢复板上正确的时间、重启被暂停的服务，并逐项核对业务状态；验收时发现 `ntpd` 已占用 `/dev/ttyS1` 和 `/dev/pps1`，且已把板钟同步到 N32 模拟的 2025 日期；该板钟并非真实 UTC，必须由板端维护人员按恢复计划处理。2026-09-28 已做限时实板验证：短暂停止 `ntpd`、临时烧录 N32 并只读运行接收程序，随后恢复原程序及 `ntpd`；未用 `--apply`。不要依赖离线的模拟源来维护生产板时钟。

**ntpd 限制：** 板上现有 ntpd 的 NMEA/PPS driver 20 会丢弃标为无效的报文；本演示的 `V`/`N` 不宣称 GNSS 有效，因此**不能用它验证 ntpd 的 RMC 锁定或持续授时**。切换 N32 固件会使当前使用旧 ZDA 的 ntpd 时间源可能失锁。只在协调停用串口竞争进程后，以本仓库的 `timesync_demo` 验证 PPS 与 RMC 配对。参见 ntpd 官方 driver 20 文档：https://www.ntp.org/documentation/drivers/driver20/ 。

程序严格校验 RMC 的 12 个字段、2000–2099 年的日期、整秒格式、XOR 校验和及尾随内容；从 Linux PPS API 的 `PPS_FETCH` 读取内核 PPS 上升沿序号与 `CLOCK_REALTIME` 戳，处理启动时刻、串口/PPS 到达顺序和超时。连续 3 组配对为 `LOCKED`，一旦不连续即重新获取。输出 `offset_ms` 是**接收端系统时间相对模拟 UTC 所需的修正量**；`--apply` 使用 PPS 戳和当前系统时间差推算设置目标，而不是将串口报文到达时刻当作整秒。时钟变动或 PPS 过旧时拒绝本次调整。这里仅授时系统 `CLOCK_REALTIME`；相机 SDK/IMU 的硬件原始时间戳不是自动等同于 UTC，也不是硬件级同步。当前实现是实验性的逐秒校时（超过 5ms 才调整），不做 RTC 写入或生产级锁相/频率伺服。

**本次 GPRMC 验收（2026-09-28）：** N32 固件格式化器与 4P 解析器交叉编译并在主机、4P 实板分别运行了模拟报文互通测试，涵盖 2025 起点、闰日、2099 年末，均通过；实板还通过了新 4P 可执行程序的 `--help` 启动检查和部署文件 SHA256 核对。测试文件均已清理，原有 `ntpd` 仍运行。此项先前的**模拟报文互通测试**没有刷写 N32、接收实线 UART 或实线 PPS、运行 `--apply`**，因此不等同于 GPRMC+PPS 硬件联调通过。

主机测试：`./apps/test.sh`，其中 `tests/test_timesync_demo.cpp` 验证 RMC 字段、日期/闰年、校验和、尾随内容、配对不匹配、乱序、丢 PPS、超时及时间插值。以下实板数据均来自**旧 ZDA 版本**，不代表新 RMC 协议已在板端验证：实板接收只读测试连配 8 次；首次 `--apply` 测试在暂停 `ntpd` 和 `phc2sys` 后，第三组配对将约 -54557775.917 秒的偏差归零，随后 6 次 PPS 测得 `offset_ms` 约 +3.75～+3.78ms；这是旧构建将 `/proc` 检测耗时计入设时目标的偏差。优化为临设前重新取样后，**第二次实板 `--apply` 测试**（2026-09-24）：第三组配对改时，输出 `residual_ms=-0.018`；之后 5 个连续 PPS 的 `offset_ms` 为 +0.016、+0.013、+0.010、+0.006、+0.003，均处于毫秒以下。两次测试后均使用 RTC 恢复板钟至 2026-09-24，并重启原来的 `phc2sys` 与 `ntpd`；这些只是现场短时日志，不代表长期精度或真实卫星 UTC。

## 历史旧版 ZDA 完整功能回归（2026-09-24；非本次 RMC 验证）

- 构建容器配置 `HBN_ROOT=/work/.build/x5`、`RTSP_ROOT=/work/build_deps/mk_api`；`./apps/build.sh` 成功，`dist/manifest.sha256` 全部校验通过。`make test`、`make -B all test` 成功（含 `apps/test.sh` 的 ZDA/PPS 单测）。
- 将以上构建产物部署到板上并核验 SHA256。运行中 `ntpd`/`phc2sys` 未停止时，`--apply` 拒绝启动（退出 1），指定 `/dev/pps0` 也被拒绝（退出 1）；两次检查均未修改系统时间。默认只读、自动识别 `/dev/pps1` 时连续匹配 7 个 PPS/ZDA（序号 3671–3677，状态到 `LOCKED`）；板钟仍为 2026 年。
- 暂停 `ntpd`/`phc2sys` 后，在实板上限时运行 `--apply` 11 秒。第 3 次配对（PPS#3724）将系统 `CLOCK_REALTIME` 校至 N32 的模拟 UTC，输出 `residual_ms=-0.003`；之后 8 个 PPS（#3725–3732）输出偏差范围为 **−0.023 至 +0.002 ms**。运行中 `date -u` 显示 2025-01-01，RTC 仍显示 2026-09-24；限时进程退出码 124 是 `timeout` 到时的预期结果，而非授时程序报错。
- 退出后用 RTC 恢复系统时间，重启 `phc2sys` 和 `ntpd`。复查 `date -u` 与 RTC 均显示 2026-09-24，两进程以及原有相机、IMU 进程均在运行；本轮**未验证相机帧或 IMU 原始时钟与 PPS 的硬同步，也未做长期漂移/外部 UTC 精度测试**。

## GPRMC 实线只读验收（2026-09-28）

- N32 临时刷入 GPRMC 固件并读回校验；独占 UART/PPS 的 `ntpd` 停止后，板上 `board_link_test.py` 从 `/dev/ttyS1` 和 `/dev/pps1` 收到连续 7 帧 `V/N` GPRMC，PPS 序号 1692–1698，**7/7 配对、0 不匹配**。
- 板上 `timesync_demo --uart /dev/ttyS1 --pps /dev/pps1` **不带 `--apply`**：连续 9/9 次秒号配对，序号 1710（第 3 组）进入 `LOCKED`，之后保持 `LOCKED`。`timeout -s INT 9` 返回 124 属正常限时退出；没有设置 CLOCK_REALTIME 或 RTC。`offset_ms` 约为 −11,958,117 ms 是**原板模拟时钟**与重启后 N32 固定起点之间的差，不是 UTC 精度测量。
- 刷写前备份的 N32 完整 512 KiB Flash 已逐字节恢复，保留页未变；在重新启用 `ntpd` 前通过旧固件的台架 `@UTC` 输入重建其原本已使用的**2025 模拟时刻**，旧 ZDA 5/5 配对，`ntpq -pn` 再次显示 `o127.127.20.0`，相机、IMU 进程在运行。不要将当前板钟误认为真实 UTC。未测试 `--apply`、GNSS 有效定位、PPS 波形/长期精度及相机/IMU 硬同步。
