# WASM 设置面板（Phase 3 增量）

## 变更摘要

本次把 WASM 插件管理能力接入到现有 Web 设置页，作为 Phase 3 的第一段可见落地。

本次目标：
- 不破坏现有 C++ 宿主架构；
- 在设置页提供插件目录与生命周期控制；
- 让诊断信息可直接查看，降低排障成本。

## 范围

### 后端（`WebSettingsServer`）
- 新增接口：`POST /api/wasm/catalog`
- 基于 `WasmPluginCatalog::Discover()` 返回：
  - `plugins[]`：`id`、`name`、`version`、`api_version`、`manifest_path`、`wasm_path`
  - `errors[]`
  - `count`、`error_count`

### 前端（WebUI + Svelte Workspace）
- 新增入口：`src/entries/wasm-main.js`
- 新增分区组件：`src/wasm/WasmPluginFields.svelte`
- 在 shell 分区新增卡片：`section_wasm_plugin`
- 在 `WebUI/app.js` 新增动作桥接：
  - `catalog`
  - `enable`
  - `disable`
  - `reload`
  - `loadManifest`
- 补齐中英文 i18n 文案（标签、状态、操作结果）。

### 构建与打包链路
- `WebUIWorkspace` 新增 `build:wasm` 与对应 `vite --mode wasm`。
- 复制脚本新增 `wasm-settings.svelte.js`。
- `WebUI/index.html` 新增 `wasm-settings.svelte.js` 脚本引入。
- `Install/MFCMouseEffect.iss` 新增预检：缺失 `webui/wasm-settings.svelte.js` 时打包失败。

## 架构说明

- WASM 设置区独立成单独 Svelte 组件，职责单一，避免把逻辑继续堆在旧文件。
- 现有自动化映射/特效/文本分区不改行为，仅新增一块管理面板。
- 面板定位为“读诊断 + 发控制命令”：
  - 状态来自 `payload.state.wasm`
  - 控制命令复用现有 API 调用模型

## 验证

1. 在 `MFCMouseEffect/WebUIWorkspace` 执行 `pnpm run build`
- 成功，包含 `dist/wasm-settings.svelte.js`

2. `Release|x64` 构建
- 清理进程/锁文件后 `MSBuild` 成功
- 说明后端新接口可编译，且 post-build WebUI 拷贝链路正常

## 当前限制

- 本次未实现更复杂的插件配置编辑器。
- 目录 UI 当前是“单选并加载”，与当前宿主生命周期模型一致。
