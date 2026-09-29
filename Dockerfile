FROM ubuntu:22.04
RUN apt-get update && apt-get install -y --no-install-recommends \
    make gcc g++ libc6-dev gcc-aarch64-linux-gnu g++-aarch64-linux-gnu python3 \
    && rm -rf /var/lib/apt/lists/*

# Versioned SDK development inputs are bundled with the Demo repository.
# These are link-time inputs only; do not copy them over the board's BSP.
COPY build_deps/x5/include/ /opt/x5/include/
COPY build_deps/x5/lib/ /opt/x5/lib/
COPY build_deps/x5/HEADERS-LICENSE build_deps/x5/LIBRARIES-LICENSE /opt/x5/
