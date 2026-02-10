# Dawn 启动快启（Stage 50）

## 背景

在启用 `dawn/auto` 后端时，应用启动阶段会先执行 GPU 运行时探测与握手，导致 `AppController::Start()` 返回前耗时增加，体感上表现为“启动后特效不是立刻可用”。

## 本次目标

1. 启动后优先保证特效立即可用（CPU 快速起效）。
2. 将 GPU 后端切换延后到启动完成后执行，减少启动阻塞。
3. 保留现有架构：不改 Web 接口语义，不移除同步探测能力。

## 方案

### 1) OverlayHostService 增加异步探测入口

- 新增 `RefreshGpuRuntimeProbeAsync()`。
- `SetRenderBackendPreference()` 与 `SetGpuBridgeModeRequest()` 改为触发异步探测，而不是直接在调用点同步阻塞。
- 仍保留 `RefreshGpuRuntimeProbe()`（同步）用于需要立即探测的路径。

### 2) AppController 启动改为 CPU 快启 + 延后应用目标后端

- 启动加载配置后：先固定使用 `cpu` 作为启动后端，保证特效对象初始化快速完成。
- 当配置目标后端不是 `cpu` 时，记录 `deferredBackendApplyPending_`。
- 在全局钩子成功后，通过一次性定时器 (`kDeferredBackendTimerId`) 延后执行：
  - 恢复用户配置的后端/桥接模式
  - 触发 `RecreateActiveEffects()` 完成后端切换

## 影响说明

- 启动体感更快：特效先以 CPU 生效。
- GPU 后端切换从“启动阻塞”变为“启动后异步阶段切换”。
- `probe_now` 等需要即时结果的调试路径不受影响。

## 验证建议

1. 配置 `render_backend=dawn`，启动应用。
2. 观察启动初期特效是否立即可见（不再长时间无响应）。
3. 启动后 1~2 秒内查看诊断区，确认后端按配置完成切换。
4. 重复切换 `cpu/dawn/auto`，确认无崩溃、无配置丢失。
