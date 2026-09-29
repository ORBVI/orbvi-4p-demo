#!/usr/bin/env bash
# One-session Wi-Fi example for the X5. Deliberately no saved credentials or boot hook.
set -euo pipefail
export LC_ALL=C
umask 077

usage() {
    cat <<'HELP'
Usage: wifi_example.sh --status IFACE
       wifi_example.sh scan IFACE
       wifi_example.sh stop IFACE
       wifi_example.sh sta IFACE SSID [--open]
       wifi_example.sh ap IFACE SSID AP_IP DHCP_FIRST DHCP_LAST CHANNEL
Use a local terminal to enter WPA2 passwords (8-63 printable ASCII bytes).
The AP address and DHCP pool must be in the same /24; AP does not share Internet.
Run as root over wired Ethernet/serial. Ctrl-C/TERM stops this session.
No saved configuration or boot-time setup is performed.
HELP
}
fail() { printf 'wifi_example: %s\n' "$1" >&2; exit 1; }
valid_iface() { [[ $1 =~ ^[a-zA-Z0-9_.-]{1,15}$ ]] && [[ $1 != -* ]]; }
valid_ssid() { [[ $1 =~ ^[a-zA-Z0-9_.\ -]{1,32}$ ]]; }
valid_ip() {
    local ip=$1 part
    local -a octets
    [[ $ip =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]] || return 1
    IFS=. read -r -a octets <<< "$ip"
    for part in "${octets[@]}"; do
        ((${#part} <= 3 && 10#$part <= 255)) || return 1
        # Reject noncanonical addresses before using them in a daemon config.
        [[ $part == 0 || $part != 0* ]] || return 1
    done
}
require_cmd() { command -v "$1" >/dev/null 2>&1 || fail "missing command: $1"; }
secret=''
tty_state=''
workdir=''
iface=''
configured_ip=''
raised=0
state_file=''
state_root=${WIFI_STATE_ROOT:-/run}
pids=()
cleanup() {
    trap - EXIT INT TERM
    local pid
    if [[ -n $tty_state ]]; then stty "$tty_state" </dev/tty 2>/dev/null || :; fi
    for pid in "${pids[@]}"; do kill "$pid" 2>/dev/null || :; done
    for pid in "${pids[@]}"; do wait "$pid" 2>/dev/null || :; done
    if [[ -n $configured_ip ]]; then ip addr del "$configured_ip/24" dev "$iface" >/dev/null 2>&1 || :; fi
    if ((raised)) && [[ $mode == sta ]]; then ip -4 addr flush dev "$iface" >/dev/null 2>&1 || :; fi
    if ((raised)); then ip link set dev "$iface" down >/dev/null 2>&1 || :; fi
    if [[ -n $state_file && -f $state_file ]]; then
        read -r registered_pid registered_start registered_dir < "$state_file" || :
        if [[ ${registered_pid:-} == "$$" && ${registered_start:-} == "$(awk '{print $22}' /proc/$$/stat)" && ${registered_dir:-} == "$workdir" ]]; then rm -f -- "$state_file"; fi
    fi
    if [[ -n $workdir ]]; then rm -rf -- "$workdir"; fi
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

if (($# == 1)) && [[ $1 == --help || $1 == -h ]]; then usage; exit 0; fi
(($# >= 2)) || { usage >&2; exit 2; }
mode=$1 iface=$2
up_re='<([^>]*,)?UP(,|>)'
valid_iface "$iface" || fail 'invalid interface name'
[[ $(id -u) == 0 ]] || fail 'run as root'
require_cmd ip
require_cmd iw
ip link show dev "$iface" >/dev/null 2>&1 || fail 'interface does not exist'
iw dev "$iface" info >/dev/null 2>&1 || fail 'not a Wi-Fi interface'
if [[ $mode == --status && $# == 2 ]]; then
    ip addr show dev "$iface"
    iw dev "$iface" link
    exit 0
fi
if [[ $mode == stop && $# == 2 ]]; then
    require_cmd awk
    state_file=$state_root/orbvi-wifi-$iface.state
    [[ -f $state_file && ! -L $state_file ]] || fail "no managed session for $iface"
    read -r owner_pid owner_start owner_dir < "$state_file" || fail "invalid session state"
    [[ $owner_pid =~ ^[0-9]+$ && $owner_start =~ ^[0-9]+$ && $owner_dir == */orbvi-wifi.* ]] || fail "invalid session state"
    [[ -r /proc/$owner_pid/stat ]] || fail "stale session state; remove it manually after inspection"
    current_start=$(awk "{print \$22}" "/proc/$owner_pid/stat")
    [[ $current_start == "$owner_start" ]] || fail "stale session state; refusing to signal another process"
    # Do not remove state or kill daemons directly: the owner session cleans up.
    kill -TERM "$owner_pid" || fail "could not stop managed session"
    printf "Stop requested for %s; the session will release its resources.\n" "$iface"
    exit 0
fi
# Fail closed if any probe fails; do not take over a system-managed interface.
ensure_free() {
    local socket link_info addr_info wifi_link
    [[ ! -e $state_root/orbvi-wifi-$iface.state ]] || fail 'managed session already exists'
    for socket in /run/wpa_supplicant/"$iface" /var/run/wpa_supplicant/"$iface" /run/hostapd/"$iface" /var/run/hostapd/"$iface"; do
        [[ ! -S $socket ]] || fail "existing Wi-Fi manager socket for $iface"
    done
    link_info=$(ip -o link show dev "$iface") || fail 'cannot inspect Wi-Fi link'
    addr_info=$(ip -o addr show dev "$iface") || fail 'cannot inspect Wi-Fi addresses'
    wifi_link=$(iw dev "$iface" link) || fail 'cannot inspect Wi-Fi association'
    [[ ! $link_info =~ $up_re ]] || fail 'interface is UP; refusing to take over existing Wi-Fi'
    [[ -z $addr_info ]] || fail 'interface has addresses; refusing to change existing Wi-Fi'
    [[ $wifi_link == *'Not connected.'* ]] || fail 'Wi-Fi is connected; refusing to take over'
}
if [[ $mode == scan && $# == 2 ]]; then
    require_cmd awk
    ensure_free
    raised=1
    ip link set dev "$iface" up
    iw dev "$iface" scan | awk '/^[[:space:]]*SSID: / {sub(/^[[:space:]]*SSID: /, ""); if (length($0) && !seen[$0]++) print}'
    exit 0
fi
case $mode in
    sta)
        (($# == 3 || $# == 4)) || fail 'usage: sta IFACE SSID [--open]'
        valid_ssid "$3" || fail 'SSID must contain 1-32 ASCII letters, digits, spaces, . _ or -'
        ssid=$3
        [[ $# == 3 || $4 == --open ]] || fail 'only --open is supported as fourth argument'
        require_cmd wpa_supplicant
        require_cmd wpa_cli
        require_cmd udhcpc
        ;;
    ap)
        (($# == 7)) || fail 'usage: ap IFACE SSID AP_IP DHCP_FIRST DHCP_LAST CHANNEL'
        valid_ssid "$3" || fail 'SSID must contain 1-32 ASCII letters, digits, spaces, . _ or -'
        ssid=$3
        if ! valid_ip "$4" || ! valid_ip "$5" || ! valid_ip "$6"; then fail 'invalid /24 IPv4 address'; fi
        [[ ${4%.*} == "${5%.*}" && ${4%.*} == "${6%.*}" ]] || fail 'DHCP pool must share AP /24'
        # AP is local-only; reject multicast, loopback and globally routed ranges.
        IFS=. read -r a b _ d <<< "$4"
        if ! ((a == 10 || a == 192 && b == 168 || a == 172 && b >= 16 && b <= 31)); then
            fail 'AP address must be RFC1918 private IPv4'
        fi
        ((10#$d >= 1 && 10#$d <= 254)) || fail 'AP address must be a /24 host address'
        ((10#${5##*.} >= 1 && 10#${5##*.} < 10#${6##*.} && 10#${6##*.} <= 254)) || fail 'invalid DHCP pool range'
        ((10#${4##*.} < 10#${5##*.} || 10#${4##*.} > 10#${6##*.})) || fail 'AP address overlaps DHCP pool'
        [[ $7 =~ ^([1-9]|1[0-3])$ ]] || fail 'channel must be 1-13 (check regulatory domain)'
        ap_ip=$4 first=$5 last=$6 channel=$7
        require_cmd hostapd
        require_cmd dnsmasq
        ;;
    *) usage >&2; exit 2 ;;
esac
# Network administrators must explicitly prepare a free interface before running.
ensure_free

if [[ $mode == ap || $# == 3 ]]; then
    [[ -r /dev/tty && -w /dev/tty ]] || fail 'password requires a local terminal'
    require_cmd stty
    tty_state=$(stty -g </dev/tty) || fail 'cannot read terminal state'
    stty -echo </dev/tty || fail 'cannot disable terminal echo'
    printf 'Wi-Fi password: ' >/dev/tty
    IFS= read -r secret </dev/tty || fail 'cannot read password'
    stty "$tty_state" </dev/tty
    tty_state=''
    printf '\n' >/dev/tty
    if ((${#secret} < 8 || ${#secret} > 63)) || [[ $secret == *[!\ -\~]* ]]; then
        fail 'password must be 8-63 printable ASCII bytes'
    fi
fi
# mktemp creates a non-predictable, root-private directory; never write to the repo.
workdir=$(mktemp -d "${TMPDIR:-/tmp}/orbvi-wifi.XXXXXXXX") || fail 'cannot create private runtime directory'
if [[ $mode == sta ]]; then
    # Only our own file is writable; disable wpa_supplicant config updates.
    {
        printf 'ctrl_interface=%s/ctrl\nupdate_config=0\nnetwork={\n    ssid="%s"\n' "$workdir" "$ssid"
        if [[ $# == 4 ]]; then printf '    key_mgmt=NONE\n';
        else
            escaped=${secret//\\/\\\\}; escaped=${escaped//\"/\\\"}
            printf '    psk="%s"\n    key_mgmt=WPA-PSK\n' "$escaped"
        fi
        printf '}\n'
    } > "$workdir/wpa.conf"
else
    {
        printf 'interface=%s\ndriver=nl80211\nssid=%s\nhw_mode=g\nchannel=%s\n' "$iface" "$ssid" "$channel"
        printf 'wpa=2\nwpa_passphrase=%s\nwpa_key_mgmt=WPA-PSK\nrsn_pairwise=CCMP\n' "$secret"
    } > "$workdir/hostapd.conf"
fi
secret='' escaped=''
require_cmd awk
[[ -d $state_root && -w $state_root ]] || fail 'runtime state directory is missing or not writable'
state_file=$state_root/orbvi-wifi-$iface.state
# Refuse pre-existing state, including symlinks; a second invocation never takes over.
( set -C; printf '%s %s %s\n' "$$" "$(awk '{print $22}' /proc/$$/stat)" "$workdir" > "$state_file" ) || fail 'managed session already exists; inspect it before retrying'
raised=1
ip link set dev "$iface" up
if [[ $mode == sta ]]; then
    mkdir -m 0700 "$workdir/ctrl"
    wpa_supplicant -D nl80211 -i "$iface" -c "$workdir/wpa.conf" > /dev/null 2>&1 &
    pids+=("$!")
    connected=0
    for ((i=0; i<25; i++)); do
        kill -0 "${pids[0]}" 2>/dev/null || fail 'wpa_supplicant exited (check driver/interface)'
        if [[ $(wpa_cli -p "$workdir/ctrl" -i "$iface" status 2>/dev/null || :) == *'wpa_state=COMPLETED'* ]]; then connected=1; break; fi
        sleep 1
    done
    ((connected)) || fail 'association timed out (check SSID, password and signal)'
    # The board's default udhcpc hook writes global DNS and installs a competing
    # default route. Restrict DHCP to the managed interface's IPv4 address.
    cat > "$workdir/dhcp.sh" <<EOF_DHCP
#!/bin/sh
set -eu
[ "\${interface:-}" = "$iface" ] || exit 1
case "\${1:-}" in
    deconfig) ip -4 addr flush dev "\$interface" ;;
    bound|renew)
        [ -n "\${ip:-}" ] && [ -n "\${subnet:-}" ] || exit 1
        ip -4 addr replace "\$ip/\$subnet" dev "\$interface" ;;
esac
EOF_DHCP
    chmod 700 "$workdir/dhcp.sh"
    # -f keeps the DHCP client attached for renewals; -n exits if the initial lease fails.
    udhcpc -i "$iface" -f -n -t 5 -T 3 -s "$workdir/dhcp.sh" > /dev/null 2>&1 &
    pids+=("$!")
    lease=0
    for ((i=0; i<18; i++)); do
        kill -0 "${pids[1]}" 2>/dev/null || fail 'DHCP client exited without a lease'
        if [[ -n $(ip -o -4 addr show dev "$iface") ]]; then lease=1; break; fi
        sleep 1
    done
    ((lease)) || fail 'DHCP did not assign an IPv4 address'
    printf 'STA session running on %s; check address with: ip -4 addr show dev %s\n' "$iface" "$iface"
else
    ip addr add "$ap_ip/24" dev "$iface"
    configured_ip=$ap_ip
    hostapd "$workdir/hostapd.conf" > /dev/null 2>&1 &
    pids+=("$!")
    # No DNS or default-gateway advertisement: this example is a local-only AP.
    dnsmasq --no-daemon --port=0 --no-resolv --no-hosts --bind-interfaces \
        --interface="$iface" --except-interface=lo \
        --dhcp-range="$first,$last,255.255.255.0,12h" \
        --dhcp-leasefile="$workdir/dnsmasq.leases" \
        --dhcp-option=option:router --dhcp-option=option:dns-server \
        > /dev/null 2>&1 &
    pids+=("$!")
    sleep 2
    for pid in "${pids[@]}"; do kill -0 "$pid" 2>/dev/null || fail 'AP or DHCP daemon exited'; done
    printf 'Local-only AP session running on %s (%s/24).\n' "$iface" "$ap_ip"
fi
printf 'Keep this terminal open; Ctrl-C stops only this session. No boot or saved credentials.\n'
while :; do
    for pid in "${pids[@]}"; do kill -0 "$pid" 2>/dev/null || fail 'Wi-Fi session process exited'; done
    sleep 1
done
