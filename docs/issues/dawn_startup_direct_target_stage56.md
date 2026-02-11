# Dawn 启动路径收紧（Stage 56）

## 背景

启动期仍有卡顿反馈，日志显示：

- `backend=cpu`
- `prime=queue_ready`
- 之后才切到 Dawn 或长期停留在 CPU

这表明“先 CPU 启动再切 Dawn”的路径仍可能产生双重初始化开销。

## 本次调整

在 `AppController::Start()` 做启动分流：

1. `render_backend=cpu`
- 维持 CPU 启动。
- 不再进入 Dawn 启动分流。

2. `render_backend=dawn/auto`
- 先尝试 `SetRenderBackendPreference(config_.renderBackend)`。
- 若返回 `true`（队列已就绪），直接以目标后端启动（避免 CPU->Dawn 二次重建）。
- 若返回 `false`，退回 CPU 启动并保留 deferred timer，等队列 ready 再切换。

## 预期收益

- Dawn 可直接 ready 的机器上，启动阶段减少一次全量 `RecreateActiveEffects()`。
- 启动卡顿峰值降低，首屏输入更平滑。
- 不改变 CPU 兜底策略。

## 变更文件

- `MFCMouseEffect/MouseFx/Core/AppController.cpp`
