# Dawn 符号诊断缓存（Stage 62）

## 目的

继续推进 Dawn 接入时，降低 `diag` 页面高频轮询对 runtime 符号探测的重复开销。

## 调整

1. 为 `GetDawnRuntimeSymbolStatus()` 增加缓存：
- 缓存窗口：`2000ms`
- 同时绑定 `probe.generation`，探测代次变化时自动失效
2. 在 `ResetDawnRuntimeProbe()` 中显式清空符号缓存。

## 影响

- 诊断模式下 `symbol-check` 查询不再每次都做完整导出符号扫描。
- 减少高频诊断开销，便于后续继续推进 Dawn 渲染主线。

## 变更文件

- `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
