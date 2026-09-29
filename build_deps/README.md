# 板端 Demo 的版本化编译输入

本目录包含客户在**只有 Demo 仓库**时编译完整 Demo 所需的头文件、ARM64 链接库；不是板端部署目录。直接在仓库根目录执行 `./build-all.sh`；只向板端部署 `dist/`，不得以此处的库覆盖板端 BSP。

| 路径 | 编译用途 | 来源 / 版本 |
| --- | --- | --- |
| `x5/include/`、`x5/lib/` | X5 SDK 头文件及直接链接库和其依赖闭包 | D-Robotics `x5-hobot-multimedia-dev` `f625c7fd28a3553f57524c2bc3721a42078af2b7`、`x5-hobot-multimedia` `0f297df905511c3429567202813359acee26dbb3`；上游许可见 `x5/*-LICENSE` |
| `mk_api/include/`、`mk_api/lib/` | BSP RTSP API 头文件、8 个 ARM64 静态库 | 内部 `d-robotics/platform_samples` 提交 `2632ca13faca5daf1c481c4c5645f1161dee03f2`，路径 `sunrise_camera/Transport/media_server/mk_api` |
| `lib/` | 目标 BSP ARM64 OpenSSL 1.1、zlib 的**链接输入** | 2026-09-23 验证的 ORBVI 4P BSP 系统库，与现场目标板 `/usr/lib` 逐文件核对 |

以上二进制不属于 Demo 自有 MIT 授权。第三方依赖的客户再分发权限、notice 要求以及目标 BSP 兼容性**需要交付方审核**；合并/向客户开放此 MR 前应完成审核。此仓库不提供工具链、固件或运行时 BSP；Docker 构建使用 Ubuntu 22.04 系统交叉编译软件包。

`x5` 不再是本机符号链接，客户构建也不依赖任何本机 `.build/` 缓存。发布时建议运行 `sha256sum -c build_deps/SHA256SUMS` 验证编译输入完整性。
