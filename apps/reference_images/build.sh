#!/bin/sh
set -eu
# HBN_ROOT points to the board SDK's /usr/hobot (headers and libraries).
sdk=${HBN_ROOT:-/opt/x5-sdk/usr/hobot}
headers=${HBN_HEADERS:-$sdk/include}
sys_lib=${HBN_SYS_LIB:-$(dirname "$sdk")/lib}
cxx=${CXX:-aarch64-linux-gnu-g++}
mkdir -p dist/bin
"$cxx" -std=c++17 -O2 -Wall -Wextra -Werror -pthread \
  -Iapps/reference_images -Inv12 -I"$headers" \
  apps/reference_images/main.cpp apps/reference_images/protocol.cpp \
  apps/reference_images/nv12_planes.cpp apps/reference_images/jpeg_codec.cpp \
  nv12/x5_camera.cpp -L"$sdk/lib" -L"$sys_lib" \
  -Wl,-rpath-link,"$sdk/lib" -Wl,-rpath-link,"$sys_lib" \
  -lcam -lvpf -lhbmem -lmultimedia -lalog -ldl -lrt -lpthread \
  -static-libstdc++ -static-libgcc -Wl,-rpath,/usr/hobot/lib \
  -Wl,--enable-new-dtags -no-pie -o dist/bin/reference_images_demo
install -m 0755 apps/reference_images/nv12_demo dist/nv12_demo
install -m 0755 apps/reference_images/jpeg_demo dist/jpeg_demo
