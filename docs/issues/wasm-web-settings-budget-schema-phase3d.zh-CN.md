# WASM 预算策略 Schema 驱动（Phase 3d）

## 变更摘要

本阶段移除了 Web 面板里预算输入的硬编码范围，改为由后端 schema 驱动。

## 改动内容

1. Schema 默认值/范围
- `SettingsSchemaBuilder` 中 WASM 预算 `default` 改为“内置默认配置”而非“当前运行配置”。
- 这样“恢复默认”行为在不同机器与配置状态下都一致。

2. Web 预算模型
- 新增 `WebUIWorkspace/src/wasm/policy-model.js`：
  - 统一解析 schema 范围
  - 按 `min/max/step` 做 clamp + step 对齐
  - 统一预算输入默认值决策

3. WASM 分区接线
- `settings-form.js` 传入 `schema.wasm` 到 WASM 分区。
- `wasm-main.js` 透传规范化后的 schema 状态。
- `WasmPluginFields.svelte` 改为：
  - 预算输入 `min/max/step` 全部来自 schema
  - 保存策略时统一走共享模型 clamp
  - 新增“恢复默认策略”按钮（恢复后立即持久化）

4. i18n
- 新增 `btn_wasm_reset_policy`（中英文）。

## 验证

1. `WebUIWorkspace` 下 `pnpm run build` 通过。
2. `Release|x64` MSBuild 通过。

## 说明

- 预算约束现在以服务端 schema 为单一事实源，避免前后端范围漂移。
