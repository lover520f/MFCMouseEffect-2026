# Dawn 首次提交预热策略收紧（Stage 70）

## 现象

Stage 69 将 Dawn 首次提交流程前移后，首次长按卡顿消失，但在“启动后一段时间”出现短时卡顿。

## 调整

将 Dawn 预热改为“仅在明显空闲时执行”，并减轻预热负载：

1. 空闲触发门槛（长空闲）
- 空闲阈值：`1100ms`
- 最大等待：`18000ms`
- 轮询间隔：`100ms`
- 稳定采样：连续 `4` 次

2. 若空闲窗口未出现
- 本轮放弃预热并回退到 `not_started`，避免在活跃交互期硬插入预热。

3. 预热负载收紧
- 保留：`TrySubmitNoopQueueWork`
- 保留：`TrySubmitEmptyCommandBufferTagged("startup_warmup")`
- 移除：`TrySubmitRippleBakedPacket(...)`（减少突发负载）

## 目的

降低“启动后一段时间突发卡顿”，将预热更明确地放到用户空闲时机。

## 变更文件

- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
