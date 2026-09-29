#!/usr/bin/env bash
# One-command, complete board-demo cross build. Uses only bundled source/link inputs.
set -euo pipefail
project=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
if [[ $# -ne 0 ]]; then
  echo 'Usage: ./build-all.sh' >&2
  exit 2
fi
image=${BUILD_IMAGE:-orbvi-4p-demo-builder:standalone}
docker build --platform linux/amd64 -t "$image" "$project"
docker run --rm --platform linux/amd64 --network none \
  --user "$(id -u):$(id -g)" -v "$project:/work:rw" -w /work "$image" \
  ./scripts/build-customer.sh
