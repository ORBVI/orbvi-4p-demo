#!/bin/sh
set -eu
directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
export LD_LIBRARY_PATH="/usr/hobot/lib:/usr/hobot/lib/sensor${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec "$directory/orbvi-4p-demo" "$@"
