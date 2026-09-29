# IMU UDP OIMU/1 协议

`imu_reader_demo` 在显式提供 `--udp-dest IPv4:PORT` 时，会把每个配对采样编码为一个固定 72 字节的 UDP 单播 datagram。未指定该参数时不会创建网络 socket，采样、打印和 CSV 行为与原有版本保持一致。

## 命令行

```text
--udp-dest IPv4:PORT
```

- 只接受数值 IPv4 地址和十进制端口。
- 解析器不查询接口掩码，以首字节 `1..223` 作为保守限制，保留 `127.0.0.0/8` 供本机测试。
- 拒绝未指定地址 `0.0.0.0`、组播 `224.0.0.0/4`、有限广播 `255.255.255.255` 和保留地址 `240.0.0.0/4`。
- 其他地址不保证是可用的单播目标；例如 `192.168.1.255` 这类依赖子网掩码的 directed broadcast 无法由解析器判定，部署时必须避开。
- 端口范围为 `1..65535`。
- 不接受域名、IPv6、端口 `0` 或端口 `65536`。
- 未显式指定 `--udp-dest` 时不发送 UDP。
- 端点解析失败会在打开 IIO、配置 buffer 或其他采集动作之前返回错误。

示例：

```sh
./imu_reader_demo --udp-dest 192.168.1.20:5000 --count 100 --print-metrics
```

UDP socket 使用 `O_NONBLOCK` 创建，发送不等待接收端。发送成功只表示本地 UDP 栈接受了该 datagram，不表示接收端已收到；本协议不提供 ACK、重传或应用层确认。

## Datagram 布局

所有整数和 float 位模式都使用网络字节序。协议固定为 OIMU/1，长度固定为 72 字节。

| Offset | Size | 字段 | 说明 |
| ---: | ---: | --- | --- |
| 0 | 4 | magic | ASCII `OIMU`（`4f 49 4d 55`） |
| 4 | 1 | `version` | `u8`，固定为 `1` |
| 5 | 1 | `type` | `u8`，固定为 `1` |
| 6 | 2 | `total_length` | `u16`，固定为 `72` |
| 8 | 8 | `sequence` | `u64`，按采样发送尝试递增，初值为 `0` |
| 16 | 8 | `accel_timestamp_ns` | `u64`，加速度 IIO `in_timestamp` 原始值，单位 ns |
| 24 | 8 | `gyro_timestamp_ns` | `u64`，角速度 IIO `in_timestamp` 原始值，单位 ns |
| 32 | 8 | `send_timestamp_ns` | `u64`，板端 `CLOCK_REALTIME` 发送时间，单位 ns |
| 40 | 4 | `flags` | `u32`，当前固定为 `0` |
| 44 | 4 | `accel_x_mps2` | `float32` 位模式，m/s² |
| 48 | 4 | `accel_y_mps2` | `float32` 位模式，m/s² |
| 52 | 4 | `accel_z_mps2` | `float32` 位模式，m/s² |
| 56 | 4 | `gyro_x_rps` | `float32` 位模式，rad/s |
| 60 | 4 | `gyro_y_rps` | `float32` 位模式，rad/s |
| 64 | 4 | `gyro_z_rps` | `float32` 位模式，rad/s |
| 68 | 4 | `crc32` | `u32`，覆盖 offset `0..67` |

本地打印和 CSV 仍使用原有字段；`Sample.timestamp_ns` 继续是 accel/gyro 原始时间戳的较小值。线协议使用两个独立原始时间戳，便于接收端自行处理配对偏差。

## CRC32/ISO-HDLC

CRC 参数：

- width: 32
- polynomial: `0x04c11db7`，反射实现多项式 `0xedb88320`
- initial value: `0xffffffff`
- refin/refout: true
- xorout: `0xffffffff`
- check: `"123456789"` -> `0xcbf43926`

CRC 字段在 datagram 中按网络字节序写入，覆盖字节 `0..67`，不包含 CRC 字段自身。

## Golden vector

测试向量：

- sequence: `0x0102030405060708`
- accel timestamp: `0x1112131415161718`
- gyro timestamp: `0x2122232425262728`
- send timestamp: `0x3132333435363738`
- accel: `{1.0, -2.5, 3.25}` m/s²
- gyro: `{-4.0, 5.5, -6.75}` rad/s
- flags: `0`

完整 72 字节：

```text
4f 49 4d 55 01 01 00 48 01 02 03 04 05 06 07 08
11 12 13 14 15 16 17 18 21 22 23 24 25 26 27 28
31 32 33 34 35 36 37 38 00 00 00 00 3f 80 00 00
c0 20 00 00 40 50 00 00 c0 80 00 00 40 b0 00 00
c0 d8 00 00 f3 60 80 31
```

CRC 为 `0xf3608031`。

## 失败和指标策略

每个采样都会调用一次发送发布逻辑。发送逻辑不会改变采样主循环，也不会因为以下情况终止采样：

- 采样中的 accel/gyro 值不是有限数，或值为 NaN；
- accel/gyro/合并时间戳为负数；
- 发送系统调用返回错误；
- 发送系统调用返回的长度不是完整的 72 字节。

这些情况只增加 `dropped`。如果发送系统调用返回完整的 72 字节，则增加 `sent`。`samples` 统计发布尝试次数，`sequence` 按发布尝试递增；因此本地拒绝或发送失败可能在接收端观察为 sequence 间隙，而成功发送仍不代表接收端一定收到。

使用 `--print-metrics` 且启用 UDP 时，程序在原有结果行之后输出：

```text
SENSOR_IMU_UDP_RESULT samples=N sent=N dropped=N
```

未启用 UDP 时不会输出该行，原有默认输出和 CSV 格式不变。
