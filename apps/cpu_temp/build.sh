#!/bin/sh
set -eu
# Run from any directory; output is kept under the project, not in shared scripts.
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$root"
flags='-std=c++17 -O2 -Wall -Wextra -Werror -Iapps/cpu_temp'
case ${1:-} in
  '') compiler=${CXX:-aarch64-linux-gnu-g++}; mkdir -p dist/bin
      $compiler $flags apps/cpu_temp/main.cpp apps/cpu_temp/cpu_temp.cpp -o dist/bin/cpu_temp_demo ;;
  --host-test) compiler=${HOST_CXX:-c++}; mkdir -p .build
      $compiler $flags apps/cpu_temp/main.cpp apps/cpu_temp/cpu_temp.cpp -o .build/cpu_temp_demo
      $compiler $flags tests/test_cpu_temp.cpp apps/cpu_temp/cpu_temp.cpp -o .build/test-cpu-temp
      .build/test-cpu-temp ;;
  *) echo 'Usage: apps/cpu_temp/build.sh [--host-test]' >&2; exit 2 ;;
esac
