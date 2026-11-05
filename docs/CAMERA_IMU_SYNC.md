# 四目相机与 IMU 触发同步验收

在 4P 板端完整 Demo 部署目录执行：

```sh
cd /userdata/orbvi-4p-demo
export LD_LIBRARY_PATH=/usr/hobot/lib:/usr/hobot/lib/sensor
ORBVI_SYNC_SECONDS=60 ./orbvi-verify sync ./orbvi-4p-demo
```

先按《部署与准备》释放其他相机、IMU 采集程序；保留 `cam-service`。
验证器独占 GPIO417，启动四路外触发采集，同时读取 IIO IMU 数据。
不需要启动 `imu_reader_demo`，也不要让它与验证器同时占用 IIO buffer。

## 运行结果

终端每秒显示当前阶段、四路实际帧数、各 IIO 设备的采样数和硬件 FSYNC 事件数。
四路图像到达用户态有延迟，运行中的帧数可能暂时比 FSYNC 少一帧；最终判定在停止采集、排空数据后进行。

验收包含前、中、后三个停止触发阶段、3 秒 15 Hz 触发，以及 `ORBVI_SYNC_SECONDS` 秒 30 Hz 触发。
停止触发时四路相机均不得出帧、IMU 不得产生 FSYNC，六轴采样继续进行。
每个触发阶段四路帧数、硬件 FSYNC 事件数必须与实际脉冲数一致，IMU 采样率、连续性和驱动错误统计也必须通过。

全部通过时打印：

```text
PASS: four-camera external-trigger counts, quiet periods, matching accel/gyro FSYNC and timestamps
```

输出保留在开头 `Artifacts:` 指定的独立目录中：

- `summary.json`：最终 `status`、各阶段帧数、FSYNC 数、采样率、时间差与驱动统计。
- `diagnostic.json`：触发记录和包含 FSYNC 元数据的逐条 IMU 样本。
- `capture/camera0.csv` 至 `camera3.csv`：相机逐帧时间戳和帧号。
- `capture.log`：四路采集进程日志。

默认不落盘 RAW 图像；需要核对真实图像文件时运行：

```sh
ORBVI_SYNC_OUTPUT_BASE=/dev/shm ./orbvi-verify sync ./orbvi-4p-demo --raw
```

RAW 验收使用 3 秒 2 Hz、1 秒 30 Hz 阶段，不受 `ORBVI_SYNC_SECONDS` 改变；
会占用数百 MB 存储空间，并检查每个 RAW 文件的大小和图像缓冲区是否为常量。
先用 `df -h /dev/shm` 确认至少约 500 MB 可用；实际占用随启动阶段帧数变化。
本次实板直接写 eMMC 时发生丢帧，使用 RAM 输出后验收通过。原命令的失败结果保留，未放宽计数条件。
`/dev/shm` 在重启后清空，需要保留时把 `Artifacts:` 指定目录复制到持久存储，核对后再重启。

## 接口与能力

Demo 按 IIO 通道发现设备，支持单个六轴设备或同一传感器的分立加速度、陀螺仪设备，不按型号分支。
同步验收要求驱动输出 `in_count0_fsync_event`、`in_count5_fsync_timestamp_ns` 和共享 `sync_stats`。
若有 `fsync_supported` 属性，其值必须为 `1`；有 `current_timestamp_clock` 时必须为 `monotonic`。
失败结果也保留已发现的能力信息，不能通过删除这些检查证明同步。

`summary.json` 的 `configuration` 保存 `timestamp_source`、`timestamp_clock`、`fsync_supported` 和实际 ODR。
驱动负责传感器寄存器与 FIFO 格式；Demo 负责按统一接口采集、关联、验收和展示。

原 `20261102-103133` BSP 的 ICM45686 只提供六轴采集，FSYNC 字段为零。
2026-11-04 的 ICM45686 修复启用 INT2/FSYNC 复用、FIFO FSYNC 标记和延迟计数，沿用原 IIO ABI。
更新应用不会自动更新板端内核；使用原 BSP 时仍会明确拒绝 FSYNC 验收。ICM56686 没有据此宣称通过实板验证。

ICM45686 的 FSYNC 标志和延迟来自传感器，绝对采样时间仍为 `host_estimated`。
通过本测试证明共同触发的事件与四路帧关联，不能据此承诺微秒级绝对时间精度或四路实际曝光窗口重合。
