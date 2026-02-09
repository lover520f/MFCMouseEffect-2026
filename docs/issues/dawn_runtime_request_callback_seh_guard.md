# Dawn Runtime requestAdapter/requestDevice 崩溃保护（SEH）

## 背景
在 Debug 环境加载 `webgpu_dawn.dll` 后，`TryInitializeDawnRuntime()` 的动态调用阶段出现访问冲突：
- `requestAdapter` 或 `requestDevice` 调用进入 DLL 后触发 `0xC0000005`
- 进程被二进制内部异常带崩，影响稳定性

## 根因
当前工程通过 `GetProcAddress + 函数指针` 调用 Dawn C API。不同 runtime 版本/ABI 组合下，回调签名或内部路径可能不兼容，异常发生在外部 DLL 内部，不能只靠普通 C++ 异常捕获。

## 修复
在 `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp` 中：
1. 对 `requestAdapter` 动态调用增加 `__try/__except` 保护。
2. 对 `requestDevice` 动态调用增加 `__try/__except` 保护。
3. 异常时返回 CPU 回退状态，不再导致进程崩溃：
   - `dawn_request_adapter_exception`
   - `dawn_request_device_exception`
4. 补齐失败路径资源释放，避免实例/适配器句柄泄漏。

在 `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp` 中：
1. 增加上述两个状态码映射。
2. 前端状态建议落入 `check_driver_and_backend` 路径，提示检查驱动和 runtime 兼容性。

## 结果
- 遇到不兼容 Dawn runtime 时，程序稳定退回 CPU，不会因 DLL 内异常直接退出。
- UI/状态接口可见明确异常码，便于后续定位 runtime 版本问题。
## 更新（调试器附加场景）

为解决 VS Debug 附加时 `webgpu_dawn.dll` first-chance 访问冲突频繁中断：

1. 在 `DawnRuntime.cpp` 增加调试器检测（仅 `_DEBUG`）。
2. 当检测到调试器附加时，跳过 `requestAdapter/requestDevice` 深度握手。
3. 返回状态 `dawn_handshake_skipped_debugger`，并继续 CPU 兜底运行。
4. Web 状态页新增对应状态码与说明，引导用户“非调试运行再做完整 GPU 探测”。

说明：
- 这不是隐藏错误，而是避免调试器在第三方 DLL first-chance 上反复打断开发流程。
- Release / 非调试运行时仍会执行完整握手与能力探测。
- 调整：调试器检测不再限定 `_DEBUG`。
- 只要 `IsDebuggerPresent()==true`（包括 Release 配置下附加调试器），都跳过 Dawn 的 requestAdapter/requestDevice 深握手，避免 third-party DLL first-chance 异常中断调试流程。
## 更新（现代 ABI 兼容过渡）

在检测到运行时导出 `wgpuInstanceWaitAny`（现代 Dawn/WebGPU ABI）时：
- 不再调用旧版回调签名的 `requestAdapter/requestDevice`，避免 ABI 不匹配导致异常。
- 改为 `createInstance` 成功后进入现代 ABI 过渡路径：
  - `dawn_overlay_bridge_ready_modern_abi`
  - `dawn_modern_abi_bridge_pending`
- 这样可先启用 Dawn backend（由 Overlay bridge 状态决定），后续再补齐 full 新 ABI 的深度握手实现。
## 更新（死锁修复）

修复 `std::system_error (RESOURCE_DEADLOCK_WOULD_OCCUR)`：
- `TryInitializeDawnRuntime()` 持有 `g_probeMutex` 时，原先调用 `GetDawnOverlayBridgeStatus()`，后者会再调用 `GetDawnRuntimeProbeInfo()` 触发同线程重入锁。
- 现改为在该锁内路径使用 `IsOverlayBridgeCompiled()`（编译期开关判断），避免重入 `g_probeMutex`。

结果：
- 消除同线程重复加锁导致的未处理异常。
- 保持 Dawn 初始化状态机行为不变（仅移除重入查询）。
