# MFC 壳解耦阶段 11：Overlay Host 后端平台化（Win32 首落地）

## 1. 背景与目标

阶段 10 已将全局输入 Hook 下沉到 `Platform/windows`，但 `OverlayHostService` 仍直接依赖 `MouseFx/Windows/OverlayHostWindow`，核心层存在平台直连。

本阶段目标：

- 为 Overlay Host 引入平台后端接口；
- 将 Win32 具体后端接入 `Platform/windows`；
- `OverlayHostService` 只负责业务编排，不再直接依赖 Win32 窗口类；
- 同步把 `OverlayHostService` 对外点位参数统一为 `ScreenPoint`，继续收敛 Win32 类型边界。

## 2. 判定

判定：`架构演进`（非功能回归修复）。

依据：

- 可见行为不变（涟漪/文本/轨迹层仍由同一渲染链路处理）；
- 主要变化为依赖方向调整与接口收敛。

## 3. 实施内容

### 3.1 新增 Overlay 后端抽象接口

新增：

- `MFCMouseEffect/MouseFx/Core/Overlay/IOverlayHostBackend.h`

接口能力：

- `Create()`
- `Shutdown()`
- `AddLayer(...)`
- `RemoveLayer(...)`

### 3.2 新增平台 Overlay 工厂

新增：

- `MFCMouseEffect/Platform/PlatformOverlayServicesFactory.h`
- `MFCMouseEffect/Platform/PlatformOverlayServicesFactory.cpp`

策略：

- Windows 返回 `Win32OverlayHostBackend`；
- 其他平台暂返回 `nullptr`（后续阶段补齐 macOS/Linux 后端）。

### 3.3 新增 Win32 Overlay 后端实现

新增：

- `MFCMouseEffect/Platform/windows/Overlay/Win32OverlayHostBackend.h`
- `MFCMouseEffect/Platform/windows/Overlay/Win32OverlayHostBackend.cpp`

实现方式：

- 后端内部组合现有 `OverlayHostWindow`；
- 通过接口对外暴露 layer 生命周期能力。

### 3.4 OverlayHostService 去 Win32 直连

修改：

- `MFCMouseEffect/MouseFx/Core/Overlay/OverlayHostService.h`
- `MFCMouseEffect/MouseFx/Core/Overlay/OverlayHostService.cpp`

关键变更：

- `host_` 替换为 `hostBackend_`（`std::unique_ptr<IOverlayHostBackend>`）；
- `Initialize()` 改为通过 `platform::CreateOverlayHostBackend()` 获取后端；
- 移除对 `OverlayHostWindow` 头文件直接依赖。

### 3.5 OverlayHostService API 点位收敛为 ScreenPoint

修改：

- `UpdateRipplePosition(uint64_t, const ScreenPoint&)`
- `ShowText(const ScreenPoint&, ...)`

边界处理：

- 在 `OverlayHostService.cpp` 内部使用 `ToNativePoint(...)` 转换后再调用现有 Win32 layer/window 实现。

### 3.6 调用链同步

修改：

- `MFCMouseEffect/MouseFx/Effects/RippleBasedHoldRuntime.cpp`
  - `UpdateRipplePosition` 直接传 `ScreenPoint`。
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmClickCommandExecutor.cpp`
  - `spawn_text` 点位改用 `ScreenPoint`。
- `MFCMouseEffect/MouseFx/Effects/TextEffect.cpp`
  - 非 emoji 文本路径通过 `OverlayHostService::ShowText(ScreenPoint, ...)`。

### 3.7 工程文件同步

修改：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

新增编译/头文件条目：

- `IOverlayHostBackend`
- `PlatformOverlayServicesFactory`
- `Win32OverlayHostBackend`

## 4. 验证

构建验证（VS 2026 MSBuild）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

## 5. 风险与边界

已控制：

- 仅改依赖边界与参数类型，不改渲染算法与用户可见效果；
- Win32 实际绘制仍复用 `OverlayHostWindow`，回归面可控。

当前边界：

- `OverlayHostWindow` 实体仍位于 `MouseFx/Windows`（尚未迁目录）；
- 非 Windows 平台后端目前仅预留工厂入口，未落地窗口实现。

## 6. 后续建议（阶段 12）

- 将 `OverlayHostWindow` 物理下沉到 `Platform/windows/Overlay`；
- 为 `OverlayCoordSpace` 引入平台边界接口，减少 `Core/Overlay` 直接依赖 Win32 API；
- 设计 macOS/Linux Overlay 后端最小能力集（窗口层、透明合成、定时刷新）。
