# WASM 动作后的状态刷新优化（Phase 3f）

## 变更摘要

WASM 面板动作不再每次都触发全量设置重载。

## 问题

在本阶段之前，以下动作：
- 启用 / 禁用
- 重载插件
- 加载清单
- 更新策略

都会调用 `reload()`，即每次都：
- 拉取 `/api/schema`
- 拉取 `/api/state`
- 全页面分区重渲染

这会带来不必要的请求开销和可见抖动。

## 方案

对 `WebUI/app.js` 做了分层刷新改造：

1. `renderSettingsSnapshot(schema, state)`
- 统一渲染、i18n 同步、工作区刷新和状态收口。

2. `refreshStateSnapshot(useCachedSchema)`
- 固定拉最新 `/api/state`
- 当语言未变时复用缓存 schema
- 仅在必要时再拉 `/api/schema`

3. `refreshAfterWasmAction()`
- WASM 动作优先走“状态优先刷新”
- 局部刷新失败时自动回退到全量 `reload()`，保证稳定性

## 效果

- WASM 动作后的状态与诊断更新更快，页面抖动明显减少。
- 手动重载与异常恢复仍保留全量刷新链路。

## 验证

1. `node --check MFCMouseEffect/WebUI/app.js` 通过。
2. `pnpm run build`（WebUIWorkspace）通过。
3. `Release|x64` MSBuild 通过。
