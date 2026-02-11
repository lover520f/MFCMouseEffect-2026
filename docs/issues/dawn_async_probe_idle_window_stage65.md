# Dawn 异步探测输入空闲窗口（Stage 65）

## 目的

缓解启动初期与切换到 Dawn 时的瞬时卡顿感。  
保持现有架构不变，仅收紧异步探测触发时机。

## 调整

1. 在 `OverlayHostService` 新增短空闲窗口等待：
- 读取系统最后输入时间（`GetLastInputInfo`）
- 当连续输入活跃时，异步线程先等待短暂空闲再执行 Dawn 握手

参数：
- 空闲阈值：`140ms`
- 最大等待：`1800ms`
- 轮询间隔：`40ms`

2. 应用位置：
- `RefreshGpuRuntimeProbeAsync()` 的后台线程里，在 `TryInitializeDawnRuntime()` 之前执行等待逻辑。

## 影响

- 启动和 `apply` 切换到 Dawn 时，减少“握手与高频输入竞争”带来的主观卡顿。
- 不影响功能正确性：最长等待超时后仍会继续执行探测，避免无限推迟。

## 变更文件

- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
