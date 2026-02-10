# Stage41: modern ABI 原生握手探针与策略可视化

## 目标
- 在正式接入 modern ABI 原生请求链前，先把运行时能力探针做完整、可观测。
- 明确区分：
  - `modern ABI 已检测`
  - `是否具备原生请求符号`
  - `当前采用的握手策略`

## 新增探针字段
- `DawnRuntimeProbeInfo`
  - `hasModernRequestAdapter`
  - `hasModernRequestDevice`
  - `hasInstanceProcessEvents`

## 新增状态字段
- `DawnRuntimeStatus`
  - `modernAbiNativeReady`
  - `modernAbiStrategy`
  - `modernAbiNativeDetail`
  - `modernAbiPrimeDetail`（已存在，沿用）

## 策略语义
- `modernAbiStrategy=legacy_skipped`
  - 在 modern ABI 下默认跳过 legacy callback prime，避免 `request_adapter_exception` 重复异常。
- `modernAbiStrategy=legacy_prime_try`
  - 仅在后续放开开关时使用，当前默认不走。
- `modernAbiStrategy=legacy_prime_compat`
  - `waitAny/processEvents` 存在，但 `requestAdapter/requestDevice` 仍是 legacy 导出时启用。
  - 该模式会继续尝试 legacy prime，不再误判为 `legacy_skipped`。
- `modernAbiStrategy=legacy_prime_blocked`
  - 在 `legacy_prime_compat` 下已出现 `request_adapter_exception/request_device_exception` 后触发熔断。
  - 后续不再重复发起 legacy prime，避免持续异常与日志刷屏。
- `modernAbiStrategy=legacy_callbacks`
  - 非 modern ABI 的常规路径。

## 兼容修正（2026-02-10）
- 修正 `ShouldSkipLegacyPrimeOnModernAbi()`：
  - 仅当 modern request symbols 同时存在时才跳过 legacy prime。
  - 对 `legacy_request_symbols_only` 的运行时，改为走兼容 prime 路径。
- 修正 `modernAbiNativeDetail`：
  - 对 `legacy request symbols only` 明确输出 `legacy_request_symbols_only`，避免误导为 `missing_modern_request_adapter_symbol`。
- 增加兼容 prime 熔断：
  - 首次抛出 `request_adapter_exception` 或 `request_device_exception` 后，切换到 `legacy_prime_blocked`。
  - `prime` 详情显示为 `legacy_prime_blocked_after_exception`。

## Web 诊断
- `dawn_probe` 输出新增：
  - `has_modern_request_adapter`
  - `has_modern_request_device`
  - `has_instance_process_events`
- `dawn_status` 输出新增：
  - `modern_abi_native_ready`
  - `modern_abi_strategy`
  - `modern_abi_native_detail`
- `diag=1` 文本与流日志附带：
  - `native yes/no`
  - `nativeDetail ...`
  - `strategy ...`
- `diag=1` 新增按钮：
  - `Check Symbols` 调用 `/api/gpu/runtime_symbol_check`
  - 追加一行 `symbol-check` 日志，快速判断当前 DLL 导出能力是否满足 modern 请求链。
  - 默认不触发 runtime refresh，避免重置熔断状态导致再次触发 legacy prime 异常。

## 下一步
- Stage42: 基于上述探针，接入 modern ABI 原生 adapter/device/queue 请求链（不再走 legacy 回调）。

## Stage42 实施（2026-02-10）
- 已在 `DawnRuntime.cpp` 落地 modern ABI prime：
  - 使用同名导出 `wgpuInstanceRequestAdapter` / `wgpuAdapterRequestDevice` 的 modern 签名（返回 `WGPUFuture`）。
  - 使用 `wgpuInstanceWaitAny` 驱动 future 完成，并通过 modern callback 填充请求结果。
  - 请求函数解析优先走 `wgpuGetProcAddress`，减少直接导出地址在不同 runtime 包装层下的不兼容风险。
  - `wgpuCreateInstance` 改为传入显式 `InstanceDescriptor`（零初始化）而非 `nullptr`，避免部分运行时对空描述符的兼容差异。
- 为避免再次触发 legacy 回调崩溃：
  - modern ABI 分支不再调用 legacy callback 签名 prime。
  - `ShouldSkipLegacyPrimeOnModernAbi()` 收紧为 `hasWaitAny` 即跳过 legacy。
- 符号判断修正：
  - 当 `hasWaitAny && hasRequestAdapter/hasRequestDevice` 时，视为具备 modern request 能力（即便没有 `RequestAdapter2` 命名导出）。
- 稳定性处理：
  - 将 `__try` 保护下沉到纯 C 风格小函数，避免 `C2712`（对象展开函数中使用 `__try`）编译错误。
  - 为 `waitAny` 返回 `WGPUWaitStatus_Error(3)` 的 runtime 增加兼容回退：
    - 自动转为 `processEvents` 轮询等待回调完成；
    - 诊断 `prime` 增加 `...fallback_process_events_ok/timeout` 标识，便于判断是否命中回退路径。

## 验证要点
- 编译验证：
  - `MSBuild ... /t:ClCompile /p:Configuration=Release;Platform=x64` 通过。
  - 全量链接失败仅因 `x64\\Release\\MFCMouseEffect.exe` 被运行中进程占用（`LNK1104`），非代码错误。
- 运行期观察：
  - 关注 `diag=1` 的 `prime` 字段是否从 `legacy_*` 迁移到 `modern_*` 结果。
  - 关注 `Q/E` 是否变为就绪（不再长期 `Q0:E0`）。
