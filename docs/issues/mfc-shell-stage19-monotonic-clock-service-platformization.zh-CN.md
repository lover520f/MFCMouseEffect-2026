# MFC 壳解耦阶段 19：Monotonic Clock Service 平台化（Win32 首落地）

## 1. 背景与目标

阶段 18 已把光标查询下沉到平台服务，但 `AppController` / `DispatchRouter` 仍直接调用 `GetTickCount64`，时间源依赖仍在 Core 控制路径中。

本阶段目标：

- 抽象单调时钟服务到 Core 接口；
- 提供 Win32 平台实现，并为非 Win 提供标准 C++ 兜底；
- 将控制路径中的时间读取改为服务调用，减少 Win32 API 直连。

## 2. 判定

判定：`架构演进`（非功能 Bug 修复）。

依据：

- 可见行为（Hover/Hold/Wasm 事件时序）语义不变；
- 改动仅涉及时间源获取路径与依赖边界。

## 3. 实施内容

### 3.1 Core 新增时间服务接口与标准兜底

新增：

- `MFCMouseEffect/MouseFx/Core/System/IMonotonicClockService.h`
- `MFCMouseEffect/MouseFx/Core/System/StdMonotonicClockService.h`

设计要点：

- `NowMs()` 返回单调毫秒计时；
- `StdMonotonicClockService` 基于 `std::chrono::steady_clock`，用于非 Windows 或工厂兜底。

### 3.2 平台工厂新增 System Services

新增：

- `MFCMouseEffect/Platform/PlatformSystemServicesFactory.h`
- `MFCMouseEffect/Platform/PlatformSystemServicesFactory.cpp`

策略：

- Windows 返回 `Win32MonotonicClockService`；
- 非 Windows 返回 `StdMonotonicClockService`。

### 3.3 Win32 单调时钟实现

新增：

- `MFCMouseEffect/Platform/windows/System/Win32MonotonicClockService.h`
- `MFCMouseEffect/Platform/windows/System/Win32MonotonicClockService.cpp`

实现：

- 使用 `GetTickCount64` 作为 Win32 单调毫秒时钟来源。

### 3.4 AppController / DispatchRouter 切换到时间服务

修改：

- `MFCMouseEffect/MouseFx/Core/Control/AppController.h`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/DispatchRouter.cpp`

关键变更：

- `AppController` 新增 `monotonicClockService_` 依赖与 `CurrentTickMs()`；
- `AppController` 内部的输入时间戳、hold 时长、VM 抑制时序改用 `CurrentTickMs()`；
- `DispatchRouter` 触发 Wasm 事件时的 `eventTickMs` 全部改用 `ctrl_->CurrentTickMs()`。

### 3.5 工程文件同步

修改：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

新增编译项：

- `Platform\PlatformSystemServicesFactory.cpp`
- `Platform\windows\System\Win32MonotonicClockService.cpp`

新增头文件项：

- `IMonotonicClockService`
- `StdMonotonicClockService`
- `PlatformSystemServicesFactory`
- `Win32MonotonicClockService`

## 4. 验证

构建验证（VS 2026 MSBuild）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

## 5. 风险与边界

已控制：

- Win32 时间源仍为 `GetTickCount64`，只调整访问路径；
- 兜底实现为 `steady_clock`，避免空实现导致行为退化。

当前边界：

- `ShortcutCaptureSession` 仍直接调用 `GetTickCount64`；
- 时间服务目前主要接入控制链路，尚未覆盖所有子模块。

## 6. 后续建议（阶段 20）

- 将 `ShortcutCaptureSession` 与其他仍直连 Win32 时间 API 的模块统一接入时钟服务；
- 对时间相关阈值提供测试态参数入口，便于你快速验证长流程行为；
- 继续收敛 Core 中剩余 Win32 API 直连点（系统剪贴板、窗口查询等）。
