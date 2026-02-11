# Dawn 诊断时间线（Stage 80）

## 背景
用户需要一次性复制诊断数据，能看到 `dawn_command_consumer` 的时间变化，而不是只看最后一帧。

## 本次改动
1. 在 `DawnCommandConsumer` 增加时间线快照结构：`DawnCommandConsumeTimelinePoint`。
2. 每次 `publish()` 时将当前消费状态写入 ring-like 缓冲（最大 180 条，超出丢弃最早记录）。
3. `/api/state` 新增字段：
   - `dawn_command_consumer_timeline`: 最近消费快照数组。
   - `dawn_command_consumer_timeline_max`: 最大保留条数（当前 180）。

## 字段说明（timeline 单条）
- `submit_tick_ms`: 帧时间戳。
- `command_count` / `trail_commands` / `ripple_*` / `particle_commands`: 本帧命令统计。
- `prepared_*`: 预处理结果摘要（trail/ripple/particle）。
- `preprocess_workers` / `preprocess_parallel`: 预处理并行信息。
- `pass_warmup_*`: pass 预热状态。
- `detail`: 本帧状态细节码。

## 影响
- 不改变渲染路径和 CPU 兜底策略，仅增强诊断可观测性。
- 内存开销可控：最多 180 条，且为轻量结构。
