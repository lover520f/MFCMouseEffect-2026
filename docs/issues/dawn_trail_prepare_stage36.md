# Dawn Trail 预处理（Stage 36）

## 目标
- 在 Stage35 的“消费闭环”上继续前进。
- 对 `TrailPolyline` 命令做批次化预处理，为下一阶段真实 DrawPass 做输入准备。

## 实现点
- 文件：`MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
- `SubmitOverlayGpuCommands(...)` 中新增：
  - Trail 顶点按批次切分（每批最多 1024 顶点，批次间保留 1 个重叠点）
  - 统计：
    - `preparedTrailBatches`
    - `preparedTrailVertices`
    - `preparedTrailSegments`
  - `detail` 升级：
    - `accepted_trail_batches_prepared`
    - `accepted_no_trail_batches`

## 状态透出
- 文件：`MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
  - `/api/state`、`/api/gpu/probe_now`、`/api/gpu/bridge_mode` 的 `dawn_command_consumer` 均新增预处理统计字段。
- 文件：`MFCMouseEffect/WebUI/app.js`
  - `?diag=1` 横幅新增 Trail 预处理统计显示（批次/顶点/线段）。

## 边界
- 本阶段仍未提交真实 Dawn DrawPass，仅完成“输入预处理 + 可观测统计”。
- CPU 兜底路径与现有视觉效果保持不变。

