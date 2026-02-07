# Trail Pause Step 1（层级稳定化）

## 目标
- 先验证“拖尾暂停”是否由窗口层级被其他特效窗口压住导致。
- 本步骤只改置顶恢复，不改渲染算法和发射算法。

## 变更
- 为 `TrailWindow` 增加：
  - foreground WinEvent hook（`EVENT_SYSTEM_FOREGROUND`）
  - `kMsgEnsureTopmost` 消息回调
  - `EnsureTopmostZOrder(force)` 低频兜底（2.5s）
- 为 `ParticleTrailWindow` 增加同样机制。

## 影响文件
- `MFCMouseEffect/MouseFx/Windows/TrailWindow.h`
- `MFCMouseEffect/MouseFx/Windows/TrailWindow.cpp`
- `MFCMouseEffect/MouseFx/Windows/ParticleTrailWindow.h`
- `MFCMouseEffect/MouseFx/Windows/ParticleTrailWindow.cpp`

## 预期
- 新顶层窗口出现、或 click/hover/hold 触发后，trail 窗口会主动恢复到 topmost。
- 若问题属于“被遮挡”，此步骤应明显改善暂停感。
