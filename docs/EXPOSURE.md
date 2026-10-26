# 按需读取曝光时间

`--read-exposure` 默认关闭。开启后，各路相机最多每秒读取一次 SC132GS 曝光寄存器，
同时查询 ISP 对照参数，分别打印寄存器换算的曝光总时长和估算开始时间。
它独立于 `--diagnostics`：只开启性能诊断不会启动本功能。

## 运行

拼接 RTSP（程序自行产生外触发）：

```sh
./mosaic_rtsp_demo --external --fps 60 --read-exposure
```

分路 RTSP、相机与 IMU 的组合入口也支持同一开关：

```sh
./cam_demo --read-exposure
./sensor_demo --read-exposure
```

独立 NV12 自由运行采集：

```sh
./orbvi-nv12-demo --frames 120 --discard --read-exposure --output exposure-capture
```

NV12 的 `--mode external` 仍需按既有方式提供触发。纯 RAW 的 `orbvi-4p-demo` 没有
ISP 节点，不提供此开关。一个时刻只运行一个占用相机的采集程序。

## 曝光时长的来源

SC132GS 数据手册 V3.1 的普通曝光单位是 1/16 行：

```text
C = ((R[0x3e00] & 0x0f) << 16) | (R[0x3e01] << 8) | R[0x3e02]
exposure_lines = C / 16.0
exposure_duration_us = exposure_lines × 1e6 / lines_per_second
```

程序直接通过 I2C 读取当前寄存器；连续两次编码相同才接受，最多尝试三对读取。
四路使用现有 4P 接线映射，并核验芯片 ID。行频来自 `hbn_isp_get_lines_persecond()`，
成功后缓存；失败或返回零时重新查询，不使用硬编码行频或 ISP 曝光值冒充寄存器时长。
这是按寄存器设置和驱动上报行频换算的时长，不是 STROBE/光学积分脉宽的独立实测。

每次输出两行。例如当前设置可能是：

```text
camera0: exposure_status=ok exposure_duration_us=9976.190 duration_source=sensor_registers sensor_rc=0 exposure_code=0x03460 exposure_lines=838.0000 lines_per_second=84000 line_rate_rc=0 line_rate_source=isp_reported isp_exposure_us=10000.000 isp_rc=0 sample_scope=register_snapshot
camera0: exposure_start_ns=unavailable exposure_start_estimated_ns=1990000000 start_status=unverified_estimate start_model=isp_timestamp_minus_isp_duration clock=isp_frame_timestamp isp_timestamp_ns=2000000000 isp_frame_id=101 image_frame_id=100 isp_frame_match=0 sensor_frame_match=unverified
```

| 字段 | 含义 |
|---|---|
| `camera0`…`camera3` | 相机端口编号 |
| `exposure_duration_us` | 当前 sensor 曝光寄存器换算的积分持续时间，单位 µs |
| `duration_source=sensor_registers` | 主时长来自传感器寄存器，不是 ISP 请求/上报值 |
| `exposure_code`、`exposure_lines` | 原始 20-bit 曝光编码、包含 1/16 行小数的行数 |
| `lines_per_second` | 驱动上报行频，不代表本功能已独立校准行时间 |
| `isp_exposure_us` | 单独保留的 ISP 上报时长，用于比较 |
| `sensor_rc`、`line_rate_rc`、`isp_rc` | 三条读取路径的返回码，0 为成功 |
| `sample_scope=register_snapshot` | 当前寄存器快照，没有图像帧号归属 |
| `exposure_start_ns=unavailable` | 实际积分开始边沿尚未测定 |
| `exposure_start_estimated_ns` | ISP 时间戳减去同次 ISP 上报时长的未校准估算值 |
| `start_model`、`clock` | 估算使用 ISP 参数及其时间戳域，不宣称是 Unix/UTC/北京时间 |
| `isp_frame_id`、`image_frame_id` | ISP 曝光参数所带帧号与当前取到的图像帧号 |
| `isp_frame_match` | 两者帧号及时间戳是否相同，不证明实际曝光生效帧 |
| `sensor_frame_match=unverified` | 寄存器快照没有经过逐帧归属验证 |

寄存器值可能因 AE 更新而变化，重复读取可发现部分不稳定更新，但不构成帧级原子快照。
本功能不把寄存器快照绑定到 ISP 或当前图像帧，也不使用该快照和其他帧的时间戳拼成起点。

实板曾出现 ISP 上报 840 行、寄存器读回 838 行的差异，因此两个来源明确分开。
不擅自给寄存器值加回两行，也不宣称已查清该差异的物理含义。
真实起止时间需结合 LEDSTROBE/光学参考及公共时基；所有模式下的开始时间估算均未物理校准。

## 错误与开销

传感器读取失败、不稳定或行频不可用时，主时长输出 `unavailable`，仍单独显示可用 ISP 值；
ISP 查询失败时，寄存器读取和时长换算仍继续。任一路诊断失败均不导致采集退出，
也不复用上次的成功值。ISP 曝光无效、时间戳缺失或减法下溢时，不生成开始时间估算。

读取在各相机原有采集线程内完成，不增加后台线程；限频使用 `CLOCK_MONOTONIC`。
关闭时不执行本功能的 ISP 查询、行频查询、曝光寄存器读取及限频时钟读取。
现有初始化阶段的 VTS/模式读回校验照常执行。
开启时不修改曝光、增益、AE 状态、触发模式或 sensor 寄存器，不改变图像/CSV/RTSP 格式。
相比只查 ISP 的旧版，I2C 读取增加调用耗时，应以本版实板结果评估性能。

## 验证

```sh
sh nv12/test.sh
make test
```

主机覆盖四路 I2C 接线、ID/读取失败、有限重试、只读保证、1/16 行编码、
默认关闭、限频、各数据源独立失败、缺失时间戳、下溢及未验证的帧归属。

## 寄存器读取版实板验证（2026-10-26）

HS ORBVI 4P V1.0 上，NV12 默认关闭与开启各运行 20 秒，四路均各 1200 帧，约 60fps，
帧号无缺口。关闭时 ISP 曝光诊断查询为 0；开启时包含预热共 84 次，每路 21 次。
四路稳态寄存器编码均为 `0x03460`，838 行，上报行频 84000 行/秒，换算 9976.190µs；
ISP 对照值为 10000.000µs。各次寄存器读取均成功。

将 ISP 曝光查询注入为 -99 后运行 5 秒，四路各 300 帧，帧号无缺口；寄存器时长仍能输出，
ISP 值及起点估算显示 unavailable。拼接 RTSP 开关各做 12 秒限时验证，均正常退出；
开启时四路共 40 次读回，关闭时曝光诊断查询为 0。上述为短时验证结果。

本轮未测量物理积分边沿，也未证明寄存器快照与图像帧的生效对应关系。
