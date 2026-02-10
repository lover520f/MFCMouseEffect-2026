# Dawn 启动卡顿进一步收紧（Stage 52）

## 现象

虽然 Stage 50/51 已明显缓解，但启动后仍有一次短暂卡顿。

## 根因补充

- 启动早期存在“桥接模式更新触发 probe”的路径。
- 即使当前后端目标是 `cpu`，仍可能提前触发 Dawn 探测，导致驱动初始化在启动窗口期发生。

## 本次收紧

### 1) CPU 后端下禁止触发异步 probe

在 `SetRenderBackendPreference()` / `SetGpuBridgeModeRequest()` 中增加条件：
- 仅当请求后端为 `dawn/auto` 时才触发 `RefreshGpuRuntimeProbeAsync()`。
- `cpu` 路径只切换状态，不触发 Dawn 探测。

### 2) 启动顺序调整

`AppController::Start()` 中改为：
1. 先 `SetRenderBackendPreference("cpu")`
2. 再 `SetGpuBridgeModeRequest(...)`

这样可以保证启动初期不会因为桥接模式设置触发 Dawn 探测。

## 预期效果

- 启动阶段进一步平滑，减少“刚启动就卡一下”的概率。
- 仍保留后续异步升级到 Dawn 的能力（队列就绪后升级）。
