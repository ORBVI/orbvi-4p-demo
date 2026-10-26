# ORBVI 4P NV12 frame-set capture

This C++ entry point uses the ORBVI X5 BSP's camera/VIN/ISP/VSE nodes. It does
not load RoboBaton's `libsc132.so` or other third-party prebuilt runtime.
The sensor sends RAW10 over MIPI; the ISP and VSE output NV12 with separate Y
and UV planes. Four VSE frames form one set only when their SDK timestamps fit
within `--max-skew-ns`. The oldest frame is released and retried otherwise.

This is a separate capture path; the default free-running mode has not completed
full board acceptance. The default RAW/FSYNC regression contract is retained.
The initial NV12
path is free-running at the existing SC132GS sensor setting (60 fps), outputs
the sensor's native 1088x1280 orientation, and does not provide RTSP, IMU,
rotation, or an internal trigger generator. Do not treat a frame-set's timestamp spread
as physical exposure synchronization without a board measurement.

## Build and test

From the repository root:

```sh
sh nv12/test.sh
docker run --rm --network none -v "$PWD:/work" -w /work \
  orbvi-4p-demo-builder:standalone sh nv12/build.sh
```

Or set `HBN_ROOT` to the compatible local X5 SDK installation and `CXX` to its
cross-compiler, then run `sh nv12/build.sh`. The ARM64 result is
`dist/orbvi-nv12-demo`. The Docker example uses the project's existing local
build image; it performs no download and does not replace board runtime libs.

On the HS ORBVI 4P V1.0 board, copy `dist/orbvi-nv12-demo` and `nv12/run.sh` to
the same directory, keep `cam-service` running, and stop other camera apps:

```sh
./run.sh --frames 10 --output /userdata/nv12-session-001
./run.sh --frames 300 --discard --output /userdata/nv12-metadata-001
```

The output directory must not exist. Each completed group writes
`camera0_000000.nv12` through `camera3_000000.nv12`, plus one row in
`frame_sets.csv`. Files concatenate the Y allocation and UV allocation as
returned by the X5 VSE. These allocations can include stride/vstride padding;
they are not JPEG, PNG, or packed RAW10. The CSV records the group's minimum
SDK timestamp, maximum observed skew and each camera's frame ID/timestamp.
`--discard` writes only CSV. A failed or interrupted group can leave files
without a corresponding CSV row; such directories are not complete datasets.

For board acceptance, confirm four visible camera images (not just file sizes),
Y/UV layout, orientation, frame ID continuity, skew/drop counts, memory/CPU
pressure, signal cleanup, and reopen after failure. Host tests and cross-builds
do not establish these properties.


## Four-camera external-trigger 60fps

Use `--mode external --fps 60 --vts 1400 --trigger-readout` for the
SC132GS Trig-Readout configuration. The capture entry point receives external
pulses; `orbvi-verify trigger-nv12 ./run.sh --fps 60 --seconds 60` owns GPIO417,
selects this configuration automatically and checks the captured frame sets.
Do not run it concurrently with another camera owner or trigger source.

For a self-contained RTSP preview, use `mosaic_rtsp_demo --external --fps 60`.
See [60fps capture and direct Ethernet RTSP](../docs/CAMERA_TRIGGER_60FPS.md)
for board and receiver commands and timestamp semantics.
