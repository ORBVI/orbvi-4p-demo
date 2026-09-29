# Modular C++ Regression (2026-09-08)

Historical record: these board results predate the all-C++ validation-tool migration.
The former board, shutdown and sync harnesses are now `orbvi-verify` subcommands;
the old implementation has been removed. Re-run board acceptance for the new tools.

Tested with the existing ORBVI 4P kernel #15; no BSP image or runtime library
was replaced. ADB did not enumerate; deployment and tests used the debug UART.
The transferred executable was SHA-256 verified before being written:
`bb7fb7792a8e25c314a7bba4baa9db1c641f83a23ea601dc7070f52613e2d041`.

## Host and standalone build

- `make test test-sync`: passed. C++ option, capture lifecycle and output tests;
  five shutdown helper tests in the former harness.
- `bash tests/build-standalone.sh`: passed from a clean temporary copy outside
  the BSP; Docker mounts only the demo, compilation runs without network.
- Build uses C++17 with `-Wall -Wextra -Werror`.
- Lifecycle tests cover all-slaves-before-streaming, prepare failure, output
  exception/frame release, no-frame timeout, reverse cleanup and signal/error
  precedence. Output tests preserve CSV values, exact RAW bytes and no-overwrite
  behavior, reject empty frames, and validate discard mode and idempotent close.

## Board

- Former board harness: passed four-camera RAW/CSV, single port, invalid options,
  existing directory rejection, SIGINT and restart.
  Artifacts: `/userdata/orbvi-demo-test-hi1zx9_2`.
- Former shutdown harness: passed normal SIGINT exit 130 and injected SDK stop
  failure exit 1. Artifacts: `/userdata/orbvi-shutdown-iebb76td`.
- Former sync harness with `ORBVI_SYNC_SECONDS=60`: passed without RAW recording.
  Artifacts: `/userdata/orbvi-sync-n2i6lrr_`.
- 15 Hz: 45 pulses, 45 frames on each camera, 45 markers on each IMU stream.
- 30 Hz for 60 seconds: 1800 pulses, 1800 frames on each camera, 1800 markers
  on each IMU stream. Maximum inter-camera SDK timestamp spread: 78125 ns.
- IMU configured at 1000 Hz, +/-16 g and +/-2000 dps. Both streams delivered
  59231 samples during the 30 Hz phase, measured 987.137 Hz; maximum sample
  interval 1.239342 ms, within the existing validation limits.
- Quiet phases produced no camera frames or FSYNC markers; IMU sampling continued.
- Measured-phase error counters, including shutdown, did not increase.
  Historical dropped samples and timestamp resets were already nonzero.
- IIO buffers disabled and GPIO417 released after validation.

Startup still emits the previously documented SIF frame-size messages. This is
a bounded functional regression, not proof of a warning-free camera stack,
physical exposure-edge alignment, or long-duration stability. SDK frame times
trail IMU FSYNC markers by about 9.82 ms due to differing timestamp semantics.
