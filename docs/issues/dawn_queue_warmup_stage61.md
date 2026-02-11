# Dawn 首段预热降抖（Stage 61）

## 问题

在 `cpu/auto -> dawn` 或应用启动初段，队列刚 ready 的前若干帧容易出现鼠标短时卡顿。

## 分析

- 该阶段常伴随拖尾历史点集中进入 Dawn 路径。
- `PreprocessTrailGeometry` 在首批帧内会出现一次性较重工作量，叠加初始化尾部动作，容易抖动。

## 方案

在 `DawnCommandConsumer` 增加“Dawn 激活后短暂预热窗口”：

1. 当 backend 从非 dawn 切到 dawn 时启动预热窗口。
2. 预热窗口参数：
- 最长 `1800ms`
- 或最多 `96` 帧（先到先结束）
3. 预热触发时机：
- backend 从非 dawn 切到 dawn；
- 或 runtime `queueReady` 从 `false -> true`。
4. 预热窗口内若存在 `trail` 命令：
- 跳过 trail 几何预处理（避免首段尖峰）
- 尝试 no-op submit 保持队列活跃
- 诊断 detail 标记：`accepted_queue_ready_warmup_trail_preprocess_skipped*`

## 影响

- 目标是把“启动/切换 Dawn 初段卡顿”压平。
- 预热窗口内 trail 视觉会短暂降级，但不会影响后续稳定阶段。

## 变更文件

- `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
