# Independent finite reference demos: acceptance plan

These are independent invocations, not a permanently combined camera daemon. Keep the existing RTSP cam_demo and OIMU/1 imu_reader_demo unchanged. Never run two camera-owning demos on the same physical cameras concurrently; an enabled UVC production pipeline is also a camera owner and must be handled by the existing operational procedure.

1. NV12 demo: run selected 1/4 camera sources for N frames each; host SDK CLI receives exactly N valid tightly packed NV12 images per selected camera (CRC valid, even dimensions, size width*height*3/2); ROS launch independently emits N per camera `sensor_msgs/msg/Image` with explicit project-specific NV12 encoding contract, and corresponding CameraInfo. Do not imply standard Humble image tooling recognizes NV12: `sensor_msgs/Image.msg` documents `data.size = step * height`, while tightly packed NV12 needs `3 * step * height / 2`; this reference demo uses a project-specific payload contract and requires a matching consumer rather than stock `cv_bridge`. N bounded transmissions do not imply sustained four-camera full-frame-rate Ethernet throughput.
2. JPEG demo: without a consumer or compressed ROS subscriber, verify zero JPEG codec input copies/encodes; after the subscriber appears, verify N real hardware JPEG frames per selected camera, JPEG signatures/decodability, and ROS CompressedImage(format='jpeg') received N times. Ensure removing subscriber ceases new encode work; board exit/disconnect and input/output codec buffer reclamation must not block existing camera sources.
3. Processor thermal demo: discover processor thermal zone by type (not IMU and not fixed zone number), publish N valid degree-C readings with source identification; malformed or missing sysfs temp never publishes a fabricated zero. Use OCPU/1 32-byte UDP wire format (specified in REFERENCE_IMAGE_TRANSPORT_V1.md) and independent port from OIMU/1. Test CRC, monotonic timestamp interpretation, and invalid temperature rejection.
4. Compatibility: run preexisting four-camera RTSP-to-SDK-to-ROS Image(mono8) tests and UDP IMU-to-SDK-to-ROS Imu tests before/after. New modes opt-in; historical launch/config paths unchanged. A separate demo's CPU temperature can run without opening any camera.

Use host SDK standalone example OR the ROS Bridge for each demo, not both simultaneously against a one-client/one-shot board listener. Frame sequence/timestamps record board clock metadata but ROS headers use documented host receipt time; no false hardware synchronization claim. Hardware codec and real board sysfs need real X5 acceptance beyond host unit/cross-build results. Existing user work-in-progress in the original demo checkout must not be modified or overwritten.

## Example invocations (each camera run is separate)

On the X5 board, after stopping all previous RTSP/UVC camera owners, deploy the
built `dist/` tree with its `bin/` subdirectory intact. Use one of:

```sh
./dist/nv12_demo --camera-mask 0xf --frames-per-camera 10 --base-port 5610
./dist/jpeg_demo --camera-mask 0xf --frames-per-camera 10 --base-port 5610
```

For either run, start **one** host consumer within the board START timeout:

```sh
orbvi_sdk_image_stream_demo --host BOARD_IPV4 --kind nv12 --camera-mask 0xf --frames-per-camera 10 --output-dir /tmp/raw-images
# Or replace nv12 with jpeg and use a different output directory.
```

Alternatively, start the corresponding **host-side** ROS launch shown in the
Bridge README instead of the SDK CLI. For JPEG, subscribe to the selected
`/orbvi/raw/camera_i/image/compressed` topics before the board's START timeout
(for all four cameras when using mask `0xf`); a `camera_info` subscription alone
does not activate JPEG. Keep each subscription alive until its N messages have
arrived. A board timeout or a partial N is a failed test, not a successful idle
service.

Processor temperature has a separate camera-free invocation. First start the
host temperature CLI **or** ROS temperature launch, then on the board run:

```sh
./dist/bin/cpu_temp_demo --samples 10 --period-ms 1000 --udp-dest HOST_IPV4:5620
```

`--thermal-root` is only for a known processor thermal sysfs root or a test
fixture. The default `/sys/class/thermal` is auto-discovered by exact zone
type `thermal-cpu`. The host must collect N valid CPU samples; do not interpret
board UDP send success as guaranteed host delivery.

## X5 hardware acceptance record (September 24, 2026)

