#!/bin/sh
set -eu
sdk=${HBN_ROOT:-/opt/x5-sdk/usr/hobot}
rtsp=${RTSP_ROOT:?set RTSP_ROOT to the BSP media_server/mk_api directory}
# Staged AArch64 system libraries (OpenSSL, zlib); override for other BSP layouts.
syslib=${HBN_SYS_LIB:-build_deps/lib}
compiler=${CXX:-aarch64-linux-gnu-g++}
mkdir -p dist/bin
# The BSP ZLMediaKit static archives are not PIC; this toolchain enables PIE by default.
"$compiler" -std=c++17 -O2 -Wall -Wextra -Werror -pthread \
  -Iapps/camera -Inv12 -I"$sdk/include" -I"$rtsp/include" \
  apps/camera/main.cpp apps/camera/camera_options.cpp apps/camera/nv12_copy.cpp \
  apps/camera/encoder_bsp.cpp apps/camera/rtsp_server.cpp apps/camera/process_group.cpp nv12/x5_camera.cpp \
  -L"$sdk/lib" -L"$(dirname "$sdk")/lib" -L"$rtsp/lib" -L"$syslib" \
  -Wl,-rpath-link,"$sdk/lib" -Wl,-rpath-link,"$(dirname "$sdk")/lib" -Wl,-rpath-link,"$syslib" \
  -Wl,--start-group -lmk_api -lzlmediakit -lzltoolkit -lmov -lext-codec \
  -lmpeg -lflv -ljsoncpp -Wl,--end-group \
  -lcam -lvpf -lhbmem -lmultimedia -lalog -lssl -lcrypto -lz -ldl -lrt -lpthread \
  -static-libstdc++ -static-libgcc \
  -Wl,-rpath,/usr/hobot/lib -Wl,--enable-new-dtags -no-pie -o dist/bin/cam_demo
"$compiler" -std=c++17 -O2 -Wall -Wextra -Werror -pthread -Iapps/camera \
  apps/camera/orchestrator.cpp apps/camera/camera_options.cpp apps/camera/process_group.cpp \
  -o dist/bin/cam_group
install -m 0755 apps/launchers/cam_demo dist/cam_demo
"$compiler" -std=c++17 -O2 -Wall -Wextra -Werror -pthread \
  -Iapps/camera -Inv12 -I"$sdk/include" -I"$rtsp/include" \
  apps/camera/mosaic_main.cpp apps/camera/mosaic_options.cpp apps/camera/mosaic_nv12.cpp \
  apps/camera/nv12_copy.cpp apps/camera/encoder_bsp.cpp apps/camera/rtsp_server.cpp \
  nv12/frame_set.cpp nv12/x5_camera.cpp \
  -L"$sdk/lib" -L"$(dirname "$sdk")/lib" -L"$rtsp/lib" -L"$syslib" \
  -Wl,-rpath-link,"$sdk/lib" -Wl,-rpath-link,"$(dirname "$sdk")/lib" -Wl,-rpath-link,"$syslib" \
  -Wl,--start-group -lmk_api -lzlmediakit -lzltoolkit -lmov -lext-codec \
  -lmpeg -lflv -ljsoncpp -Wl,--end-group \
  -lcam -lvpf -lhbmem -lmultimedia -lalog -lssl -lcrypto -lz -ldl -lrt -lpthread \
  -static-libstdc++ -static-libgcc \
  -Wl,-rpath,/usr/hobot/lib -Wl,--enable-new-dtags -no-pie -o dist/bin/mosaic_rtsp_demo
install -m 0755 apps/launchers/mosaic_rtsp_demo dist/mosaic_rtsp_demo
