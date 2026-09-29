#!/bin/sh
# Hermetic NTP mock; spawns short-lived clock-daemon-name probes but never changes the host clock.
set -eu
root=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'if [ -n "${service_pid:-}" ]; then kill "$service_pid" 2>/dev/null || true; wait "$service_pid" 2>/dev/null || true; fi; rm -rf "$tmp"' EXIT
mkdir -p "$tmp/bin"
cat > "$tmp/bin/id" <<'MOCK'
#!/bin/sh
printf '%s\n' "${TEST_UID:-0}"
MOCK
cat > "$tmp/bin/ntpdate" <<'MOCK'
#!/bin/sh
printf '%s\n' "$*" >> "$TEST_TMP/ntpdate_calls"
[ "${TEST_NTP_FAIL:-0}" -eq 0 ]
MOCK
chmod +x "$tmp"/bin/*
export TEST_TMP="$tmp" PATH="$tmp/bin:$PATH"
script="$root/scripts/sync_time.sh"

"$script" --help >/dev/null
[ ! -e "$tmp/ntpdate_calls" ]
if TEST_UID=1000 "$script" 192.0.2.1 >/dev/null 2>&1; then exit 1; fi
[ ! -e "$tmp/ntpdate_calls" ]
"$script" 192.0.2.1 >/dev/null
[ "$(wc -l < "$tmp/ntpdate_calls")" -eq 1 ]
[ "$(cat "$tmp/ntpdate_calls")" = '-u -b -t 5 192.0.2.1' ]
rm "$tmp/ntpdate_calls"
for args in '' '-invalid' 'bad name' 'bad/path' '1.2.3.4;true'; do
    # Deliberately split test arguments to check invalid arity and whitespace.
    # shellcheck disable=SC2086
    if "$script" $args >/dev/null 2>&1; then exit 1; fi
    [ ! -e "$tmp/ntpdate_calls" ]
done
# A real task named ntpd/phc2sys must be detected even if pidof is unreliable.
# Copying sleep gives the kernel a matching /proc/PID/comm without clock changes.
for service in ntpd phc2sys chronyd ntpdate; do
    cp "$(command -v sleep)" "$tmp/$service"
    "$tmp/$service" 30 &
    service_pid=$!
    sleep 0.1
    [ "$(cat "/proc/$service_pid/comm")" = "$service" ]
    if "$script" 192.0.2.1 >/dev/null 2>&1; then kill "$service_pid"; exit 1; fi
    [ ! -e "$tmp/ntpdate_calls" ]
    kill "$service_pid"
    wait "$service_pid" 2>/dev/null || true
    service_pid=
done
if TEST_NTP_FAIL=1 "$script" 192.0.2.1 >/dev/null 2>&1; then exit 1; fi
[ "$(cat "$tmp/ntpdate_calls")" = '-u -b -t 5 192.0.2.1' ]
printf 'PASS: sync_time one-shot NTP examples\n'
