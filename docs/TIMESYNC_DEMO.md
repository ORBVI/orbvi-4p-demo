# UART/PPS 授时 Demo

`timesync_demo` 0.1.1 默认自动接收 ZDA 和 RMC，无需选择协议。两种报文共用 PPS 配对和调钟逻辑。

## 接线与时间源

N32 PA2/USART2 TX 接 4P UART1 RX，PB1/TIM3_CH4 接 PPS 输入，并共地。UART 为 115200 8N1，PPS 为 1 Hz；报文中的整秒对应发送前的 PPS 上升沿。

N32 固件默认 `make TIME_PROTOCOL=ZDA`，也可用 `make TIME_PROTOCOL=RMC`；接收程序和工装无需跟随切换。N32 每次复位从 **2025-01-01 00:00:00 UTC** 模拟起算、逐秒递增，不需要电脑提供时间。

## 运行

确认 UART/PPS 空闲后，在板端运行：

```sh
cd /userdata/orbvi-4p-demo
# 只读：自动接收 ZDA/RMC，查看 PPS 配对。
./timesync_demo --uart /dev/ttyS1 --pps /dev/pps1
# 需要验证实际授时时，停止竞争的调钟服务后运行：
./timesync_demo --uart /dev/ttyS1 --pps /dev/pps1 --apply
```

连续 3 组有效报文与 PPS 配对进入 `LOCKED`，不连续或超时会重新获取。接收端继续校验报文校验和、合法日期和整秒格式。`--protocol ZDA|RMC` 仅供手动排查时限定输入，正常运行不需要；`--protocol AUTO` 等同默认行为。

## 检测结果

只读模式不修改板钟。`--apply` 后检查连续 `LOCKED`、后续 `offset_ms` 和退出后的 `date -u` 是否跟随报文时间；`APPLIED` 表示本轮实际改时，板钟已在 5 ms 内时无需重复改时。

工装直接运行默认接收模式，检查配对连续性和板钟读回，不按协议或启动说明文字判定。受管 ntpd 的暂停、恢复仍按原流程执行。

`--apply` 会将板钟设为模拟日期，需与其他调钟程序错开，并在测试后恢复正确时间。此项验证 UART/PPS 授时功能，不代表真实 UTC 精度或相机/IMU 硬同步通过。

## 验证

主机执行 `make test`，交叉编译执行 Docker 内的 `sh scripts/build-customer.sh`。本次代码经过主机和构建验证，尚未做新版实线 UART/PPS 验收；旧版本的历史板测不能替代本次验收。
