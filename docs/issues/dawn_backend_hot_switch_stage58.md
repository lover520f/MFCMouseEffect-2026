# Dawn 切换去重建化（Stage 58）

## 问题

在 `cpu <-> dawn`、`auto -> dawn` 切换时，旧路径会走：

- `OverlayHostService::SetRenderBackendPreference(...)` 内部 `Shutdown()`
- `AppController` 侧 `RecreateActiveEffects()` 全量重建

这条路径会触发多层窗口销毁/重建与特效对象重建，容易在切换瞬间造成明显卡顿。

## 调整

将后端偏好切换改为“在现有 Host 上热切提交上下文”，不再默认销毁 Host：

1. `SetRenderBackendPreference(...)` 不再调用 `Shutdown()`。
2. `cpu` 目标：
- 直接更新 `activeBackend_=cpu`、`pipelineMode_=cpu_layered`。
- 若 Host 已存在，调用 `host_->SetGpuSubmitContext(...)`。
3. `dawn/auto` 目标：
- 仍异步探测 Dawn runtime。
- queue ready 时直接更新 `activeBackend_=dawn` 与 pipeline mode。
- 若 Host 已存在，同步更新 submit context。
4. 保留 `auto<->dawn`（已在 Dawn）偏好等价切换不重建。

## 影响

- 后端切换由“销毁+重建”改为“上下文热切”，显著降低切换峰值卡顿。
- 特效对象与 OverlayHostWindow 生命周期更稳定。
- CPU fallback 与 Dawn 探测逻辑不变。

## 变更文件

- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
