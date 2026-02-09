# Dawn Trail 几何预处理（Stage 37）

## 目标
- 将 Stage36 的 Trail 批次化进一步升级为“可上传的三角形几何”准备。
- 在不改变现有 CPU 渲染结果前提下，验证 GPU DrawPass 前的数据路径完整性。

## 主要改动
- 文件：`MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
  - 对每个 `TrailPolyline` 命令执行：
    - 批次切分（最多 1024 顶点/批次，批次间 1 顶点重叠）
    - 线段转四边形（每段 2 三角形）
    - 生成 upload-ready 的浮点打包数据（`x,y,color`）
  - 新增统计：
    - `preparedTrailTriangles`
    - `preparedUploadBytes`
    - `noopSubmitAttempts`
    - `noopSubmitSuccess`
  - `detail` 升级：
    - `accepted_trail_geometry_prepared`
    - `accepted_no_trail_geometry`

## 诊断透出
- 文件：`MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
  - `dawn_command_consumer` 新增：
    - `prepared_trail_triangles`
    - `prepared_upload_bytes`
- 文件：`MFCMouseEffect/WebUI/app.js`
  - `?diag=1` 横幅显示 `b/v/s/t/u`（批次/顶点/线段/三角形/上传字节）以及 `submit ok/try`。

## 说明
- 本阶段新增了真实 `wgpuQueueSubmit(0)` 探针（有队列上下文时）。
- 仍未实现真实 `wgpuQueueWriteBuffer + RenderPass`。
- 但 Trail 已具备“几何转换 + 上传体量估算 + 状态可观测”，可直接衔接下一阶段真正绘制。
