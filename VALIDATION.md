# Validation history and current migration

See [2026-09-08 modular validation](docs/MODULAR_VALIDATION.md) for the previous
board-tested revision. All validation tools and host tests have since moved to
C++17; historical board results do not certify the replacement tools. The commands
below use the current C++ entry points. Re-run board acceptance after deployment.

# Review Fix Acceptance (kernel #15)

2026-09-08: current C++ binary SHA-256:
`132827e2a240b4e3c1030b710d0080cafe01ecc7d9089d610880cc0f205cb976`.

- Independent Docker C++ build and host options tests pass.
- Five host shutdown/error-cleanup tests pass.
- Real SDK stop failure injection: normal SIGINT returns 130, cleanup failure
  returns 1; both verified on the board without RAW recording.
- 120 seconds at 30 Hz: each camera 3600 frames, each IMU 3600 matching
  FSYNC markers; 15 Hz and quiet phases also pass.
- Final error counters are checked after camera exit and IIO shutdown;
  no drop/reset/SPI/FIFO increments occurred through that interval.
- PASS is emitted only after resource cleanup completes. Cleanup errors
  cannot hide the original error or skip attempts to release other resources.
- Close/reopen, another 60 seconds at 30 Hz: 1800 frames/markers per stream,
  full shutdown checks pass; `/userdata/orbvi-sync-lp9wpmf4`.
- Real consumer backpressure: stop reading accel for 2 seconds, gyro retains
  1975 continuous samples at 987.116 Hz; both streams recover. Only the expected
  stalled-consumer drop count increases; no SPI/FIFO/reset errors.
  `/userdata/imu-backpressure-review.json`.
- Artifacts: `/userdata/orbvi-sync-qh8ug1hx` and
  `/userdata/orbvi-shutdown-6yoaqczt`.

IMU remains nominal 1 kHz, +/-16 g, +/-2000 dps. Prior results below are historical;
their steady-state-only acceptance did not cover the full shutdown interval.

# Current 1 kHz Configuration

2026-09-08, kernel #12: nominal 1 kHz, +/-16 g, +/-2000 dps.
IIO frequency/scales and GYRO_CONFIG0 (0x4f), ACCEL_CONFIG0 (0x50)
were read back during capture; both registers read 0x06.

- 60 seconds at 30 Hz: 1800 frames on each camera and 1800 FSYNC markers
  on each IMU stream; accel/gyro edge timestamps match.
- Actual rate: 987.173 Hz; maximum sample interval 1.138099 ms.
- 15 Hz: 45 frames/markers each; quiet phases: zero frames/markers.
- No steady-state increments in drop/reset/SPI/FIFO error counters.
- No RAW recording required. C++ executable is unchanged.
- Artifacts: `/userdata/orbvi-sync-borl5p24`.
- Full close/reopen, another 60 seconds at 30 Hz: PASS, 1800 frames/markers
  each; artifacts: `/userdata/orbvi-sync-id4zd5rh`. Terminal confirmed PASS;
  the second summary was not retrieved because ADB disconnected after the test.
- Driver host timing tests: 106 assertions passed, including 1 ms FSYNC decoding.

Boot image SHA-256:
`0158429b09fa68d8bfb22505cc88dae82b044dc6de5defe3a632e5b04f14f456`.

The 500 Hz and 200 Hz results below are historical, not the current configuration.

# C++17 Release Validation

The C++17 executable passes the independent Docker build, clean-directory build,
and C++ host option tests. It uses standard C++ threads/atomics and deterministic
camera/memory cleanup while retaining the SDK C ABI and existing command line.

Board regression on 2026-09-08:
- Four-camera RAW/CSV, single camera, invalid arguments, no-overwrite, SIGINT
  and restart: PASS, `/userdata/orbvi-demo-test-2yhm0vq0`.
- 30 Hz for 120 seconds: four cameras each 3600 frames and both IMUs each
  3600 FSYNC markers, PASS, `/userdata/orbvi-sync-5z4hawxp`.
- Full close/reopen then 30 Hz for 60 seconds: 1800 frames/markers each,
  PASS, `/userdata/orbvi-sync-4td870ht`.
- Continuous IMU output >=200 Hz, quiet intervals, matching accel/gyro markers,
  sample gaps and steady-state error counters: PASS.
