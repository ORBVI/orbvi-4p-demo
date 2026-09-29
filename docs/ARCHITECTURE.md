# C++ 模块设计

参考 [RoboBaton_4p_demo](https://github.com/Hessian-matrix/RoboBaton_4p_demo)
对入口、配置、采集流程和输出职责的划分。本项目独立实现接口，没有引入其
二进制库或复制其源码；仍使用固定版本的 D-Robotics 公开编译依赖。

## 模块边界

| 文件 | 职责 | 是否需要 X5 SDK |
| --- | --- | --- |
| `src/main.cpp` | 进程入口 | 否 |
| `src/application.*` | 板型检查、信号、内存模块、对象组装 | 是 |
| `src/options.*` | CLI 参数和默认值 | 否 |
| `src/capture_session.*` | 启停顺序、每路线程、超时、退出结果 | 否 |
| `src/x5_camera.*` | SC132GS/MIPI/VIN 配置、SDK 句柄与帧借用 | 仅实现文件 |
| `src/file_sink.*` | CSV 元数据、可选 RAW 文件、不覆盖已有文件 | 否 |

调用关系：`main -> application -> CaptureSession -> CameraSource / FrameSink`。
应用层注入 `X5Camera` 和 `FileSink`；主机测试注入假相机和假输出，无需板卡和 SDK。
SDK 类型通过 `X5Camera` 的私有实现隐藏，不进入采集流程和输出接口。

## 生命周期与并发

1. 应用检查板型、创建新输出目录、安装信号处理器并打开共享内存模块。
2. 会话准备传感器并打开输出。外触发模式始终先准备四颗传感器，
   即使只采一路，也必须等四颗共享 FSYNC 的从模式配置完成才启动流。
3. 启动选定通道，每路一个工作线程。全部成功启动后输出 `Capture ready`。
4. 每次借帧后同步调用输出接口，无论写入成功、失败或抛异常，都归还该帧。
5. 达到帧数、收到信号、超时或发生错误后，等待全部工作线程退出，
   按端口逆序停止流、销毁相机并关闭输出，最后关闭共享内存模块。

每个相机和输出对象仅由对应的采集线程使用；主线程等待线程结束后才清理。
会话内错误标志是原子变量；信号处理器只更新 `sig_atomic_t`。
会话只允许运行一次，依赖对象必须比 `run()` 活得更久。
`close()` 必须支持重复调用和部分初始化后的清理，并返回错误而不抛异常。
正常退出返回 0，参数错误返回 2，信号退出返回 130；采集或清理错误优先返回 1。

## 接入处理算法或其他输出

实现 `FrameSink` 的 `open(port)`、`write(index, frame)`、`close()`，
在应用层替换对应通道的 `FileSink` 即可，不需要修改 SC132GS 初始化和采集线程。
`FrameView` 提供像素地址、字节数、尺寸、步长、帧号和三个时间戳。
SDK 两个时间戳保留原值；`received_ns` 是主机调用返回后的单调时钟纳秒数。
不要把接收时间当作曝光边沿时间。

`FrameView::data` 只在这次 `write()` 返回前有效，禁止保存地址供后台线程使用。
异步处理必须复制数据到自行拥有的缓冲区，并自行定义有界队列、背压和丢帧策略。
当前输出同步执行，没有隐含的无限缓存；`--discard` 不保存 RAW，仍写元数据。

替换相机后端可实现 `CameraSource`：成功 `acquire()` 借出一帧，
后续由会话调用一次 `release()`；非零返回值按当前 CLI 的无帧超时策略重试。
硬件相关改动需额外进行实板验证，主机假后端不能证明电气或曝光同步。

## IMU 与外触发

C++ 可执行程序负责相机采集，不自行驱动 FSYNC，也不包含 IMU 采集器。
`orbvi-verify sync` 是 C++ 外触发与 IMU 的联合验收入口：相机验证最高 30 Hz，
IMU 配置为 1 kHz、16 g、2000 dps，检查两路 IIO 数据中的 FSYNC 标记。
重构不改变触发 GPIO、IIO 配置和验收判据，不引入 RTSP、ROS 或录包依赖。

## C++ 验证工具

`tools/verify_main.cpp` 解析 `sync` / `board` / `shutdown` / `backpressure` 子命令。
`tools/workflows.cpp` 编排板端流程，使用独立实时线程产生 GPIO417 脉冲；
主线程读取两路 IIO，采集子进程仍运行 `orbvi-4p-demo`。
`tools/runtime.*` 封装子进程、IIO 配置与恢复、分片读取、CSV/RAW 校验和 JSON 写入。
`tools/validation.*` 实现 scan layout 解码、采样连续性、持续排空的退出流程和逐项清理；
`tools/phase.cpp` 按硬件时间戳划分阶段，检查帧数、FSYNC 及 IMU 采样。

验证工具不依赖 X5 SDK，不调用解释器。启动 IIO 前先完成两路设备预检查，
恢复时先关闭 buffer，再关闭描述符，恢复 length 和 scan 设置、watermark，最后恢复 enable。
触发线程在清理 GPIO 前停止并 join；相机正常退出期间主线程继续读取 IMU。
任何校验、清理或报告写入失败都不会输出最终 PASS。
JSON 保持纳秒整数精度；报告为紧凑 JSON，新增 status/error/cleanup_errors 字段。
背压报告的 samples 与同步诊断一致，使用带 device 字段的记录数组。

主机测试使用临时目录和 FIFO 验证真实非阻塞分片读取、单路暂停和配置恢复，
不访问板端 GPIO/IIO。历史板测仍需在本次迁移后重新执行。

## 验证

```bash
make test test-sync
bash tests/build-standalone.sh
```

主机测试覆盖参数、先准备后启动、部分初始化失败、输出异常时还帧、
无帧超时、逆序清理，以及清理错误优先于信号退出。
板端命令及实测边界见 `VALIDATION.md`。
