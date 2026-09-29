#!/bin/sh
# Optional one-shot NTP example. Network and the NTP server are configured separately.
set -eu

usage() {
    printf 'Usage: %s NTP_SERVER_IP_OR_HOSTNAME\n' "$0"
    printf 'Query the specified NTP server once and step the board system clock.\n'
    printf 'Does not configure network, ntpd, PTP, timezone or RTC.\n'
}

if [ "$#" -eq 1 ] && { [ "$1" = '-h' ] || [ "$1" = '--help' ]; }; then
    usage
    exit 0
fi
if [ "$#" -ne 1 ]; then
    usage >&2
    exit 1
fi
server=$1
case "$server" in
    ''|-*|*[!a-zA-Z0-9._:-]*)
        printf 'sync_time: invalid NTP server: %s\n' "$server" >&2
        exit 1;;
esac
[ "$(id -u)" -eq 0 ] || { echo 'sync_time: run as root' >&2; exit 1; }
command -v ntpdate >/dev/null 2>&1 || { echo 'sync_time: ntpdate is missing' >&2; exit 1; }
# pidof on the target intermittently misses live processes. Check each task's
# kernel comm name instead; never proceed when /proc cannot be inspected.
[ -r /proc/self/comm ] || { echo 'sync_time: /proc unavailable; refusing to set clock' >&2; exit 1; }
for task in /proc/[0-9]*/comm; do
    [ -r "$task" ] || continue
    name=$(cat "$task" 2>/dev/null) || continue  # Process may have exited.
    case "$name" in
        ntpd|phc2sys|chronyd|ntpdate)
            echo 'sync_time: stop other clock-setting services first; decide the clock hierarchy before restarting them' >&2
            exit 1;;
    esac
done
ntpdate -u -b -t 5 "$server"
