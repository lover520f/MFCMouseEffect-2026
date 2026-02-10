# Stage40: Dawn 空命令缓冲提交骨架

## 目标
- 在 `queue submit(0, nullptr)` 之上，增加一条更接近真实渲染管线的最小命令路径：
  - `DeviceCreateCommandEncoder`
  - `CommandEncoderFinish`
  - `QueueSubmit(1, commandBuffer)`
- 先验证“命令编码->提交”链路可用性，再进入真正 DrawPass。

## 关键改动
- `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h/.cpp`
  - 新增 `TrySubmitEmptyCommandBuffer(std::string* detailOut)`
  - 解析并缓存命令编码相关 API：
    - `wgpuDeviceCreateCommandEncoder`
    - `wgpuCommandEncoderFinish`
    - `wgpuCommandEncoderRelease`
    - `wgpuCommandBufferRelease`
  - 使用 SEH 安全包装执行空命令缓冲提交，避免 DLL 侧异常直接中断进程。
  - 在 `modern ABI` 检测分支中新增“legacy 回调握手尝试”（短超时、失败回退）：
    - 成功时 detail: `dawn_overlay_bridge_ready_modern_abi_queue_ready`
    - 失败时保持原 detail: `dawn_overlay_bridge_ready_modern_abi`
  - 结合实测 `request_adapter_exception`，默认关闭该 legacy prime（避免反复异常）：
    - `modern_abi_prime_detail=legacy_callback_incompatible_skipped`
    - 保持 CPU 兜底，等待后续原生 modern ABI 握手接入

- `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
  - 在 trail 几何预处理成功后：
    - 先做 noop submit
    - 再做 empty command buffer submit
  - 新增诊断计数：
    - `emptyCommandSubmitAttempts`
    - `emptyCommandSubmitSuccess`

- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
  - API 输出新增：
    - `empty_command_submit_attempts`
    - `empty_command_submit_success`
  - `dawn_status` 新增：
    - `queue_ready`
    - `command_encoder_ready`
    - `modern_abi_detected`
    - `modern_abi_prime_detail`（legacy prime 失败阶段原因）
  - `dawn_probe` 新增：
    - `has_wait_any`

- `MFCMouseEffect/WebUI/app.js`
  - `?diag=1` 增加 `命令缓冲提交: success/attempts`。

## 边界
- 该阶段仍未进行真实 DrawPass 绑定/渲染目标提交。
- 作用是确认 Dawn 命令缓冲生命周期与队列提交通道的稳定性。
- 若 `queue_ready=false`，消费层会进入 `*_waiting_queue` 状态，不再无意义累加提交尝试计数。
- 若同时 `modern_abi_detected=true`，消费 detail 会标注为 `*_waiting_queue_modern_abi`。
- 为避免“queue 未就绪阶段”的无效 CPU 消耗，预处理会直接跳过并返回
  - `accepted_waiting_queue_preprocess_skipped`
  - `accepted_waiting_queue_modern_abi_preprocess_skipped`
- Web 状态条在 `gpu_in_use=true && queue_ready=false` 时会追加明确提示：
  - GPU 后端已选中，但当前仍以 CPU 路径为主。
- 服务层补充状态码：`queue_not_ready`，用于明确区分“运行时已加载”与“队列握手未完成”。
- `diag=1` 页面增加高频轮询（500ms）与刷新年龄显示，便于区分“前端未刷新”与“后端确实无命令”：
  - 命令流显示 `frame_tick_ms`
  - 显示 `刷新: Nms前`
- `diag=1` 页面新增长文本诊断流面板（可复制/清空）：
  - 实时追加关键状态行（backend/command stream/queue/submit/detail）
  - 追加 `prime=...`，用于显示 modern ABI queue 预热失败点
  - 便于用户直接贴日志文本进行远程排查
- 兼容策略调整（latest modern ABI 可运行优先）：
  - 当 `modernAbiDetected=true` 且桥接就绪时，即使 `queue_ready=false` 也允许 Dawn 后端激活；
  - 诊断继续明确 `queue/encoder` 未就绪，避免误判为“完整 GPU 渲染”。
  - 对应实现：`TryInitializeDawnRuntime()` 在 modern ABI + bridge ready 分支固定 `ok=true, backend=dawn`，
    queue 是否就绪由 `queue_ready/command_encoder_ready` 单独反映。

## 下一步
- Stage41: 对 trail 批次建立真实 GPU pipeline（vertex buffer + render pass）。
- Stage42: 迁移 hover/hold 的热点路径到同一提交流水线并加节流策略。
