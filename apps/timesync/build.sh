#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$root"
compiler=${CXX:-aarch64-linux-gnu-g++}
mkdir -p dist/bin
"$compiler" -std=c++17 -O2 -Wall -Wextra -Werror -pthread \
    apps/timesync/main.cpp apps/timesync/time_sync.cpp -o dist/bin/timesync_demo
install -m 0755 apps/launchers/timesync_demo dist/timesync_demo
