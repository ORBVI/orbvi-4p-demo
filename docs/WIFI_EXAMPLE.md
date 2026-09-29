# X5 Wi-Fi 前台会话示例

本示例 `scripts/wifi_example.sh` 是独立实现，参照 [产品 Wi-Fi 配置文档](https://4p-docs.readthedocs.io/zh-cn/latest/getting-started/wifi-configuration.html) 的设备命令（`ip`、`iw`、`hostapd`、`dnsmasq`、`wpa_supplicant`、`wpa_cli`、`udhcpc`）和 [参考方案](https://github.com/Hessian-matrix/RoboBaton_4p_demo) 的 AP/STA 功能范围，不复制上游 Apache-2.0 代码。本仓库 MIT 代码不包含参考仓库脚本。

**与参考方案不同**：不默认 `wlan0`，必须明确传入已确认的 Wi-Fi 网卡名；不调用全局 `killall`，不 `source` 状态文件，不保存凭据或开机恢复，不改 `/userdata/startup.sh` 或其他系统启动文件。仅在接口 *DOWN、未配置地址、未连接* 时启动；运行时占用当前终端，`stop` 只向本示例登记的会话发送 TERM。该会话退出时停止自己启动的进程、删除该接口由本次设置的 AP 地址或 STA 的 IPv4 地址，并把该接口置 DOWN。不碰 eth0 或其他网卡。**如果正通过 Wi-Fi SSH 控制板卡，退出将断线**；请保持有线 SSH/USB/1.8V DEBUG_UART 控制通道。暂不提供自动启动。

## 有线接入及只读预检

在开发机编译后 `dist/wifi_example.sh` 随 `make all` / `./scripts/build-customer.sh` 打包（`dist/manifest.sha256` 中校验）。也可以单独拷贝源码，无需交叉编译；脚本依赖板端 Bash。执行前请先经有线 SSH 或 USB ADB root 登录，确认你当前使用的网络不是目标 Wi-Fi 接口：

```sh
ip -o link show
ip -o addr show
# 从以上输出选出实际 Wi-Fi 接口名，例子只供示意
WIFI_IFACE=wlan0
ip link show dev "$WIFI_IFACE"
iw dev "$WIFI_IFACE" info
# 板上脚本（放在 /userdata/orbvi-4p-demo/，仅代码；不拷贝凭据）
cd /userdata/orbvi-4p-demo
./wifi_example.sh --status "$WIFI_IFACE"
```

操作脚本需要 root、`ip`、`iw`、`awk`、`mktemp`、`stty`、`wpa_supplicant`/`wpa_cli`/`udhcpc` (STA) 或 `hostapd`/`dnsmasq` (AP)。受控会话登记在 `/run/orbvi-wifi-<接口>.state`，配置在私有临时目录（默认 `/tmp/orbvi-wifi.*`），均不应上传或提交。**临时配置包含明文密码**，权限由 `umask 077` 限制；脚本不把密码作为命令行参数、环境变量、扫描日志或 stdout 打印。请不要使用 `set -x`、终端录屏、审计配置内容或把私有目录同步到开发机。终端上只在出现密码提示后再输入密码。

## 可控步骤

先确认接口为 DOWN、无地址、未连接，且没有其他服务管理该接口；脚本若发现接口在用会拒绝，不会停止原有服务。`scan` 只临时将空闲接口置 UP 扫描 SSID，随后复位为 DOWN，SSID 列表可能包含现场敏感信息。

```sh
./wifi_example.sh scan "$WIFI_IFACE"
# STA（SSID 必须自行确认，不能传密码参数；提示后从终端隐藏输入 WPA2 密码）
./wifi_example.sh sta "$WIFI_IFACE" 'ExampleSSID'
# 对确实不设密码的网络，必须显式使用 --open
./wifi_example.sh sta "$WIFI_IFACE" 'OpenSSID' --open
```

STA 用 `wpa_supplicant -D nl80211` 关联（约 25 秒超时），`udhcpc -f -n -t 5 -T 3` 负责 DHCP 续租，其会话专用钩子仅设置此 Wi-Fi 接口的 IPv4 地址，不修改系统默认路由或 DNS；启动后用**另一个有线/USB 终端**检查 `./wifi_example.sh --status "$WIFI_IFACE"` 和 `ip -4 addr show dev "$WIFI_IFACE"`。脚本会等待 DHCP 地址分配成功才报告就绪，仍需检查路由和实际通信。为了不抢占有线管理通道，STA 默认仅能访问该 Wi-Fi 网段；如要通过 Wi-Fi 访问其他网段或使用其 DNS，请在确认路由优先级后自行配置，脚本不会自动更改系统默认路由或 DNS。

AP 必须明确选择与现有有线网段不冲突的 RFC1918 私网 `/24` 网段、DHCP 范围与 2.4 GHz 信道，并确认所在地区及板载设备允许该信道。示例**不提供**上网共享/NAT/默认网关/DNS，不支持开放热点：

```sh
./wifi_example.sh ap "$WIFI_IFACE" 'ExampleAP' 192.168.5.1 192.168.5.2 192.168.5.20 1
# 另一有线/USB 终端中：
./wifi_example.sh --status "$WIFI_IFACE"
./wifi_example.sh stop "$WIFI_IFACE"
```

也可在运行会话的终端按 Ctrl-C。`stop` 仅发送停止请求，待前台会话清理后，检查 `ip link show dev "$WIFI_IFACE"` 和 `/run/orbvi-wifi-<接口>.state` 是否消失；不应重复运行本脚本占用同一接口。异常断电/SIGKILL 时无法清理，确认没有关联进程后手工检查并删除失效状态文件、私有临时目录和遗留接口配置。切换模式前先停止前一会话。不要对别人的会话强行删除状态文件。SSH 终端断线时不保证自动清理；通过有线/USB 执行 `stop`。

限制：仅支持 SSID 中 ASCII 字母/数字/空格/点/下划线/短横线（1–32 字节），仅 WPA2-PSK 8–63 位可打印 ASCII 密码或显式开放 STA；不支持隐藏 SSID、WPA3/Enterprise、IPv6、静态 IP、频段切换、与系统 NetworkManager/其他 Wi-Fi 服务并存或自动恢复。本次实板测试已确认 AP 客户端获取 DHCP 地址并 ping 通板卡，以及 STA 关联临时热点、获取 DHCP 地址并 ping 通；部署到其他网络环境仍须复测。

宿主机测试：`make test-wifi`（仅模拟 `ip`/`iw`/无线守护进程，不操作宿主机网卡）。完整 `make test` 另含 NTP 测试；若宿主机运行 chronyd，现有 `tests/test_sync_time.sh` 可能拒绝继续；无需停掉主机 chronyd，分开报告其他测试。
