#!/bin/sh
set -eu
sdk=${HBN_ROOT:-/opt/x5}
compiler=${CXX:-aarch64-linux-gnu-g++}
mkdir -p dist
set -- -L"$sdk/lib" -Wl,-rpath-link,"$sdk/lib"
system_lib=$(dirname "$sdk")/lib
if [ -d "$system_lib" ]; then
    set -- "$@" -L"$system_lib" -Wl,-rpath-link,"$system_lib"
fi
"$compiler" -std=c++17 -O2 -Wall -Wextra -Werror -pthread -Inv12 -I"$sdk/include" \
    nv12/main.cpp nv12/options.cpp nv12/frame_set.cpp nv12/sink.cpp nv12/x5_camera.cpp \
    "$@" -Wl,-rpath,/usr/hobot/lib -Wl,--enable-new-dtags \
    -lcam -lvpf -lhbmem -lalog -lpthread -o dist/orbvi-nv12-demo
