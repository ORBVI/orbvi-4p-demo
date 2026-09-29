#!/bin/sh
# Cross-compile the complete board-side demo against the customer's matching BSP.
set -eu
project=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$project"

HBN_ROOT=${HBN_ROOT:-"$project/build_deps/x5"}
RTSP_ROOT=${RTSP_ROOT:-"$project/build_deps/mk_api"}
HBN_SYS_LIB=${HBN_SYS_LIB:-"$project/build_deps/lib"}
: "${CXX:=aarch64-linux-gnu-g++}"

require_file() {
  if [ ! -r "$1" ]; then
    printf 'Missing build input: %s\n' "$1" >&2
    exit 2
  fi
}
require_file "$HBN_ROOT/include/hbn_api.h"
require_file "$HBN_ROOT/lib/libcam.so"
require_file "$RTSP_ROOT/include/mk_mediakit.h"
for lib in mk_api zlmediakit zltoolkit mov ext-codec mpeg flv jsoncpp; do
  require_file "$RTSP_ROOT/lib/lib$lib.a"
done
for lib in ssl crypto z; do
  require_file "$HBN_SYS_LIB/lib$lib.so"
done
if ! command -v "$CXX" >/dev/null 2>&1; then
  printf 'Missing AArch64 C++ compiler: %s\n' "$CXX" >&2
  exit 2
fi

make CXX="$CXX" HBN_ROOT="$HBN_ROOT" all
HBN_ROOT="$HBN_ROOT" RTSP_ROOT="$RTSP_ROOT" HBN_SYS_LIB="$HBN_SYS_LIB" CXX="$CXX" sh apps/build.sh
printf 'Built board binaries in %s/dist/\n' "$project"
