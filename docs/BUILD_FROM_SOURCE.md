# 从源码编译板端 Demo

客户只需克隆**本 Demo 仓库**；X5 头文件及链接库、BSP `mk_api`、目标板同版本 OpenSSL/zlib 链接库已纳入 `build_deps/`。无需访问 BSP 或 SDK 源码仓库，也不需要将 `build_deps/` 上传到板端。

## 构建完整 Demo

编译机需要 Docker、Bash 和 Git；Docker 初次构建仍需获取 Ubuntu 22.04 基础镜像和发行版 AArch64 工具链软件包。位于仓库根目录运行：

```sh
sha256sum -c build_deps/SHA256SUMS
./build-all.sh
```

`build-all.sh` 在 Docker 中交叉编译 RAW 主程序、相机/RTSP、IMU 和其他板端示例，并在 `dist/` 生成完整产物；编译阶段不接入网络。客户无需配置 `HBN_ROOT`、`RTSP_ROOT`、`HBN_SYS_LIB` 或电脑上的交叉编译器。**只部署 `dist/`，不要部署 `build_deps/`。**

如仅构建 RAW 主程序及验证工具，执行 `./build.sh` 即可；此命令**不会**构建 `apps/`。已有自己的交叉编译器时，也可执行 `./scripts/build-customer.sh`，脚本默认使用仓库中的编译输入，必要时可覆盖 `HBN_ROOT`、`RTSP_ROOT`、`HBN_SYS_LIB`、`CXX`。

## 交付边界

构建输入的来源和版本见 [`build_deps/README.md`](../build_deps/README.md)。运行时须使用目标板兼容 BSP、驱动和动态库；**编译通过不代表跨 BSP 版本可运行**。本目录的第三方库仅作为交叉编译/链接输入，不能据此覆盖设备运行库。客户再分发与许可 notice 须经交付方审核。
