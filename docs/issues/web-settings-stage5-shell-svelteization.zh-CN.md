# Web 设置页阶段 5.1：页面壳层 Svelte 化（JS-only）

## 背景
虽然核心分区和表单已逐步迁移到 Svelte，但 `index.html` 仍保留大量静态壳层 DOM（背景、顶部按钮、卡片容器）。
这会导致页面结构演进时需要同时维护大块 HTML 与多处脚本挂载约束。

本阶段把壳层迁移为 Svelte 组件，并保持 JS-only（不引入 TypeScript）。

## 本次改动

### 1. 新增壳层组件（Svelte + JS）
- `MFCMouseEffect/WebUIWorkspace/src/shell/WebSettingsShell.svelte`
- `MFCMouseEffect/WebUIWorkspace/src/shell/TopBar.svelte`
- `MFCMouseEffect/WebUIWorkspace/src/shell/SettingsGrid.svelte`
- `MFCMouseEffect/WebUIWorkspace/src/shell/sections.js`
- `MFCMouseEffect/WebUIWorkspace/src/entries/shell-main.js`

设计点：
- `TopBar` 专注品牌信息与操作按钮。
- `SettingsGrid` 专注分区卡片与 mount 点布局。
- `sections.js` 统一管理卡片元数据，避免在视图文件硬编码重复文案键。

### 2. 构建链路扩展
- `MFCMouseEffect/WebUIWorkspace/vite.config.js`
  - 新增 `shell` 构建模式，产物 `settings-shell.svelte.js`
- `MFCMouseEffect/WebUIWorkspace/package.json`
  - 新增 `build:shell`
  - `build` 总流程中先执行 `build:shell`
- `MFCMouseEffect/WebUIWorkspace/scripts/copy-output.mjs`
  - 新增 `settings-shell.svelte.js` 复制到 `MFCMouseEffect/WebUI`

### 3. 页面接入方式调整
- `MFCMouseEffect/WebUI/index.html`
  - 移除大段静态壳层 DOM
  - 仅保留 `<div id="web_settings_shell_mount"></div>` 挂载点
  - 加载 `settings-shell.svelte.js`，并放在各分区 bundle 之前

## 兼容性与约束
- 保留以下关键 id，不改外部契约：
  - `status`
  - `btnReload` / `btnReset` / `btnStop` / `btnSave`
  - `workspace_sidebar_mount`
  - `general_settings_mount` / `effects_settings_mount` / `input_indicator_settings_mount` / `text_settings_mount` / `trail_settings_mount` / `automation_editor_mount`
- 保留 `data-i18n` / `data-i18n-title` / `data-i18n-placeholder` 语义，不影响现有 i18n 运行时。
- 全程未引入 `.ts`，仍为 JS-only。

## 验证记录
1. 在 `MFCMouseEffect/WebUIWorkspace` 执行 `pnpm run build` 成功。
2. 产物已复制到 `MFCMouseEffect/WebUI/settings-shell.svelte.js`。
3. 后续 `MSBuild x64 Debug` 构建成功，PostBuild 日志包含 `settings-shell.svelte.js` 拷贝。

## 后续建议（阶段 5.2）
1. 将按钮行为绑定从 `app.js` 抽离到可复用动作模块，进一步降低入口复杂度。
2. 评估将 `status` 显示逻辑迁移为 Svelte store 驱动，减少直接 DOM class 操作。


