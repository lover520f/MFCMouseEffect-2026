# GPU 命令流抽象（Stage 34）

## 目标
- 为拖尾、长按、悬浮建立统一的 GPU 命令数据接口。
- 保持现有 CPU/GDI+ 渲染不变，只新增“可被 GPU 消费的帧数据”。
- 让后续 Dawn 渲染接入时，不再直接耦合各特效内部状态结构。

## 新增
- `MFCMouseEffect/MouseFx/Gpu/OverlayGpuCommandStream.h`
  - `OverlayGpuCommandType`
  - `OverlayGpuVertex`
  - `OverlayGpuCommand`
  - `OverlayGpuCommandStream`

## 接口扩展
- `MFCMouseEffect/MouseFx/Interfaces/IOverlayLayer.h`
  - 新增 `AppendGpuCommands(gpu::OverlayGpuCommandStream&, uint64_t)` 默认空实现。
  - 保证旧图层不改也可编译运行。

## 已接入图层
1. 拖尾
- 文件：`MFCMouseEffect/MouseFx/Layers/TrailOverlayLayer.h/.cpp`
- 输出 `TrailPolyline` 命令，包含轨迹点、生命周期衰减、基础颜色。

2. 长按/悬浮（Ripple 管线）
- 文件：`MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.h/.cpp`
- 输出 `RipplePulse` 命令，包含中心点、半径/进度、持续/非持续标记、样式颜色。

3. 粒子拖尾
- 文件：`MFCMouseEffect/MouseFx/Layers/ParticleTrailOverlayLayer.h/.cpp`
- 输出 `ParticleSprites` 命令，包含粒子位置、尺寸、生命值和颜色。

## 命令汇总入口
- 文件：`MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.h/.cpp`
- 在 `OnTick()` 中新增 `CollectGpuCommandStream(nowMs)`：
  - 每帧先让所有存活图层输出 GPU 命令；
  - 仍走原 CPU 渲染路径，行为不变。

## 状态对外
- 文件：`MFCMouseEffect/MouseFx/Core/OverlayHostService.h/.cpp`
  - 新增命令流统计查询接口（总命令数 + 各类型数量）。
- 文件：`MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
  - `/api/state` 新增 `gpu_command_stream` 字段。
- 文件：`MFCMouseEffect/WebUI/app.js`
  - 在 `?diag=1` 模式下，GPU 横幅附加显示命令流统计，便于验证抽象层是否有数据产出。

## 说明
- 本阶段不切换渲染后端，不引入视觉变化。
- 该抽象的作用是为 Stage 35+ 的 Dawn 消费层做准备，降低特效实现与 GPU 线程/后端细节耦合。
