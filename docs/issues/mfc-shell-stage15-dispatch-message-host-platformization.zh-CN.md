# MFC 壳解耦阶段 15：Dispatch Message Host 平台化（Win32 首落地）

## 1. 背景与目标

阶段 14 后，`AppController` 仍在 Core 中直接承担 Win32 消息窗口职责（`CreateWindowExW`、`WndProc`、`SendMessageW`、`SetTimer/KillTimer`），平台边界仍不干净。

本阶段目标：

- 把“消息分发宿主”抽象成 Core 接口；
- 在 `Platform/windows` 落地 Win32 消息宿主实现；
- `AppController` 改为依赖抽象，不再直接创建/维护 Win32 窗口过程。

## 2. 判定

判定：`架构演进`（非功能 Bug 修复）。

依据：

- 可见功能（指令路由、计时器触发、输入事件分发）保持一致；
- 变更是平台职责下沉与依赖方向优化。

## 3. 实施内容

### 3.1 Core 新增 Dispatch Host 抽象

新增：

- `MFCMouseEffect/MouseFx/Core/Control/IDispatchMessageHandler.h`
- `MFCMouseEffect/MouseFx/Core/Control/IDispatchMessageHost.h`
- `MFCMouseEffect/MouseFx/Core/Control/NullDispatchMessageHost.h`

接口能力：

- 创建/销毁消息宿主；
- 同步发送消息（`SendSync`）；
- 定时器设置与回收；
- 查询 native 句柄、OwnerThread、错误码。

### 3.2 平台工厂扩展

新增：

- `MFCMouseEffect/Platform/PlatformControlServicesFactory.h`
- `MFCMouseEffect/Platform/PlatformControlServicesFactory.cpp`

策略：

- Windows 返回 `Win32DispatchMessageHost`；
- 非 Windows 返回 `NullDispatchMessageHost`。

### 3.3 Win32 消息宿主实现下沉

新增：

- `MFCMouseEffect/Platform/windows/Control/Win32DispatchMessageHost.h`
- `MFCMouseEffect/Platform/windows/Control/Win32DispatchMessageHost.cpp`

实现要点：

- 封装 message-only window 生命周期；
- 在 `WndProc` 中回调 `IDispatchMessageHandler::OnDispatchMessage`；
- 封装 `SendMessageW`、`SetTimer/KillTimer`、`LastError`。

### 3.4 AppController 改为依赖 Dispatch Host

修改：

- `MFCMouseEffect/MouseFx/Core/Control/AppController.h`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.ConfigUpdates.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/DispatchRouter.cpp`

关键变更：

- `AppController` 实现 `IDispatchMessageHandler`；
- 原 `DispatchWndProc`、窗口注册/创建/销毁逻辑从 `AppController` 移除；
- `GetConfigSnapshot`、`HandleCommand`、Hold/Hover 计时器操作改走 `IDispatchMessageHost`；
- `CancelPendingHold` 从 `HWND` 入参改为宿主内部计时器回收；
- `DispatchRouter::OnButtonDown` 改用 `AppController::ArmHoldTimer()`。

### 3.5 工程清单同步

修改：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

新增编译项：

- `Platform\PlatformControlServicesFactory.cpp`
- `Platform\windows\Control\Win32DispatchMessageHost.cpp`

新增头文件项：

- `IDispatchMessageHandler` / `IDispatchMessageHost` / `NullDispatchMessageHost`
- `PlatformControlServicesFactory`
- `Win32DispatchMessageHost`

## 4. 验证

构建验证（VS 2026 MSBuild）：

- `Debug|x64`：通过
- `Release|x64`：通过

说明：

- 首次 Release 链接阶段遇到 `LNK1104`（`MFCMouseEffect.exe` 占用）；
- 已按协作约定自动结束占用进程后重试，构建通过。

## 5. 风险与边界

已控制：

- `DispatchRouter` 业务分发行为保持不变；
- 通过 `NullDispatchMessageHost` 保证非 Win32 路径可安全退化。

当前边界：

- `DispatchRouter` 仍基于 Win32 消息语义（`WM_*`、`WPARAM/LPARAM`）；
- 输入协议与消息语义完全解耦需后续继续阶段化推进。

## 6. 后续建议（阶段 16）

- 抽象输入事件总线（替代 `WM_MFX_*` 直连）；
- 让 `DispatchRouter` 逐步从 Win32 message contract 迁到平台无关 event contract；
- 再下沉 `MouseFx/Windows` 中 remaining legacy window（Text/Trail/ParticleTrail）到 `Platform/windows`。
