# WASM 运行时诊断可视化（Phase 3e）

## 变更摘要

本阶段将后端已提供的 WASM 执行预算诊断信息完整展示到 Web 面板，补齐运维可观测性。

## 改动内容

1. 诊断模型抽取
- 新增 `WebUIWorkspace/src/wasm/diagnostics-model.js`。
- 统一处理：
  - 诊断字段归一化
  - 风险判定（预算超限/截断/解析异常）
  - 调用指标与预算标记文本格式化

2. 状态归一化扩展
- `WebUIWorkspace/src/entries/wasm-main.js` 新增透传字段：
  - `last_call_duration_us`
  - `last_output_bytes`
  - `last_command_count`
  - `last_call_exceeded_budget`
  - `last_call_rejected_by_budget`
  - `last_output_truncated_by_budget`
  - `last_command_truncated_by_budget`
  - `last_budget_reason`
  - `last_parse_error`

3. 面板展示增强
- `WasmPluginFields.svelte` 新增展示项：
  - 最近调用指标
  - 预算标记
  - 预算原因
  - 解析错误
- 当检测到预算/解析风险时，使用 `is-warn` 样式高亮。

4. 文案与样式
- i18n 新增中英文键值（诊断标签与标记名）。
- 新增 `.wasm-value.is-warn` 样式。

## 验证

1. `WebUIWorkspace` 下 `pnpm run build` 通过。
2. `Release|x64` MSBuild 通过。

## 说明

- 本阶段仅增强可视化，不改变 Host/WASM 执行逻辑。
- 可快速区分“渲染异常”与“预算拒绝/截断”两类问题来源。
