# MFC 壳解耦阶段 18：Cursor Position Service 平台化（Win32 首落地）

## 1. 背景与目标

阶段 17 已把控制与自动化链路统一为 `ScreenPoint`，但 `AppController` 和 `DispatchRouter` 仍直接调用 `GetCursorPos`。这会让 Core 控制路径继续绑定 Win32 API。

本阶段目标：

- 抽象光标位置查询服务到 Core 接口；
- 在 `Platform/windows` 提供 Win32 实现；
- `AppController/DispatchRouter` 改为依赖服务抽象，不直接触达 `GetCursorPos`。

## 2. 判定

判定：`架构演进`（非功能 Bug 修复）。

依据：

- 可见交互行为保持一致；
- 变更为平台能力下沉与依赖方向优化。

## 3. 实施内容

### 3.1 Core 新增光标服务抽象与空实现

新增：

- `MFCMouseEffect/MouseFx/Core/System/ICursorPositionService.h`
- `MFCMouseEffect/MouseFx/Core/System/NullCursorPositionService.h`

接口能力：

- `TryGetCursorScreenPoint(ScreenPoint* outPt)`

### 3.2 平台工厂扩展

修改：

- `MFCMouseEffect/Platform/PlatformInputServicesFactory.h`
- `MFCMouseEffect/Platform/PlatformInputServicesFactory.cpp`

新增工厂：

- `CreateCursorPositionService()`

策略：

- Windows 返回 `Win32CursorPositionService`；
- 非 Windows 返回 `NullCursorPositionService`。

### 3.3 Win32 光标服务实现

新增：

- `MFCMouseEffect/Platform/windows/System/Win32CursorPositionService.h`
- `MFCMouseEffect/Platform/windows/System/Win32CursorPositionService.cpp`

实现：

- 使用 Win32 `GetCursorPos` 获取光标并转换为 `ScreenPoint`。

### 3.4 AppController / DispatchRouter 接入抽象

修改：

- `MFCMouseEffect/MouseFx/Core/Control/AppController.h`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/DispatchRouter.cpp`

关键变更：

- `AppController` 增加 `cursorPositionService_` 依赖与 `QueryCursorScreenPoint(...)` 封装；
- `OnDispatchActivity`（hover end）与 `TryEnterHover` 统一通过该服务取光标；
- `DispatchRouter` 在 scroll/button/自测路径改为调用 `AppController::QueryCursorScreenPoint`，移除 direct `GetCursorPos`。

### 3.5 工程文件同步

修改：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

新增编译项：

- `Platform\windows\System\Win32CursorPositionService.cpp`

新增头文件项：

- `ICursorPositionService`
- `NullCursorPositionService`
- `Win32CursorPositionService`

## 4. 验证

构建验证（VS 2026 MSBuild）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

## 5. 风险与边界

已控制：

- 光标读取来源未变，仅替换调用路径为服务抽象；
- 失败兜底仍保持原逻辑（坐标回退到消息点位或零点）。

当前边界：

- 控制路径仍运行在 Win32 消息宿主上；
- 时间源（`GetTickCount64`）仍是 Win32 直接调用，尚未抽象。

## 6. 后续建议（阶段 19）

- 抽象事件时间服务（tick/monotonic clock）并下沉到平台层；
- 继续收敛 `AppController` 中剩余 Win32 API 直接依赖；
- 为 macOS/Linux 补齐对应 cursor/time 服务实现，先保证控制链路接口闭环。
