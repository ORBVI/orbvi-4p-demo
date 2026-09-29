#!/usr/bin/env bash
set -euo pipefail
project=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
scratch=$(mktemp -d)
trap 'rm -rf "$scratch"' EXIT
tar -C "$project" --exclude=.git --exclude=.build --exclude=dist -cf - . | tar -C "$scratch" -xf -
cd "$scratch"
env -u SDK_ROOT bash ./build.sh
test -s dist/orbvi-4p-demo
test -s dist/orbvi-verify
test -x dist/run.sh
echo 'PASS: standalone build from a clean copy outside the BSP'
