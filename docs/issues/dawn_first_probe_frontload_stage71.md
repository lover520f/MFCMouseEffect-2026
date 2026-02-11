# Dawn 首轮探测前置（Stage 71）

## 问题

当前策略把 Dawn 初始化/预热延后到启动后几秒，导致用户感知为“启动后一段时间突然卡顿”。

## 调整

在 `OverlayHostService::RefreshGpuRuntimeProbeAsync()` 中引入“首轮探测前置”策略：

1. 首轮探测（进程内首次）
- 不等待短空闲窗口，直接执行 `TryInitializeDawnRuntime()`。
- 预热也立即执行（不等待长空闲窗口）。

2. 后续探测
- 继续使用短空闲窗口等待后再握手。
- 预热继续要求长空闲窗口。

3. 状态控制
- `g_dawnFirstProbePending`：确保“首轮前置”仅触发一次。
- 复用 `g_dawnStartupWarmupState`：预热一次性执行。

## 目的

将卡顿从“启动后几秒突发”前移到“首轮启动阶段”，避免运行中突然抖动。

## 变更文件

- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
