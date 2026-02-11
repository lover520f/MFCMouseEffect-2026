# Dawn 启动/切换初期卡顿收敛（Stage 59）

## 现象

- 配置为 `dawn` 启动时，前几秒仍会走 `backend=cpu`，并出现明显卡顿。
- `auto -> dawn`/`cpu -> dawn` 切换时，会出现一段“先卡后恢复”。

## 根因

1. 启动路径里对非 CPU 配置仍强制写回一次 `cpu`，导致后端目标被改写，Dawn 就绪后不能立即升为 active backend。
2. `SetRenderBackendPreference(dawn/auto)` 在发起异步探测后立刻读取 runtime 状态，可能与后台探测锁竞争，放大主线程卡顿。
3. 当 `requestedBackend` 未变化时直接返回，错过“queue 已就绪后把 active 从 cpu 升级到 dawn”的时机。

## 调整

1. 启动阶段不再对非 CPU 配置强制写回 `cpu`，仅保留目标后端并按状态异步升级。
2. `SetRenderBackendPreference` 增加“非阻塞激活”逻辑：
- 后台探测运行中不做阻塞读取；
- 探测结束后可在同一请求值下把 `activeBackend` 从 `cpu` 提升到 `dawn`。
3. 同请求值下不再盲目早退，允许执行“就绪后提级”。
4. Dawn 队列就绪后的延迟升级路径不再 `RecreateActiveEffects()`，只做后端提级，避免切换瞬间重建抖动。

## 预期效果

- 启动默认 `dawn` 时，减少“长时间停留 CPU 再切换 Dawn”的窗口。
- `auto/dawn` 切换不再因为 runtime 锁竞争放大鼠标卡顿。
- backend 目标与 active 状态更一致，避免日志长期 `prime=queue_ready` 但 `backend=cpu`。

## 变更文件

- `MFCMouseEffect/MouseFx/Core/AppController.cpp`
- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
