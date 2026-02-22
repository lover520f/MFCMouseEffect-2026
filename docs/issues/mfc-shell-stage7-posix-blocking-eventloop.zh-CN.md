# MFC 壳解耦阶段 7：POSIX 事件循环去轮询（阻塞唤醒版）

## 1. 背景与目标

阶段 5/6 中 macOS 与 Linux 的 `EventLoopService` 使用了 `10ms` 轮询睡眠。
该方案能跑通流程，但会带来无意义唤醒与额外 CPU 抖动。

本阶段目标：

- 将 POSIX 事件循环从“轮询”升级为“阻塞等待 + 退出唤醒”；
- 保持接口不变（`Run/RequestExit`），不破坏现有壳层架构。

## 2. 判定

判定：`架构演进`（稳定性/能效优化，非功能回归修复）。

依据：

- 外部可见行为不变（仍由 `RequestExit` 结束主循环）；
- 内部实现从 sleep-polling 改为条件变量唤醒，减少空转。

## 3. 实施内容

### 3.1 新增 POSIX 公共阻塞循环组件

目录：`MFCMouseEffect/Platform/posix/Shell/`

- `PosixBlockingEventLoop.h/.cpp`
  - `Run()`：阻塞等待退出信号；
  - `RequestExit()`：设置退出标记并通知条件变量；
  - 防御性处理重复 `Run()`（返回 `-1`）。

### 3.2 macOS 事件循环改造

文件：

- `MFCMouseEffect/Platform/macos/Shell/MacosEventLoopService.h`
- `MFCMouseEffect/Platform/macos/Shell/MacosEventLoopService.cpp`

变更：

- 移除原 `std::atomic + sleep_for(10ms)` 轮询实现；
- 复用 `PosixBlockingEventLoop`。

### 3.3 Linux 事件循环改造

文件：

- `MFCMouseEffect/Platform/linux/Shell/LinuxEventLoopService.h`
- `MFCMouseEffect/Platform/linux/Shell/LinuxEventLoopService.cpp`

变更：

- 移除原 `std::atomic + sleep_for(10ms)` 轮询实现；
- 复用 `PosixBlockingEventLoop`。

### 3.4 平台 README 同步

- `MFCMouseEffect/Platform/macos/Shell/README.md`
- `MFCMouseEffect/Platform/linux/Shell/README.md`

将 `EventLoopService` 描述更新为“POSIX 阻塞循环（无轮询）”。

## 4. 验证

Windows 回归构建（保证主线无影响）：

- `Release|x64`：通过
- `Debug|x64`：通过

说明：

- 新增 POSIX 文件未进入当前 Windows 工程编译清单，不影响现有发布产物；
- Windows 主流程与托盘行为保持不变。

## 5. 风险与边界

已控制：

- 仅替换 POSIX 事件循环内部机制，接口边界未变化；
- 通过公共组件减少 macOS/Linux 重复代码。

当前边界：

- 仍非平台原生主循环（macOS `CFRunLoop` / Linux `GLib` 等）；
- 本阶段未引入任务派发能力，仅处理“等待退出”。

## 6. 后续建议（阶段 8）

- 引入 `EventLoopTaskDispatcher`，支持跨线程投递壳层任务；
- 在 macOS/Linux 上分别切到原生主循环桥接；
- 托盘服务从 stub 升级为原生图标+菜单实现。
