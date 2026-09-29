# External build dependencies

The Demo sources use the repository's MIT license. The third-party ARM64 SDK/BSP
headers and binary link inputs under `build_deps/` have **separate upstream
terms**; they are not relicensed as MIT. Their exact source revisions and
checksums are recorded in [`build_deps/README.md`](build_deps/README.md) and
[`build_deps/SHA256SUMS`](build_deps/SHA256SUMS).

| Input | Provenance | Terms / action before customer release |
| --- | --- | --- |
| `build_deps/x5/` | D-Robotics `x5-hobot-multimedia-dev` at `f625c7fd28a3553f57524c2bc3721a42078af2b7` and `x5-hobot-multimedia` at `0f297df905511c3429567202813359acee26dbb3` | Upstream Apache-2.0 license copies: `build_deps/x5/HEADERS-LICENSE`, `LIBRARIES-LICENSE`. Check any additional upstream notice requirements. |
| `build_deps/mk_api/` | Internal `d-robotics/platform_samples` at `2632ca13faca5daf1c481c4c5645f1161dee03f2`, `sunrise_camera/Transport/media_server/mk_api` | Upstream archives contain prebuilt ZLMediaKit-related libraries. **No standalone redistribution license was found alongside these archives**; confirm supplier/customer redistribution terms before merging or granting customer access. |
| `build_deps/lib/` | Matching ORBVI 4P target BSP's ARM64 OpenSSL 1.1 / zlib development link inputs | Confirm the BSP's OpenSSL/zlib redistribution notices and matching BSP version before customer release. |
| Cross compiler, glibc and base image | Ubuntu 22.04 system packages pulled by `Dockerfile` | Stay under Ubuntu/package terms; not committed to this repository. |

The `build_deps/` binaries are for cross-linking only. The actual device
requires its own matching drivers, firmware, sensor plugin, X5 runtime
libraries and OpenSSL/zlib. **Do not install build inputs over the device's
BSP**; compilation alone cannot establish runtime compatibility.
