# MFC 壳解耦阶段 10：全局输入 Hook 平台化下沉（Win32）

## 1. 背景与目标

阶段 9 已完成输入协议类型（`ScreenPoint`、`ClickEvent`、`KeyEvent`）与 Win32 Hook 定义解耦。  
但 `GlobalMouseHook` 实现仍位于 `MouseFx/Core/System`，导致核心层继续承载平台细节（`SetWindowsHookExW`、`WH_MOUSE_LL`、`WH_KEYBOARD_LL`）。

本阶段目标：

- 在核心层引入 Hook 抽象接口，避免 `AppController` 直接绑定 Win32 类；
- 将 Win32 Hook 具体实现移动到 `Platform/windows`；
- 保持现有输入行为不变（点击、移动、滚轮、长按、按键）。

## 2. 判定

判定：`架构演进`（非功能回归修复）。

依据：

- 用户可见交互流程不变；
- 仅调整依赖方向和代码组织边界。

## 3. 实施内容

### 3.1 Core 层新增 Hook 抽象

新增：

- `MFCMouseEffect/MouseFx/Core/System/IGlobalMouseHook.h`

接口能力：

- `Start(uintptr_t dispatchHandle)`
- `Stop()`
- `LastError()`
- `ConsumeLatestMove(ScreenPoint&)`
- `SetKeyboardCaptureExclusive(bool)`

说明：接口层只使用跨平台协议类型（`ScreenPoint`），不再暴露 Win32 `POINT`。

### 3.2 增加空实现兜底

新增：

- `MFCMouseEffect/MouseFx/Core/System/NullGlobalMouseHook.h`

作用：

- 作为非 Win32 平台或异常工厂分支的无操作实现；
- 保证上层依赖可以始终拿到对象，降低空指针路径。

### 3.3 平台工厂新增输入服务入口

新增：

- `MFCMouseEffect/Platform/PlatformInputServicesFactory.h`
- `MFCMouseEffect/Platform/PlatformInputServicesFactory.cpp`

作用：

- 统一创建 `IGlobalMouseHook`；
- Windows 返回 `Win32GlobalMouseHook`，其他平台返回 `NullGlobalMouseHook`。

### 3.4 Win32 Hook 实现下沉到 Platform

新增：

- `MFCMouseEffect/Platform/windows/System/Win32GlobalMouseHook.h`
- `MFCMouseEffect/Platform/windows/System/Win32GlobalMouseHook.cpp`

迁移内容：

- 原 `GlobalMouseHook` 全部 Win32 逻辑（低级鼠标/键盘 hook、事件转发、修饰键状态维护）迁入平台目录；
- `ConsumeLatestMove` 输出改为 `ScreenPoint`。

### 3.5 AppController 改为依赖抽象接口

修改：

- `MFCMouseEffect/MouseFx/Core/Control/AppController.h`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.cpp`

关键点：

- 成员从 `GlobalMouseHook` 改为 `std::unique_ptr<IGlobalMouseHook>`；
- 构造时通过 `platform::CreateGlobalMouseHook()` 注入；
- `ConsumeLatestMove` 在边界处执行 `ScreenPoint -> POINT` 转换（`ToNativePoint`）；
- 启动流程改为通过抽象接口取 `LastError`。

### 3.6 清理旧 Core Win32 Hook 文件

删除：

- `MFCMouseEffect/MouseFx/Core/System/GlobalMouseHook.h`
- `MFCMouseEffect/MouseFx/Core/System/GlobalMouseHook.cpp`

### 3.7 工程清单同步

修改：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

变更：

- 移除旧 `GlobalMouseHook` 编译项；
- 增加 `IGlobalMouseHook`、`NullGlobalMouseHook`、`PlatformInputServicesFactory`、`Win32GlobalMouseHook` 新条目。

## 4. 验证

构建验证（VS 2026 `MSBuild`）：

- `Debug|x64`：通过
- `Release|x64`：通过

说明：

- 首次 `Release|x64` 链接阶段出现 `LNK1104`（目标 exe 被占用）；
- 已按协作约定自动结束占用进程后重试，构建通过。

## 5. 风险与边界

已控制：

- 行为逻辑未改，只迁移实现归属和依赖入口；
- `AppController` 仍维持原消息分发机制，功能回归风险低。

当前边界：

- `AppController` 本身仍是 Win32 消息窗口驱动；
- `OverlayHost`、窗口创建、坐标空间等模块尚未迁移到 `Platform/windows`。

## 6. 后续建议（阶段 11）

- 抽象输入事件分发器（替代 `uintptr_t dispatchHandle`）；
- 逐步下沉 `OverlayHostService` 相关 Win32 细节到 `Platform/windows`；
- 为未来 macOS/Linux 输入接入预留统一的“事件桥接层（Input Bridge）”。
