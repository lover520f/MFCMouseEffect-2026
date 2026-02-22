# MFC 壳解耦阶段 20：Shortcut Capture 与前台进程缓存时间源收敛

## 1. 背景与目标

阶段 19 已将 `AppController/DispatchRouter` 的时间读取切到 `IMonotonicClockService`，但 Core 仍有两处残留：

- `ShortcutCaptureSession` 直接使用 `GetTickCount64`；
- `ForegroundProcessResolver` 的默认参数绑定 `::GetTickCount64`。

本阶段目标：

- 让快捷键捕获会话复用统一时钟服务；
- 清理前台进程缓存逻辑里的 Win32 时间默认参数；
- 进一步降低 Core 对 Win32 时间 API 的直接依赖。

## 2. 判定

判定：`架构演进`（非功能 Bug 修复）。

依据：

- 快捷键捕获和前台进程匹配行为保持一致；
- 变更聚焦时间源获取路径，不改变业务规则。

## 3. 实施内容

### 3.1 ShortcutCaptureSession 接入统一时钟服务

修改：

- `MFCMouseEffect/MouseFx/Core/Automation/ShortcutCaptureSession.h`
- `MFCMouseEffect/MouseFx/Core/Automation/ShortcutCaptureSession.cpp`

关键变更：

- 新增 `SetClockService(const IMonotonicClockService*)`；
- `NowMs()` 改为：优先使用注入的 `IMonotonicClockService`，无注入时回退 `std::chrono::steady_clock`；
- `IsModifierKey/KeyTokenFromVk` 参数统一为 `uint32_t`，减少头文件对 Win32 类型暴露。

### 3.2 AppController 注入会话时钟

修改：

- `MFCMouseEffect/MouseFx/Core/Control/AppController.cpp`

关键变更：

- 在构造阶段完成 `shortcutCaptureSession_` 的时钟注入：
  - `shortcutCaptureSession_.SetClockService(monotonicClockService_.get())`。

### 3.3 ForegroundProcessResolver 移除 Win32 时间默认参数

修改：

- `MFCMouseEffect/MouseFx/Core/System/ForegroundProcessResolver.h`

关键变更：

- `CurrentProcessBaseName()` 拆分为：
  - 无参版本：内部用 `steady_clock` 获取当前 tick；
  - 带参版本：保留外部显式传入 tick 的能力；
- 删除 `::GetTickCount64` 默认参数。

## 4. 验证

构建验证（VS 2026 MSBuild）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

## 5. 风险与边界

已控制：

- 超时窗口、缓存窗口阈值未调整，仅替换时钟来源；
- ShortcutCapture 仍有会话内互斥保护，线程安全行为保持不变。

当前边界：

- `ForegroundProcessResolver` 仍使用 Win32 进程查询 API（这是其平台职责，后续可继续下沉到 `Platform/windows`）；
- `Platform/windows/Overlay/Win32InputIndicatorOverlay.cpp` 中时间读取仍是平台内直连 `GetTickCount64`（不属于 Core 残留）。

## 6. 后续建议（阶段 21）

- 将 `ForegroundProcessResolver` 抽象为平台服务并迁到 `Platform/windows/System`；
- 统一平台层时间读取策略（必要时让 `Win32InputIndicatorOverlay` 也复用同一时钟服务）；
- 继续按能力切片下沉 Core 中剩余 Win32 专有系统访问点。
