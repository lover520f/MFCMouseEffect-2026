# MFC 壳解耦阶段 25：TrailPoint 与 Trail Host 链路去 Win32 类型化

## 1. 背景与目标

阶段 24 完成 `OverlayCoordSpace` 去 Win32 类型后，Trail 链路仍存在关键残留：

- `ITrailRenderer` 在公共接口中使用 `POINT`；
- `TrailOverlayLayer` host 路径仍以 `POINT` 作为输入；
- `TrailEffect` 在 host 模式仍存在不必要的 `ScreenPoint -> POINT` 往返转换。

本阶段目标：

- 将 `TrailPoint` 统一为平台无关 `ScreenPoint`；
- 收敛 host 渲染链路类型，减少 Win32 泄漏；
- 保留 legacy `TrailWindow`（Windows fallback）能力不变。

## 2. 判定

判定：`架构演进`（非功能回归修复）。

依据：

- 渲染器读取坐标与生命周期逻辑不变；
- 仅替换数据承载类型与调用层的转换位置。

## 3. 实施内容

### 3.1 ITrailRenderer 接口平台无关化

修改：

- `MFCMouseEffect/MouseFx/Interfaces/ITrailRenderer.h`

关键调整：

- 移除 `<windows.h>` 依赖；
- `TrailPoint::pt` 从 `POINT` 改为 `ScreenPoint`。

### 3.2 Host Trail Layer 输入收敛

修改：

- `MFCMouseEffect/MouseFx/Layers/TrailOverlayLayer.h`
- `MFCMouseEffect/MouseFx/Layers/TrailOverlayLayer.cpp`

关键调整：

- `AddPoint(const POINT&)` -> `AddPoint(const ScreenPoint&)`；
- `latestCursorPt_ / lastSamplePt_` 改为 `ScreenPoint`；
- `GetCursorPos` 路径改为就地转换（`POINT -> ScreenPoint`）。

### 3.3 TrailEffect host/fallback 分流优化

修改：

- `MFCMouseEffect/MouseFx/Effects/TrailEffect.cpp`

关键调整：

- host 模式直接传 `ScreenPoint` 到 `TrailOverlayLayer`；
- 仅 fallback 到 `TrailWindow` 时执行 `ToNativePoint`。

### 3.4 Renderer 与 legacy window 适配

修改：

- `MFCMouseEffect/MouseFx/Renderers/Trail/LineTrailRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Trail/ElectricTrailRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Trail/StreamerTrailRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Trail/MeteorRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Trail/TubesRenderer.h`
- `MFCMouseEffect/MouseFx/Windows/TrailWindow.cpp`

关键调整：

- 渲染器直接消费 `TrailPoint::pt`（`ScreenPoint`）；
- `TubesRenderer` 的内部目标点缓存改为 `ScreenPoint`；
- `TrailWindow` 采样点入队时做 `POINT -> ScreenPoint` 转换，fallback 行为保持一致。

## 4. 验证

构建验证（VS 2026 MSBuild amd64）：

- `Debug|x64`：通过
- `Release|x64`：通过

结果：

- 0 warning
- 0 error

## 5. 风险与边界

已控制：

- trail 生命周期、透明度、头尾宽度与特效算法未改；
- fallback `TrailWindow` 仍使用 Win32 取鼠标位置，仅在入队层转换数据类型。

当前边界：

- `TrailWindow` 本身仍是 Win32 组件；
- `ParticleTrailWindow`/其他 legacy window 尚未统一到 `ScreenPoint`。

## 6. 下一步建议（阶段 26）

- 继续平台化 legacy `MouseFx/Windows/*Window` 输入路径；
- 评估 `GetCursorPos` 读取抽象到 `ICursorPositionService`，避免 Layer 直接调用 Win32 API；
- 梳理 `InputTypesWin32.h` 使用点，逐步下沉到纯平台目录。
