# Web 设置页阶段 5.3：壳层交互桥接到 Svelte 状态层（JS-only）

## 背景
阶段 5.1 后，页面壳层 DOM 已迁移到 Svelte，但 `app.js` 仍直接操作顶部按钮和状态横幅（`status`）节点。
这会让壳层 Svelte 化与运行态交互层割裂，仍存在较多 imperative DOM 操作。

本阶段把这部分交互桥接到 Svelte 壳层状态层，同时保持 JS-only，不引入 TypeScript。

## 本次改动

### 1. TopBar 改为动作事件派发
- 文件：`MFCMouseEffect/WebUIWorkspace/src/shell/TopBar.svelte`
- 变更：
  - 新增 `actionsDisabled` 属性。
  - `Reload/Apply/Reset/Stop` 按钮改为派发动作事件：
    - `reload`
    - `save`
    - `reset`
    - `stop`

### 2. Shell 接管状态横幅渲染
- 文件：`MFCMouseEffect/WebUIWorkspace/src/shell/WebSettingsShell.svelte`
- 变更：
  - 新增 `statusMessage`、`statusTone`、`actionsDisabled`、`onAction` 属性。
  - 通过响应式 class 生成逻辑统一管理状态横幅样式（`show/warn/ok/offline`）。
  - 接收 `TopBar` 动作并向外转发。

### 3. 新增壳层桥接 API
- 文件：`MFCMouseEffect/WebUIWorkspace/src/entries/shell-main.js`
- 变更：
  - 新增全局桥接对象 `window.MfxWebShell`：
    - `onAction(listener)`：订阅顶部动作
    - `setStatus(message, tone)`：更新状态横幅
    - `setActionsEnabled(enabled)`：控制顶部动作按钮可用性

### 4. `app.js` 接入桥接并保留降级路径
- 文件：`MFCMouseEffect/WebUI/app.js`
- 变更：
  - `setStatus` 与 `setActionButtonsEnabled` 优先调用 `MfxWebShell`。
  - 顶部动作优先通过 `MfxWebShell.onAction` 绑定，不再依赖固定按钮 DOM。
  - 保留 legacy DOM fallback（壳层桥接不可用时仍可工作）。

## 兼容性
- 不改后端 API 和配置结构。
- 不改 `window.MfxDialog`、`window.MfxAutomationUi`、`window.MfxSettingsForm` 契约。
- 顶部按钮 id 与 `status` id 仍保留，兼容 legacy 降级路径。

## 验证记录
1. `pnpm run build`（`WebUIWorkspace`）通过，壳层 bundle 正常输出并复制。
2. `node --check` 通过：
   - `MFCMouseEffect/WebUI/app.js`
   - `MFCMouseEffect/WebUIWorkspace/src/entries/shell-main.js`
3. `MSBuild x64 Debug` 通过，PostBuild 产物完整。
4. 动作回归：
   - 重载、应用、恢复默认、关闭监听行为与迁移前一致。
   - 状态横幅颜色和文本反馈一致。


