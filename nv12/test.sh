#!/bin/sh
set -eu
mkdir -p .build
cxx=${HOST_CXX:-c++}
"$cxx" -std=c++17 -Wall -Wextra -Werror -pthread -Inv12 \
    tests/test_nv12_group.cpp nv12/frame_set.cpp -o .build/test-nv12-group
.build/test-nv12-group
"$cxx" -std=c++17 -Wall -Wextra -Werror -Inv12 \
    tests/test_nv12_sink.cpp nv12/sink.cpp -o .build/test-nv12-sink
.build/test-nv12-sink
"$cxx" -std=c++17 -Wall -Wextra -Werror -Inv12 \
    tests/test_nv12_options.cpp nv12/options.cpp -o .build/test-nv12-options
.build/test-nv12-options
