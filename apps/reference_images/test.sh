#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$root"
compiler=${HOST_CXX:-c++}
headers=${HBN_HEADERS:-${HBN_ROOT:-/opt/x5}/include}
if [ ! -f "$headers/hb_media_codec.h" ]; then
    echo "Missing $headers/hb_media_codec.h: set HBN_HEADERS to X5 public headers" >&2
    exit 2
fi
mkdir -p .build
"$compiler" -std=c++17 -O2 -Wall -Wextra -Werror -pthread -Iapps/reference_images -Inv12 -I"$headers" \
    tests/test_reference_image_board.cpp apps/reference_images/nv12_planes.cpp \
    apps/reference_images/protocol.cpp -o .build/test-reference-image-board
.build/test-reference-image-board
"$compiler" -std=c++17 -O2 -Wall -Wextra -Werror -Iapps/reference_images -Inv12 -I"$headers" \
    tests/test_reference_jpeg_codec.cpp apps/reference_images/jpeg_codec.cpp \
    apps/reference_images/nv12_planes.cpp apps/reference_images/protocol.cpp \
    -o .build/test-reference-jpeg-codec
.build/test-reference-jpeg-codec
echo 'PASS: reference images board protocol/stride and JPEG codec mock'
