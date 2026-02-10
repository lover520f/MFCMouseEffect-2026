# Dawn 探测去抖与单线程合并（Stage 53）

## 现象

日志中可见 Dawn 运行时模块反复装载/卸载，且驱动初始化异常频繁，导致切换/启动阶段仍有短时卡顿。

## 根因

- 异步 probe 会被多处触发（后端切换、桥接切换、初始化回退路径）。
- 旧实现每次都 `ResetDawnRuntimeProbe()`，会触发 `FreeLibrary(webgpu_dawn.dll)` + 再加载，导致抖动。
- 多次触发时会创建多个后台探测线程，形成探测风暴。

## 本次优化

### 1) 异步 probe 单线程合并

- 增加 `asyncProbeRunning_` 标记，只允许一个后台 probe 线程运行。
- 新请求只递增 token，不再额外开线程。
- 运行中的线程会在结束前对比 token，若存在新请求则继续合并处理。

### 2) 异步 probe 去掉强制 reset

- `RefreshGpuRuntimeProbeAsync()` 不再调用 `ResetDawnRuntimeProbe()`。
- 避免每次异步探测都做 DLL 卸载/重载。
- 同步 `RefreshGpuRuntimeProbe()` 保持原行为，用于显式刷新场景。

### 3) 后台优先级下调

- 异步 probe 线程设置为 `THREAD_PRIORITY_BELOW_NORMAL`，减小对输入线程的争抢。

## 预期

- 启动/Apply 时卡顿进一步降低。
- 诊断日志中 Dawn 相关初始化波动减少，submit/cmdbuf 更稳定。
