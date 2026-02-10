# Stage39: 拖尾几何预处理并行化与解耦

## 目标
- 将拖尾几何预处理逻辑从 `DawnCommandConsumer` 拆分，降低耦合。
- 在大负载下利用多核并行预处理，缓解拖尾/长按/悬浮场景下的 CPU 峰值。
- 保持当前行为不变：仍是诊断链路，不引入真实 DrawPass 渲染变更。

## 改动概览
- 新增 `MFCMouseEffect/MouseFx/Gpu/DawnTrailGeometryPreprocessor.h`
  - `PreprocessTrailGeometry(const OverlayGpuCommandStream&)`
  - 输出 `TrailGeometryPrepResult`（batches/vertices/segments/triangles/uploadBytes/workers/usedParallel）
- `DawnCommandConsumer` 中：
  - 删除内联的大段几何预处理实现，改为调用预处理器
  - 新增诊断字段：
    - `preprocessWorkers`
    - `preprocessParallel`
    - `preparedParticleBatches`
    - `preparedParticleSprites`
    - `preparedParticleUploadBytes`
  - `detail` 增加并行标识分支：
    - `accepted_trail_geometry_prepared_parallel_and_submitted`
    - `accepted_trail_geometry_prepared_parallel_submit_pending_*`

## 并行策略
- 并行触发条件：
  - `trail command >= 2`
  - `total trail vertices >= 2048`
- worker 数：
  - `min(trail_command_count, hardware_concurrency)`
- 异常回退：
  - 若 `std::async` 启动失败，自动回退单线程预处理

## 粒子命令预处理（新增）
- 对 `ParticleSprites` 命令新增批次数/精灵数/估算上传字节统计。
- 该统计先用于诊断（定位悬浮/长按的粒子开销），不改变现有渲染输出。

## Web 诊断
- `dawn_command_consumer` 新增：
  - `preprocess_workers`
  - `preprocess_parallel`
  - `prepared_particle_batches`
  - `prepared_particle_sprites`
  - `prepared_particle_upload_bytes`
- `?diag=1` 文案显示：
  - `预处理并行: 是/否 (wN)`

## 结果与边界
- 这一步解决的是“GPU前的数据准备开销”问题，仍未进入真实 GPU 绘制流水线。
- 下一步（Stage40）建议：将拖尾预处理结果接入真正的 GPU DrawPass 上传与提交。
