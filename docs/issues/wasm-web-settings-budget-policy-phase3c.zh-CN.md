# WASM 预算策略控制（Phase 3c）

## 变更摘要

本次把 WASM 执行预算从硬编码改为“可配置 + 可持久化”。

新增策略字段：
- `output_buffer_bytes`
- `max_commands`
- `max_execution_ms`

这些字段会写入 `config.json`，并同步到设置页状态，同时应用到运行时 WASM Host。

## 架构改动

### 1. 配置模型与编解码
- `WasmConfig` 新增预算字段。
- WASM 配置解析/序列化新增预算键处理。
- 新增清洗规则：
  - `output_buffer_bytes`: `1024..262144`
  - `max_commands`: `1..2048`
  - `max_execution_ms`: `0.1..20.0`

### 2. AppController 策略应用
- 新增 `SetWasmExecutionBudget(...)`。
- 新增 `ApplyWasmConfigToHost(bool tryLoadManifest)` 统一应用入口。
- 启动与重载时会同时应用策略与预算到 Host。

### 3. 命令/API 桥接
- `wasm_set_policy` 支持预算字段。
- `/api/wasm/policy` 转发预算更新。
- 数值解析增加防御，避免负数/溢出导致异常。

### 4. Web 设置页
- WASM 分区新增预算输入框与策略保存按钮。
- 同时展示“配置预算值”和“运行时实际预算值”。

## 验证

1. `pnpm run build`（WebUIWorkspace）通过。
2. `Release|x64` MSBuild 通过。

## 说明

- 预算策略由宿主统一 clamp 和执行，避免插件侧破坏稳定性。
- 运行时快照可用于定位“配置值”和“实际生效值”不一致问题。