- Final GPIO and IIO buffers released. Board binary matches the local build.

Executable SHA-256:
`84c58261915da6ceab9137a39c9c7623fb19090cfd25f3e34c90489a750280a4`.
The historical 5-minute test below used the preceding C executable; the C++
version was independently revalidated for 3 minutes at 30 Hz plus restart.

# Historical 500 Hz Rate Acceptance

## Extended real-time acceptance: PASS

Four complete capture lifecycles: 120 s + 60 s + 60 s + 60 s at 30 Hz,
with no pixel recording. Total 9000 trigger pulses, 9000 frames on each of
four cameras and 9000 FSYNC markers on each IMU stream in the 30 Hz phases.
Each run additionally passed 45 pulses at 15 Hz and three quiet intervals.

- Actual accel/gyro output: 493.5834..493.5999 Hz, continuously >=200 Hz.
- No missing camera frames or FSYNC markers in the accepted runs.
- Accel/gyro FSYNC timestamps matched; sample-gap checks passed.
- Steady-state drop/reset/SPI/FIFO error counters did not increase.
- Three full close/reopen cycles passed; final buffers disabled and GPIO released.
- Maximum inter-camera SDK timestamp spread: 91375 ns (not physical exposure skew).
- 120-second source pulse intervals: min 33.249791 ms, median 33.333084 ms,
  max 33.447667 ms. Board temperature at completion approximately 48 C.

Board artifacts:
- `/userdata/orbvi-sync-sdffn38l` (120 seconds).
- `/userdata/orbvi-sync-1lamwhh9` (first 60-second restart).
- `/userdata/orbvi-sync-7vhjxd1g` (second 60-second restart).
- `/userdata/orbvi-sync-hl8wzqn_` (third 60-second restart).

The initial long test using a single Python process failed with 3590/3600
camera frames on all four ports. Its pulse timestamps showed catch-up bursts
as short as 19.021 ms, violating the intended 30 Hz source cadence, while the
IMU recorded all 3600 edges. The test harness now uses a separate SCHED_FIFO
pulse process and aborts on >2 ms scheduling delay instead of catching up.
All four accepted runs above use that corrected source. No BSP changes or
reflashing were needed in this extended-verification round. Buffers are also
stopped before offline analysis to avoid irrelevant post-capture overflow.

This establishes bounded real-time capture and restart behavior, not hours-long
qualification under arbitrary application load.


Scope confirmed by the user: real-time capture only; RAW disk recording is
not an acceptance requirement. The 30 Hz camera / >=200 Hz IMU requirement
passes under this scope. Storage diagnostics below are informational only.

