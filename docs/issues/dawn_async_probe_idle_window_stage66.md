# Dawn 异步探测进一步收紧（Stage 66）

## 背景

Stage 65 已把 Dawn 握手放到异步线程，并在短空闲窗口后执行。实际体验中仍有“启动后/切换到 Dawn 初期轻微卡顿”。

## 本次收紧

1. 提高“输入静默窗口”门槛，减少在用户仍高频移动鼠标时触发握手。
- 空闲阈值：`320ms`
- 最大等待：`4200ms`
- 轮询间隔：`50ms`
- 稳定采样：连续 `3` 次满足阈值才执行

2. 避免重复握手。
- 在异步探测中先读取 `DawnRuntimeStatus`。
- 若 `queueReady=true`，直接跳过 `TryInitializeDawnRuntime()`。

## 预期效果

- 启动与切换到 Dawn 时，握手更偏向“用户短暂停止输入”时执行。
- 减少重复探测带来的额外抖动和竞争。

## 变更文件

- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
