# Dawn 消费热路径锁收紧（Stage 54）

## 目标

降低 GPU 消费热路径中不必要的长时互斥锁占用，减少高频输入下的卡顿放大风险。

## 问题

`SubmitOverlayGpuCommands(...)` 之前把整段逻辑放在 `DawnConsumerMutex` 下执行，包含：

- 命令统计
- 几何预处理（含并行分支）
- 提交判定与状态拼接

这会让状态锁覆盖重计算路径，不利于交互平滑。

## 调整

### 1) 改为“快照 + 发布”

- 进入函数时短锁读取 `DawnCommandConsumeStatus` 快照。
- 计算/预处理/提交流程在锁外执行。
- 所有 return 分支在退出前短锁回写最终状态。

### 2) 节流 tick 改为原子

- `s_lastNonTrailSubmitTickMs` 由普通静态变量改为 `std::atomic<uint64_t>`。
- 避免在无大锁保护时出现数据竞争。

## 影响

- 行为语义保持不变（诊断字段、计数器、detail 输出未改协议）。
- 锁持有时间显著缩短，降低热路径阻塞概率。

## 变更文件

- `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
