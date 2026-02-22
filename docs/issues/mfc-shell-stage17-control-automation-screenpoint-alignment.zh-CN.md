# MFC 壳解耦阶段 17：控制与自动化链路坐标协议收敛（ScreenPoint）

## 1. 背景与目标

阶段 16 完成了 Dispatch message 编解码平台化，但控制链路内部仍存在 `POINT` 扩散：

- `DispatchRouter` 在 Core 中大量使用 Win32 点位类型；
- `AppController` 的移动/长按/悬停状态接口仍是 `POINT`；
- `InputAutomationEngine` 与 `GestureRecognizer` 仍把 `POINT` 作为公共输入。

这会让 Core 自动化路径持续绑定 Win32 坐标协议，不利于后续 macOS/Linux 接入。

本阶段目标：

- 将 `DispatchRouter -> AppController -> InputAutomationEngine -> GestureRecognizer` 统一为 `ScreenPoint`；
- 在控制链路中移除 `InputTypesWin32` 依赖；
- 保持现有效果行为与自动化触发语义不变。

## 2. 判定

判定：`架构演进`（非功能 Bug 修复）。

依据：

- 对用户可见的点击/移动/滚轮/长按/悬停效果无语义变更；
- 主要变更为坐标协议与接口边界收敛。

## 3. 实施内容

### 3.1 AppController 控制接口改为 ScreenPoint

修改：

- `MFCMouseEffect/MouseFx/Core/Control/AppController.h`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.cpp`

关键变更：

- 公开给 `DispatchRouter` 的以下方法全部改用 `ScreenPoint`：
  - `ConsumeLatestMove`
  - `BeginHoldTracking`
  - `ConsumePendingHold`
  - `TryEnterHover`
- `PendingHold::pt` 从 `POINT` 改为 `ScreenPoint`；
- `ConsumeLatestMove` 不再做 Win32 转换，直接透传 Hook 返回的 `ScreenPoint`。

### 3.2 DispatchRouter 去 Win32 坐标桥接头依赖

修改：

- `MFCMouseEffect/MouseFx/Core/Control/DispatchRouter.cpp`

关键变更：

- 移除 `InputTypesWin32.h` 依赖；
- 新增本地 helper：
  - `MessagePoint(...)`：从消息载荷构造 `ScreenPoint`
  - `TryReadCursorScreenPoint(...)`：读取系统光标并转换为 `ScreenPoint`
- 路由分支（Move/Scroll/ButtonDown/ButtonUp/Hover/Hold）统一使用 `ScreenPoint`；
- 给 Effect、Wasm 事件、输入自动化都直接传 `ScreenPoint`。

### 3.3 输入自动化引擎改为 ScreenPoint

修改：

- `MFCMouseEffect/MouseFx/Core/Automation/InputAutomationEngine.h`
- `MFCMouseEffect/MouseFx/Core/Automation/InputAutomationEngine.cpp`

关键变更：

- `OnMouseMove/OnButtonDown/OnButtonUp` 入参由 `POINT` 改为 `ScreenPoint`；
- 对 `GestureRecognizer` 的调用链同步为 `ScreenPoint`。

### 3.4 GestureRecognizer 坐标协议去 Win32 绑定

修改：

- `MFCMouseEffect/MouseFx/Core/Input/GestureRecognizer.h`
- `MFCMouseEffect/MouseFx/Core/Input/GestureRecognizer.cpp`

关键变更：

- 头文件改依赖 `InputTypes.h`，移除 `windows.h`；
- 采样缓存与几何计算从 `POINT` 全量切换为 `ScreenPoint`；
- 方向量化、距离计算逻辑保持不变。

## 4. 验证

构建验证（VS 2026 MSBuild）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

## 5. 风险与边界

已控制：

- 坐标值本身仍来自同一 Win32 数据源（消息与 `GetCursorPos`）；
- 只改数据类型与接口签名，不改触发阈值、动作识别规则和渲染调度。

当前边界：

- `DispatchRouter` 仍运行在 Win32 事件循环上（消息源平台化已做，输入源尚在 Windows）；
- `AppController` 文件仍包含 `windows.h`（涉及 Host 生命周期与少量 Win32 API）。

## 6. 后续建议（阶段 18）

- 抽象 `CursorPositionProvider`/`InputClock`，继续减少控制路径对 Win32 API 的直接调用；
- 把 Core 控制路径里剩余 `windows.h` 依赖按能力拆分为平台服务接口；
- 为 macOS/Linux 输入桥准备同构的 `ScreenPoint` 注入点，先打通最小事件闭环。
