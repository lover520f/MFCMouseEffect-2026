# Dawn 命令语义细分（Stage 63）

## 目的

为后续 Dawn 全量特效接入先打好命令语义基础，让命令流可以明确区分 `ripple / hover / hold`，并避免不同类型共用同一 `flags` 位导致歧义。

## 调整

1. 统一扩展 `OverlayGpuCommandFlags`：
- 新增 `kTrailChromatic`
- 新增 `kParticleChromatic`
2. `RippleOverlayLayer` 的 continuous 命令改为明确语义：
- `hover_continuous`
- `hold_continuous`
- 非 continuous 维持 `ripple`
3. `RippleOverlayLayer` 写入更明确的 flags：
- continuous: `kContinuous`
- hover continuous: `kHoverContinuous | kLoop`
- hold continuous: `kHoldContinuous`
4. `DawnCommandConsumer` 增加 ripple 子类型统计：
- `rippleClickCommandCount`
- `rippleHoverCommandCount`
- `rippleHoldCommandCount`
5. `DawnCommandConsumer` 的 detail 对 non-trail ripple 增加类型后缀：
- `_ripple_click`
- `_ripple_hover`
- `_ripple_hold`
- `_ripple_mixed`

## 影响

- 诊断日志可以直接看出 non-trail 提交是 click/hover/hold 哪种 ripple。
- 为下一阶段“按特效分流到 Dawn 路径”提供稳定语义字段，减少后续分支判断耦合。

## 变更文件

- `MFCMouseEffect/MouseFx/Gpu/OverlayGpuCommandStream.h`
- `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.cpp`
- `MFCMouseEffect/MouseFx/Layers/TrailOverlayLayer.cpp`
- `MFCMouseEffect/MouseFx/Layers/ParticleTrailOverlayLayer.cpp`
- `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
