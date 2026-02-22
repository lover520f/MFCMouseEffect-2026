# MFC 壳解耦阶段 14：输入指示器 Overlay 平台化（Win32 首落地）

## 1. 背景与目标

阶段 13 后，`AppController` 仍直接持有 `InputIndicatorOverlay`（位于 Core），该模块本质是 Win32 分层窗口实现，放在 Core 会持续放大平台耦合。

本阶段目标：

- 为输入指示器 Overlay 提供 Core 抽象接口；
- 将 Win32 具体实现迁移到 `Platform/windows/Overlay`；
- 通过平台工厂注入到 `AppController`，清理 Core 对 Win32 类名的直接依赖。

## 2. 判定

判定：`架构演进`（非功能 Bug 修复）。

依据：

- 用户可见行为（点击/滚轮/按键提示）保持一致；
- 变更聚焦接口与实现分层，不改交互语义。

## 3. 实施内容

### 3.1 Core 新增输入指示器抽象与空实现

新增：

- `MFCMouseEffect/MouseFx/Core/Overlay/IInputIndicatorOverlay.h`
- `MFCMouseEffect/MouseFx/Core/Overlay/NullInputIndicatorOverlay.h`

接口能力：

- `Initialize/Shutdown/Hide`
- `UpdateConfig`
- `OnClick/OnScroll/OnKey`

### 3.2 Win32 实现迁移到 Platform

迁移并重命名：

- `MFCMouseEffect/MouseFx/Core/Overlay/InputIndicatorOverlay.h/.cpp`

到：

- `MFCMouseEffect/Platform/windows/Overlay/Win32InputIndicatorOverlay.h/.cpp`

关键点：

- 类名统一为 `Win32InputIndicatorOverlay`；
- 实现 `IInputIndicatorOverlay`；
- 原分层窗口、多显示器 clone、GDI 渲染逻辑保持不变。

### 3.3 平台工厂扩展

修改：

- `MFCMouseEffect/Platform/PlatformOverlayServicesFactory.h/.cpp`

新增工厂：

- `CreateInputIndicatorOverlay()`

策略：

- Windows 返回 `Win32InputIndicatorOverlay`；
- 非 Windows 返回 `NullInputIndicatorOverlay`。

### 3.4 AppController 改为依赖抽象

修改：

- `MFCMouseEffect/MouseFx/Core/Control/AppController.h`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.ConfigUpdates.cpp`

关键变更：

- 成员从值对象改为 `std::unique_ptr<IInputIndicatorOverlay>`；
- 构造时通过 `platform::CreateInputIndicatorOverlay()` 注入；
- 工厂异常时回退 `NullInputIndicatorOverlay`，避免空指针路径；
- 原调用点统一改为指针访问。

### 3.5 工程清单同步

修改：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

变更：

- 移除 `MouseFx\Core\Overlay\InputIndicatorOverlay.*`；
- 新增 `IInputIndicatorOverlay`、`NullInputIndicatorOverlay`、`Win32InputIndicatorOverlay.*`。

## 4. 验证

构建验证（VS 2026 MSBuild）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

## 5. 风险与边界

已控制：

- 业务入口不变，`DispatchRouter -> AppController -> IndicatorOverlay` 调用链保持一致；
- 工厂+Null 兜底避免平台实现未就绪时崩溃。

当前边界：

- `DispatchRouter/AppController` 仍由 Win32 消息窗口驱动；
- 输入捕获到消息桥接尚未完全平台抽象。

## 6. 后续建议（阶段 15）

- 抽象消息分发桥（替换 `HWND` 直连）；
- 为 macOS/Linux 实现对应输入指示器平台后端或保持 Null 策略并在设置层明确能力状态。
