# MouseFx 文件夹结构整理（2026-01-30）

## 目标
`MFCMouseEffect/MouseFx` 原先文件数量较多、集中在同一层，阅读和定位成本高。
本次在不改变行为的前提下，把 MouseFx 按“职责”进行子目录归类，降低目录噪声、提升可发现性。

## 新结构
### `MFCMouseEffect/MouseFx/Core/`
生命周期/配置/IPC/钩子等“系统胶水层”。
- `AppController.*`
- `ConfigPathResolver.*`
- `EffectConfig.*`
- `EffectFactory.*`
- `GdiPlusSession.h`
- `GlobalMouseHook.*`
- `IpcController.*`
- `JsonLite.*`
- `MouseFxMessages.h`

### `MFCMouseEffect/MouseFx/Effects/`
按分类触发的 effect（业务入口），只负责把事件映射到窗口/渲染器等组件。
- `RippleEffect.*`、`TrailEffect.*`、`ParticleTrailEffect.*`
- `ScrollEffect.*`、`HoldEffect.*`、`HoverEffect.*`
- `IconEffect.*`、`TextEffect.*`

### `MFCMouseEffect/MouseFx/Windows/`
分层窗口与对象池（渲染承载窗口/帧循环等）。
- `RippleWindow*`、`TrailWindow*`、`ParticleTrailWindow*`、`TextWindow*`

### `MFCMouseEffect/MouseFx/Styles/`
主题与样式参数（配色/时长/半径等）。
- `ThemeStyle.*`
- `RippleStyle.h`

### `MFCMouseEffect/MouseFx/Interfaces/`
跨层共享的接口/策略定义，供 Effects/Windows/Renderers 复用。
- `IMouseEffect.h`
- `IRippleRenderer.h`、`ITrailRenderer.h`
- `TrailRenderStrategies.h`

### `MFCMouseEffect/MouseFx/ThirdParty/`
第三方单文件库（避免和业务代码混放）。
- `json.hpp`

### `MFCMouseEffect/MouseFx/Legacy/`
历史/备用目录（当前不作为主路径依赖）。旧的 `RenderStrategies/StandardRenderers` 已删除，可从 git 历史回溯。

### `MFCMouseEffect/MouseFx/Renderers/`
渲染器实现（保持原目录结构）。

## 约定
- 跨目录引用尽量使用 `MouseFx/...` 的工程根路径 include，避免“相对路径漂移”。
- 仅做归档与 include/vcxproj 同步，不改变运行逻辑。