2026-09-08: previous BSP (kernel #11), camera external trigger up to 30 Hz,
ICM42688 nominal 500 Hz, actual approximately 493.6 Hz. This supersedes the
nominal 200 Hz firmware used for the historical tests below.

| Check | Result |
| --- | --- |
| 15 Hz: 45 pulses, four cameras and both IMU marker streams | PASS: 45 each |
| 30 Hz: 90 pulses, four cameras and both IMU marker streams | PASS: 90 each |
| IMU actual sample rate >=200 Hz, including quiet phases | PASS: approximately 493.6 Hz |
| Steady-state sample gaps, drop/reset/error increments | PASS |
| 30 Hz RAW to RAM, 1 second | PASS: 30 images per camera and 30 markers per IMU |
| RAW to onboard eMMC, 30 Hz for 3 seconds | FAIL: camera0 52/90; sustained writing not qualified |

Metadata artifacts: `/userdata/orbvi-sync-35cf4a6z`.
RAM RAW artifacts, moved to persistent storage: `/userdata/orbvi-sync-wiv8degi`.
Image SHA-256: `d114927f0b167abd4fe3dcc707b105b0168c896a5ba72bdd7ee7df2035fc05f2`.
The demo executable is unchanged; the BSP and acceptance script were updated.

Run the bounded RAM test with:

```sh
ORBVI_SYNC_OUTPUT_BASE=/dev/shm ./orbvi-verify sync /userdata/orbvi-4p-demo/run.sh --raw
```

Reserve approximately 500 MiB free RAM filesystem space. Preserve results
before reboot. Full-size four-camera RAW at 30 Hz needs approximately
320 MiB/s; the short RAM test is not a sustained eMMC recording guarantee.
IMU FSYNC marks a continuous ODR sample and supplies hardware edge timing;
it does not start an IMU physical measurement on demand.

## Historical Validation

# Validation

Tested on 2026-09-08 with HS ORBVI 4P V1.0, four SC132GS cameras and the
X5 Buildroot BSP. The demo binary was built independently using the public
headers/libraries pinned in the Dockerfile and Ubuntu AArch64 GCC. These results are a short smoke test, not
long-duration performance or synchronization qualification.

| Check | Result |
| --- | --- |
| First `build.sh` run, including Docker image creation | PASS |
| Repeated `build.sh` run using Docker cache | PASS |
| Clean source copy under /tmp, no SDK input or BSP mount | PASS |
| Host option defaults, invalid values and port selection | PASS |
| Four cameras, three RAW frames per camera | PASS |
| RAW byte counts, dimensions and nonconstant buffers | PASS |
| CSV SDK timestamps strictly increasing per camera | PASS |
| Camera 2, 30 frames, CSV only | PASS |
| Reject existing output directory and invalid frame count | PASS |
| SIGINT returns 130; subsequent four-camera capture | PASS |
| Post-flash external trigger, 15 Hz / 25 Hz | PASS: 45 / 75 frames per camera and markers per IMU |
| Trigger pauses before, between and after active phases | PASS: zero frames and FSYNC markers |
| Accel/gyro FSYNC timestamps | PASS: identical for each pulse |
| Steady-state drop/reset/SPI/FIFO error increments | PASS: zero |
| External-trigger RAW capture, 2 Hz / 5 Hz | PASS: 6 / 15 frames per camera; 84 formal RAW files |
| RAW size and nonconstant content, including startup | PASS: 94 files total |

To repeat the board smoke test, deploy `dist/orbvi-verify` and the camera executable to the board. It needs an idle camera pipeline and about 40 MiB of free
userdata space. It creates a unique `/userdata/orbvi-demo-test-*` directory,
preserves RAW/CSV/log files, and prints its path at completion.

```sh
./orbvi-verify board /userdata/orbvi-4p-demo/run.sh
```

`orbvi-verify board` checks free-running capture only. For joint external-trigger
acceptance use `orbvi-verify sync`, with `--raw` to save images. See README for
commands and hardware prerequisites. Both tests preserve their output.

Post-flash results (2026-09-08):

- Metadata synchronization: `/userdata/orbvi-sync-_y3t6yjd`.
- RAW synchronization: `/userdata/orbvi-sync-e3f1hj60` (250 MiB, includes startup).
- Earlier standalone-build free-running regression: `/userdata/orbvi-demo-test-a33r189y`.
- Boot image SHA-256: `62f04d50c4ac4533a60f5ec5c791f226e3e6f97290e4076da43d0eb6cd4f2242`.

The corrected BSP uses GPIO417 (module terminal 240) and 256-record ICM42688
buffers. The camera runtime includes the SC132GS slave stream-on fix. The demo
prepares all four sensors before starting any flow; MIPI startup checks remain
enabled. Board runtime libraries were not replaced by the independent build.

At 15/25 Hz, maximum inter-camera SDK timestamp spread was 125667 ns.
Frame timestamps followed IMU FSYNC by approximately 9.77 ms. These timestamps
have different acquisition semantics; the offset is not an exposure-sync error
measurement. Physical exposure edges and long-duration performance remain
unqualified. Startup is excluded from steady-state error-count checks; counters
were nonzero before the measured phases and did not increase within them.

Final binary regression after documentation/help-text cleanup:

- Free-running RAW, single port, signal exit and restart: PASS,
  `/userdata/orbvi-demo-test-ad56yvzv`.
- Repeated 15/25 Hz synchronization: PASS, `/userdata/orbvi-sync-rzfddwxt`.
- Deployed executable SHA-256:
  `4b366c2a4154f1541efca239a30fefcd59606c950c827a73d4be76556e9b8c78`.
- IMU buffers disabled and GPIO417 released after completion.
- USB ADB was manually disconnected by the operator (confirmed afterward);
  deployment and testing completed through the debug serial console instead.

Kernel logs still contain SIF frame-size messages during stream startup.
The bounded frame-count, RAW-content and synchronization checks pass despite
these messages; this run does not certify a warning-free camera stack.
