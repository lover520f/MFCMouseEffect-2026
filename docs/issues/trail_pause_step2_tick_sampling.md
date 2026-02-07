# Trail Pause Step 2（拖尾入点改为 Tick 采样）

## 目标
- 避免拖尾更新完全依赖 `WM_MFX_MOVE` 处理节奏。
- 当 click/hover/hold 让消息处理抖动时，拖尾仍能按窗口 tick 持续更新。

## 方案
- `TrailWindow`：
  - `AddPoint` 只更新最新鼠标位置。
  - `OnTick` 每帧采样（优先最新位置，否则 `GetCursorPos`）并写入点列。
- `ParticleTrailWindow`：
  - `ParticleTrailEffect::OnMouseMove` 改为只上报位置。
  - `OnTick` 根据当前/上次位置位移自动计算发射量并 `Emit`。

## 影响文件
- `MFCMouseEffect/MouseFx/Windows/TrailWindow.h`
- `MFCMouseEffect/MouseFx/Windows/TrailWindow.cpp`
- `MFCMouseEffect/MouseFx/Windows/ParticleTrailWindow.h`
- `MFCMouseEffect/MouseFx/Windows/ParticleTrailWindow.cpp`
- `MFCMouseEffect/MouseFx/Effects/ParticleTrailEffect.cpp`

## 预期
- click/hover/hold 叠加时，trail 仍连续，明显降低“暂停”体感。
