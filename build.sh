#!/usr/bin/env bash
set -euo pipefail
project=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
if [[ $# -ne 0 ]]; then
    echo "Usage: $0 (no SDK directory required)" >&2
    exit 2
fi
image=${BUILD_IMAGE:-orbvi-4p-demo-builder:standalone}
docker build --platform linux/amd64 -t "$image" "$project"
docker run --rm --platform linux/amd64 --network none \
    --user "$(id -u):$(id -g)" \
    -v "$project:/work:rw" -w /work "$image" make -B all test
echo "Built: $project/dist/orbvi-4p-demo"
