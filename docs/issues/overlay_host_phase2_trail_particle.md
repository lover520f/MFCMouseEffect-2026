# OverlayHost Phase 2: Trail + Particle 迁移

## 1. 本阶段目标

在不改动 click/scroll/hold/hover/text 的前提下，先把 `trail` 迁移到 `OverlayHostWindow`：

- `TrailEffect`（line/streamer/electric/meteor/tubes）
- `ParticleTrailEffect`（particle）

目标是先验证“单宿主窗口 + 单帧循环”对拖尾暂停与层级稳定性的收益。

## 2. 代码改动点

### 2.1 新增基础设施

- `MouseFx/Interfaces/IOverlayLayer.h`
- `MouseFx/Windows/OverlayHostWindow.h/.cpp`
- `MouseFx/Core/OverlayHostService.h/.cpp`

能力：

- Host 内支持 `AddLayer/RemoveLayer`
- 仅在有图层时启动 16ms Tick
- 前台窗口切换后自动 `EnsureTopmost`（WinEvent hook + 低频重置）

### 2.2 新增 Trail 图层

- `MouseFx/Layers/TrailOverlayLayer.h/.cpp`
- `MouseFx/Layers/ParticleTrailOverlayLayer.h/.cpp`

要点：

- Trail 图层复用原 `ITrailRenderer`
- 保留 tick 采样鼠标逻辑（无 move 消息时回退 `GetCursorPos`）
- Particle 图层在 Host Tick 内完成发射与衰减

### 2.3 接入效果层

- `MouseFx/Effects/TrailEffect.*`
- `MouseFx/Effects/ParticleTrailEffect.*`

策略：

- 优先接入 `OverlayHostService`
- 若 Host 初始化失败，回退旧 `TrailWindow` / `ParticleTrailWindow` 路径

## 3. 风险与回滚

风险：

- Host 图层与旧窗口路径并存时，需确保不会重复渲染
- Trail renderer 依赖虚拟屏坐标，迁移后需保持坐标系一致

回滚：

- 仅撤回 `TrailEffect/ParticleTrailEffect` 对 Host 的接入即可恢复旧行为
- 其他效果完全未接入，不受本阶段影响

## 4. 手工验收清单

1. 仅开 trail（line/particle）时流畅且无暂停
2. trail 运动中触发 click/hover/hold，拖尾不冻结
3. 打开新顶层窗口后，trail 仍保持可见且在最上层
4. `reload_config` 后 trail 仍正常
