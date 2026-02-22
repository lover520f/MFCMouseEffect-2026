# MFC 壳解耦阶段 24：OverlayCoordSpace Core 接口去 Win32 类型化

## 1. 背景与目标

阶段 23 后，`OverlayCoordSpace` 仍在 Core 头文件暴露 `HWND/POINT`，导致：

- Core 头文件直接依赖 `<windows.h>`；
- 多个 Layer/Renderer 通过 `POINT` 走坐标换算，平台边界不清晰；
- `AppController` 侧已收敛类型，但渲染链路仍有 Win32 暴露点。

本阶段目标：

- 将 `OverlayCoordSpace` 公共 API 收敛为平台无关类型；
- 保持 Win32 行为不变（坐标结果与渲染位置不变）；
- 同步清理调用点，避免引入兼容分支。

## 2. 判定

判定：`架构演进`（非功能回归修复）。

依据：

- 坐标原点计算仍由 `IOverlayCoordSpaceService`/Win32 实现决定；
- 本次仅替换接口表面类型，不改变时序与算法。

## 3. 实施内容

### 3.1 Core API 去 Win32 类型暴露

修改：

- `MFCMouseEffect/MouseFx/Core/Overlay/OverlayCoordSpace.h`
- `MFCMouseEffect/MouseFx/Core/Overlay/OverlayCoordSpace.cpp`

关键调整：

- `SetOverlayWindowHandle(HWND)` -> `SetOverlayWindowHandle(uintptr_t)`
- `GetOverlayOrigin()` 返回 `ScreenPoint`
- `ScreenToOverlayPoint(const ScreenPoint&)` 返回 `ScreenPoint`
- 移除 `OverlayCoordSpace.h` 对 `<windows.h>` 的直接依赖。

### 3.2 Layer 接口同步收敛

修改：

- `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.h`
- `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.cpp`
- `MFCMouseEffect/MouseFx/Layers/TextOverlayLayer.h`
- `MFCMouseEffect/MouseFx/Layers/TextOverlayLayer.cpp`
- `MFCMouseEffect/MouseFx/Core/Overlay/OverlayHostService.cpp`

关键调整：

- `RippleOverlayLayer::UpdatePosition` 改为 `ScreenPoint`；
- `RippleOverlayLayer::ResolveRenderCenter` 改为 `ScreenPoint`；
- `TextOverlayLayer::ShowText`/内部 `startPt` 改为 `ScreenPoint`；
- `OverlayHostService` 直接传递 `ScreenPoint` 给 `Ripple/Text` 层，减少 Win32 往返转换。

### 3.3 渲染器调用点统一到 ScreenPoint

修改：

- `MFCMouseEffect/MouseFx/Layers/ParticleTrailOverlayLayer.cpp`
- `MFCMouseEffect/MouseFx/Renderers/Hold/HoldQuantumHaloGpuV2D2DBackend.cpp`
- `MFCMouseEffect/MouseFx/Renderers/Trail/LineTrailRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Trail/ElectricTrailRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Trail/StreamerTrailRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Trail/MeteorRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Trail/TubesRenderer.h`

关键调整：

- `ScreenToOverlayPoint(...)` 统一使用 `ScreenPoint` 入参与返回值；
- 原 `POINT` 坐标在调用点就地转为 `ScreenPoint`，不新增兼容分支。

### 3.4 Win32 Overlay Host 适配

修改：

- `MFCMouseEffect/Platform/windows/Overlay/Win32OverlayHostWindow.cpp`

关键调整：

- `SetOverlayWindowHandle(timerHwnd_)` 改为传入 `uintptr_t`。

## 4. 验证

构建验证（VS 2026 MSBuild amd64）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

说明：

- 一次 `Release` 链接阶段出现 `LNK1104`（目标 exe 被占用），结束占用进程后复跑通过，属于构建环境占用问题，非代码回归。

## 5. 风险与边界

已控制：

- 坐标换算服务实现未变，窗口原点逻辑未变；
- 仅接口表面去 Win32 类型，渲染行为保持一致。

当前边界：

- `ITrailRenderer` 与部分老窗口路径仍保留 `POINT`（历史结构），尚未完全平台无关。

## 6. 下一步建议（阶段 25）

- 平台化 `ITrailRenderer/TrailPoint`（`POINT` -> `ScreenPoint`）；
- 清理 `TrailEffect/ParticleTrailEffect` 中 host 层的 Win32 转换；
- 逐步将 legacy `MouseFx/Windows/*Window` 仅保留为 Windows fallback，并在 Core 路径中移除 Win32 头依赖。
