# ORBVI image reference-demo wire protocol v1 (not production transport)

The JPEG and raw NV12 demo applications are separate camera owners. Neither starts RTSP or the other demo. Each demo runs a finite number of image deliveries (`--frames-per-camera N`) for the selected camera mask; default four cameras. Camera i listens on TCP base_port+i; one connection per camera. SDK clients on the host connect before image copies/encoding. No cameras are shared with UVC or other camera applications. Existing cam_demo/RTSP/OIMU/1 interfaces stay unchanged.

## Session control

On connect the client sends exactly 8 bytes: ASCII `OCTL` (4), protocol_version 1 (u8), command START 1 (u8), camera index 0..3 (u8), reserved zero (u8). The board must validate the request; until valid START for this camera it must not copy an NV12 frame into JPEG codec input and must not JPEG encode. On disconnect, stop creating JPEG work. `--wait-timeout-ms` bounds the wait for START; an absent consumer times out with a clear result. A standalone SDK CLI demo constitutes a consumer; the ROS Bridge connects only while a compressed ROS subscriber is present.

## Frames

After START the board sends frames as a 48-byte network-byte-order header followed by the payload:
- bytes 0..3: ASCII `OIMG`;
- byte 4: version=1; byte 5: kind=1 (NV12) or 2 (JPEG); byte 6: camera index; byte 7: reserved=0;
- bytes 8..9: header_size=48 (u16); bytes 10..11: reserved=0;
- bytes 12..19: monotonically increasing per-camera sequence (u64, starts at 1);
- bytes 20..27: board capture timestamp_ns (u64, board clock domain; do NOT claim Unix/synchronized ROS time);
- bytes 28..31: width (u32); bytes 32..35: height (u32);
- bytes 36..39: stride (u32); bytes 40..43: payload_size (u32);
- bytes 44..47: CRC32/ISO-HDLC over payload bytes (u32), not header.

NV12 payload is *tightly packed*, even positive width and height, `stride=width`, length exactly width*height*3/2; Y comes first, then interleaved UV. Source padded Y and UV rows must be copied individually. JPEG payload is a complete standard JPEG interchange image, `stride=0`; width/height describe the decoded output. Receiver validates header/size/CRC before delivery and caps dimensions, payload size, and memory. Header CRC is not needed for this finite reference demo; bad header fields are rejected. Host header stamps use host receive time, and board timestamps are metadata only. Each selected camera's successful end-to-end deliveries must reach N; board TCP send success alone is not a host receipt proof. Explicitly report per-camera delivered/dropped/CRC errors.

Suggested defaults: base_port=5610, image wait timeout=10000ms, frames_per_camera=10. JPEG Bridge only starts SDK sessions after at least one ROS CompressedImage subscriber for that camera. Raw Bridge may start immediately. No RTSP image decoder or OIMU/1 wire format changes are needed.

Processor temperature uses a separate wire format and port, and is not an IMU field.

## Processor temperature transport: OCPU/1

The processor temperature demo owns **no camera**. It sends one 32-byte UDP datagram per valid processor thermal reading to a host-selected address and port, independent of the OIMU/1 IMU port. Network byte order applies to integers and IEEE-754 binary32 bits:

| Byte range | Contents |
| --- | --- |
| 0–3 | ASCII `OCPU` |
| 4 | version `1` |
| 5 | message type `1` (CPU temperature) |
| 6–7 | reserved `0` |
| 8–15 | sequence (u64, starts at 0) |
| 16–23 | board `CLOCK_MONOTONIC` timestamp nanoseconds (u64) |
| 24–27 | Celsius (IEEE-754 binary32 bits, big endian) |
| 28–31 | CRC-32/ISO-HDLC (u32 big endian) over bytes 0–27; init/final xor `0xffffffff`, reflected polynomial `0xedb88320` |

Board demo discovers the thermal zone by the exact sysfs `type=thermal-cpu`, not a fixed `thermal_zoneN` path and never an IMU temperature. Malformed/missing temperature is an error and cannot produce a default or zero reading. Host Bridge reports `sensor_msgs/msg/Temperature.temperature` in Celsius with source identifier `cpu`, with host receipt time as ROS stamp (the monotonic board clock cannot be interpreted as ROS wall time). Independently invoke either the SDK receiver CLI or the host Bridge receiver, never two UDP receivers on the same bound endpoint for a one-shot acceptance test.

## Camera ownership boundary

Choosing a capture mode via CLI parameters documents intent, but a parameter by itself does not arbitrate between concurrent processes. Operators must stop any camera-owning RTSP, UVC, or other camera process before launching either image demo. If multiple independent launchers can run concurrently, use a shared exclusive lock or equivalent supervisor across *all* camera-owning launchers; locking only these new image demos cannot protect against a legacy UVC process that does not participate. Temperature/IMU demos do not claim the camera.
