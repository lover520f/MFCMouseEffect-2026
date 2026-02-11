# Dawn 首次提交路径后台预热（Stage 69）

## 背景

用户反馈：当前卡顿并非特定特效，而是 Dawn 后端首次切入时的整体卡顿。`apply` 切换已改善，但首次真实交互仍可能触发底层首次提交开销。

## 调整

在 `OverlayHostService::RefreshGpuRuntimeProbeAsync()` 中：

1. 异步探测在 queue ready 后追加“一次性提交预热”
- `TrySubmitNoopQueueWork`
- `TrySubmitEmptyCommandBufferTagged("startup_warmup")`
- `TrySubmitRippleBakedPacket(4, 96)`（微型包，仅用于打通路径）

2. 预热只做一次
- 通过进程内原子状态 `g_dawnStartupWarmupState` 控制（0/1/2）。

## 目的

把 Dawn 首次命令提交/分支路径首触发的开销前移到后台异步探测阶段，减少首次用户交互时的卡顿峰值。

## 变更文件

- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
