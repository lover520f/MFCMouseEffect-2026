# Stage38: Dawn noop queue submit 诊断闭环

## 背景
- Stage35-37 已完成命令消费与拖尾几何预处理，但还缺少一个最小 GPU 提交通路验证。
- 仅有 `adapter/device` 成功并不能证明队列提交通路可用，用户侧会出现“已启用 GPU 但体验无变化”的困惑。

## 本阶段目标
- 在不引入真实 DrawPass 的前提下，增加一次 `wgpuQueueSubmit(queue, 0, nullptr)` 的 noop 提交尝试。
- 将尝试次数与成功次数暴露到 Web 诊断，便于区分：
  - 命令预处理成功但提交通路不可用
  - 提交通路可用但仍以 CPU 绘制为主

## 代码设计
### 1. DawnRuntime 持有最小队列上下文
- 在 `TryInitializeDawnRuntime()` 成功创建 device 后，尝试获取 queue。
- 缓存 live `device/queue` 与对应 release/submit proc。
- `ResetDawnRuntimeProbe()` 时统一释放，避免句柄泄漏。

### 2. 提供安全 noop 提交接口
- 新增 `TrySubmitNoopQueueWork(std::string* detailOut)`：
  - 队列未就绪返回 `queue_not_ready`
  - 提交成功返回 `queue_submit_noop_ok`
  - 提交异常返回 `queue_submit_noop_exception`

### 3. DawnCommandConsumer 集成
- 在有拖尾三角几何输出时触发 noop submit。
- 增加统计字段：
  - `noopSubmitAttempts`
  - `noopSubmitSuccess`
- `detail` 细化为：
  - `accepted_trail_geometry_prepared_and_submitted`
  - `accepted_trail_geometry_prepared_submit_pending_*`

## Web 诊断字段
- `dawn_command_consumer.noop_submit_attempts`
- `dawn_command_consumer.noop_submit_success`
- `?diag=1` 文案追加：`提交: success/attempts`

## 影响与边界
- 这是“GPU 队列通路可用性诊断”，不是完整 GPU 绘制切换。
- 即使 submit 成功，当前特效主绘制仍可能主要消耗 CPU（尤其拖尾/悬浮/长按）。

## 后续建议
- Stage39: 将拖尾批次上传+DrawPass 落到真实 GPU pipeline。
- Stage40: 将长按/悬浮热点路径迁移到 GPU 侧或降低 CPU 热路径分配频率。