This is a real X5 board test at `192.168.1.10` (host `192.168.1.111`), not an emulation. The temporary deployment under `/userdata/orbvi-4p-demo/reference-qa-20260924/` did **not** replace production executables. Each camera test first stopped the already-running production UVC camera owner with `/opt/orbvi/current/scripts/x5_camera_service.sh` using `ORBVI_X5_ROOT=/opt/orbvi/current ORBVI_X5_PRODUCT=4p-x5`, then used an `EXIT` trap to restart the original service and confirmed its `status` was running. No SSH password or ADB access was used; an already-authorized board serial shell and board `wget` transported the QA executable. The final tested AArch64 `reference_images_demo` SHA-256 is `57b04498de5ffd46a81438358c2c304086c2281969cd793cafa1abce4aa330e3`.

| Independent invocation | Board -> host validation | Observation |
| --- | --- | --- |
| `cpu_temp_demo --samples 10 --period-ms 300 --udp-dest HOST:5620` | SDK standalone OCPU consumer, followed by a **separate** ROS `sensor_msgs/Temperature` session | Both 10/10 CPU readings, approximately 58 °C, `header.frame_id=cpu`, correct ROS stamps. Processor `thermal-cpu` sysfs (not IMU). |
| `nv12_demo --camera-mask 0xf --frames-per-camera 3` | SDK standalone OIMG consumer | Camera 0–3 each 3/3, 0 drops/CRC errors; twelve 1,088×1,280 tightly packed NV12 frames (2,088,960 bytes each). |
| Same NV12 board demo, a new independent session | Host ROS Bridge with **reliable** image and CameraInfo subscribers | Four `sensor_msgs/Image(encoding=nv12)` topics each 3/3 plus four CameraInfo topics each 3/3. Bridge retained publishers through finite ROS drain. |
| `jpeg_demo --camera-mask 0xf --frames-per-camera 3` | SDK standalone JPEG consumer | Camera 0–3 each 3/3, 0 drops/CRC errors; twelve real `MEDIA_CODEC_ID_JPEG` frames decoded to actual pixels with Pillow, dimensions 1,088×1,280; complete SOI/EOI. X5 JPEG output reports up to 7 zero alignment bytes after EOI: board demo now trims at most 15 zero bytes **only** after validating complete JPEG and SOF dimensions. |
| Same JPEG board demo, a new independent session | Host ROS Bridge with **reliable** `sensor_msgs/CompressedImage` and CameraInfo subscribers | Four `format=jpeg` topics each 3/3; each image decoded with Pillow; four CameraInfo topics each 3/3. Before subscribing, the board remained at `awaiting jpeg START` with no encode lines; after subscriber discovery it encoded and sent exactly N. |
| JPEG demo with no subscriber | Board-only timeout; concurrent NV12 demo probes lock | Before timeout, no START and no encode. NV12 demo failed to acquire `/run/lock/orbvi-reference-camera-owner.lock`; JPEG timed out and returned nonzero, production UVC restored. Existing UVC/RTSP are **not** lock participants and must be stopped separately. |

The initial NV12 ROS trial used a best-effort, shallow QoS subscriber and got only 2/3 large image payloads per camera although CameraInfo was 3/3. The subsequent Bridge fix uses a reliable, deeper publishing queue and a bounded ROS drain; the final **reliable** subscriber trial got all 3/3 images and CameraInfo on all cameras. Best-effort DDS does not promise exact subscriber delivery. The first hardware JPEG trial also failed because the X5 codec reports aligned, zero-padded output; it was fixed and retested to 12/12 decoded JPEGs. Keep these failure observations in the acceptance record rather than treating the original untested cross-build as a hardware pass.

Final host regressions: SDK CTest **35/35** (`sdk-test.log`), rebuilt Bridge CTest **20/20** (`bridge-final-test.log`), reference-image protocol/stride and mocked JPEG codec tests (`demo-host-image.log`), CPU sysfs/protocol tests (`demo-host-temp.log`), and the preexisting demo standalone build (`demo-standalone.log`) all passed. The final rebuilt Bridge binary was then used for the separate four-camera NV12 and JPEG ROS sessions above (`nv12-ros-finalbuild-{node,sub}.log` and `jpeg-ros-finalbuild-{node,sub}.log`). The earlier RTSP/OIMU board acceptance is recorded separately in `/tmp/orbvi-acceptance-20260924-101113/`: SDK IMU 1000 valid packets with zero CRC/parse/sequence errors, four RTSP feeds with at least 10 decoded frames each, ROS four-camera Image and 500 Imu messages. These are short-run checks, not a long-duration reliability claim.

Host test logs are staged under `/tmp/orbvi-board-qa-20260924/` (outside the repository); board test logs are in the temporary deployment directory. The production UVC service was verified running again after **every** camera invocation; the existing IMU daemon remained running.
