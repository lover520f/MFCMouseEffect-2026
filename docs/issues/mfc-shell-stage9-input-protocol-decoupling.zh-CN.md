# MFC 壳解耦阶段 9：输入协议去 Hook 绑定与 Win32 类型下沉（第一步）

## 1. 背景与目标

当前 `MouseFx/Interfaces` 和部分 `Core` 头文件仍直接依赖 `GlobalMouseHook.h` 或 `windows.h`，导致公共层与平台实现耦合过高，不利于后续 macOS/Linux 复用。

本阶段目标（第一步）：

- 抽离公共输入协议类型（点位、点击、按键）到独立协议头；
- 让 `IMouseEffect` / `IHoldRuntime` 基于公共协议类型，不直接依赖 Win32 类型；
- 保持 Windows 行为不变，通过边界转换适配现有渲染/窗口代码。

## 2. 判定

判定：`架构演进`（非功能回归修复）。

依据：

- 用户可见行为不变；
- 仅调整类型边界与依赖方向，降低平台耦合。

## 3. 实施内容

### 3.1 新增公共输入协议类型

新增：

- `MFCMouseEffect/MouseFx/Core/Protocol/InputTypes.h`

核心类型：

- `ScreenPoint`
- `MouseButton`
- `ClickEvent`
- `KeyEvent`

说明：以上类型不再从 `GlobalMouseHook` 定义，改为公共协议层单独维护。

### 3.2 新增 Win32 点位桥接

新增：

- `MFCMouseEffect/MouseFx/Core/Protocol/InputTypesWin32.h`

提供：

- `ToScreenPoint(const POINT&)`
- `ToNativePoint(const ScreenPoint&)`

用于在“公共协议类型”和“Win32 原生类型”之间显式转换。

### 3.3 接口层去 Win32 依赖（第一批）

修改：

- `MFCMouseEffect/MouseFx/Interfaces/IMouseEffect.h`
  - `OnMouseMove/OnHoverStart/OnHoldStart/OnHoldUpdate` 改为 `ScreenPoint` / `uint32_t`
  - `ScrollEvent/EdgeEvent` 的点位改为 `ScreenPoint`
- `MFCMouseEffect/MouseFx/Interfaces/IHoldRuntime.h`
  - `Start/Update` 入参改为 `ScreenPoint`
- `MFCMouseEffect/MouseFx/Interfaces/EffectCommands.h`
  - 命令枚举底层类型改为 `uint32_t`
- `MFCMouseEffect/MouseFx/Interfaces/EffectMetadata.h`
  - `trayCmd` 类型改为 `uint32_t`

### 3.4 GlobalMouseHook 与事件消费方解耦

修改：

- `MFCMouseEffect/MouseFx/Core/System/GlobalMouseHook.h/.cpp`
  - 删除本地事件类型定义，改为复用 `InputTypes.h`
  - 在 Hook 边界处做 `POINT <-> ScreenPoint` 转换

### 3.5 关键调用链适配

修改：

- `MFCMouseEffect/MouseFx/Core/Control/DispatchRouter.cpp`
- `MFCMouseEffect/MouseFx/Core/Overlay/InputIndicatorOverlay.*`
- `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.*`
- `MFCMouseEffect/MouseFx/Effects/*`（Hold/Hover/Trail/ParticleTrail/Text/RippleBasedHoldRuntime/HoldQuantumHaloGpuV2DirectRuntime）

处理方式：

- 对外使用 `ScreenPoint`；
- 进入 Win32/GDI/窗口层时通过桥接函数转换到 `POINT`。

## 4. 验证

构建验证：

- `Release|x64`：通过（全量 Rebuild）
- `Debug|x64`：通过

结果：

- 0 warning
- 0 error

## 5. 风险与边界

已控制：

- 仅做类型与依赖边界迁移，渲染/交互行为保持一致；
- 通过 Win32 桥接头集中管理点位转换，避免隐式转换扩散。

当前边界：

- `MouseFx/Core` 仍存在较多 `windows.h` 依赖（窗口、消息、hook、渲染后端）；
- 本阶段未把这些模块整体迁移到 `Platform/windows`，只先清理“公共输入协议”这一条链路。

## 6. 后续建议（阶段 10）

- 继续拆分 `Core` 中 Win32 专属模块（Hook/Window/Overlay Host）到 `Platform/windows`；
- 为 `ScreenPoint` 增加平台无关坐标空间辅助工具（缩放、clamp、monitor transform）；
- 在 macOS/Linux 实现中复用相同输入协议类型，避免平台层再反向污染接口层。
