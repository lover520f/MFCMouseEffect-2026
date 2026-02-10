# Dawn 切换卡顿收敛（Stage 51）

## 问题

在设置页点击 `Apply` 从 `cpu -> dawn`（或 `auto -> dawn`）时，UI 线程会出现明显卡顿。

## 根因

- 切换后端时会触发效果重建。
- 原实现中 `OverlayHostService::Initialize()` 在 UI 线程内同步执行 `TryInitializeDawnRuntime()`，其中包含 adapter/device 握手等待，导致主线程阻塞。

## 本次调整

### 1) Initialize 改为非阻塞判定

- `Initialize()` 不再在 UI 线程同步调用 `TryInitializeDawnRuntime()`。
- 改为读取 `DawnRuntimeStatus`：
  - `queueReady=true` 才激活 `dawn`。
  - 未就绪时立即回退 `cpu`，并后台触发 `RefreshGpuRuntimeProbeAsync()`。

### 2) AppController 增加 Dawn 自动升级重试

- 切到 `dawn/auto` 且当前仍为 `cpu` 时，开启轻量定时轮询。
- 轮询到 `IsDawnQueueReady()==true` 后再重建特效，完成从 CPU 到 Dawn 的平滑升级。
- 增加重试上限（50 次 * 200ms），避免无限轮询。

## 结果

- 点击 `Apply` 时主线程不再因为 Dawn 握手而长时间卡住。
- GPU 未就绪期间保持 CPU 可用；就绪后自动切到 Dawn。

## 影响范围

- `MouseFx/Core/OverlayHostService.*`
- `MouseFx/Core/AppController.*`

## 验证建议

1. 设置 `render_backend=dawn`，点击 `Apply`，确认鼠标不会长时间卡死。
2. 观察诊断日志从 CPU 逐步进入 `Q1:E1:M1:N1`，并出现 submit/cmdbuf 递增。
3. 测试 `cpu <-> dawn`、`auto <-> dawn` 往返切换。
