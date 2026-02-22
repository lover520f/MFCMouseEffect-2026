# MFC 壳解耦阶段 8：事件循环任务派发能力（PostTask）落位

## 1. 背景与目标

阶段 7 已将 POSIX 事件循环从轮询改为阻塞唤醒，但壳层动作（打开设置、退出）仍可能由不同线程直接触发。
为后续接入 macOS/Linux 原生托盘回调线程，本阶段补齐统一的“投递到事件循环线程执行”能力。

目标：

- 在 `IEventLoopService` 中引入 `PostTask` 接口；
- Windows 与 POSIX 实现统一支持任务投递；
- `AppShellCore` 的 shell 动作通过事件循环调度执行，降低跨线程调用风险。

## 2. 判定

判定：`架构演进`（线程模型增强，非功能 Bug 修复）。

依据：

- 可见行为不变（设置入口/退出行为保持一致）；
- 内部执行路径从“直接跨线程调用”演进为“事件循环线程串行执行”。

## 3. 实施内容

### 3.1 核心接口扩展

文件：`MFCMouseEffect/MouseFx/Core/Shell/IEventLoopService.h`

- 新增：
  - `bool PostTask(std::function<void()> task);`

### 3.2 Windows 事件循环支持任务派发

文件：

- `MFCMouseEffect/Platform/windows/Shell/Win32EventLoopService.h`
- `MFCMouseEffect/Platform/windows/Shell/Win32EventLoopService.cpp`

要点：

- 引入线程安全任务队列；
- `PostTask` 通过 `PostThreadMessageW` 唤醒主消息循环；
- 在消息循环中处理内部任务消息并执行队列任务。

### 3.3 POSIX 公共事件循环支持任务派发

文件：

- `MFCMouseEffect/Platform/posix/Shell/PosixBlockingEventLoop.h`
- `MFCMouseEffect/Platform/posix/Shell/PosixBlockingEventLoop.cpp`

要点：

- 在阻塞循环中加入任务队列；
- `PostTask` 入队后 `condition_variable` 唤醒；
- 事件循环线程串行执行任务。

### 3.4 macOS/Linux EventLoopService 适配

文件：

- `MFCMouseEffect/Platform/macos/Shell/MacosEventLoopService.h/.cpp`
- `MFCMouseEffect/Platform/linux/Shell/LinuxEventLoopService.h/.cpp`

要点：

- 新增 `PostTask` 实现；
- 统一委托到 `PosixBlockingEventLoop`。

### 3.5 AppShellCore 接入事件循环派发

文件：`MFCMouseEffect/MouseFx/Core/Shell/AppShellCore.h/.cpp`

变更：

- 新增 `PostShellTask` 与 `RequestExitOnLoop`；
- `OpenSettingsFromShell()` 与 `RequestExitFromShell()` 优先走 `PostTask`；
- 若派发失败，回落到同步执行，保证功能可用性。

### 3.6 平台文档同步

- `MFCMouseEffect/Platform/macos/Shell/README.md`
- `MFCMouseEffect/Platform/linux/Shell/README.md`

更新 `EventLoopService` 描述为支持 `PostTask`。

## 4. 验证

Windows 回归构建：

- `Release|x64`：通过
- `Debug|x64`：通过

说明：

- 新增/修改 POSIX 文件不进入当前 Windows 编译目标；
- Windows 现有托盘与主循环行为不回归。

## 5. 风险与边界

已控制：

- 保留失败回落路径，避免调度失败直接影响用户操作；
- 壳层公共动作统一走事件循环线程，降低跨线程 UI 调用风险。

当前边界：

- 仍未接入 macOS/Linux 原生主循环（`CFRunLoop`/`GLib`）；
- 任务优先级、延迟任务、取消机制尚未提供。

## 6. 后续建议（阶段 9）

- 抽象 `IEventLoopTaskScheduler`（支持延时任务/取消句柄）；
- 对接 macOS `CFRunLoop` 与 Linux `GLib MainLoop`；
- 将 macOS/Linux tray 从 stub 升级为原生图标和菜单，并通过 `PostTask` 对接 `AppShellCore`。
