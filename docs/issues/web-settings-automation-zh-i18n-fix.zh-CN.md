# Web 设置页：自动化映射中文文案残留英文修复

## 现象
- 当 UI 语言切换为中文时，自动化映射区域仍出现部分英文：
  - 选项标签（鼠标动作/手势方向/手势触发键）可能保持英文。
  - 部分中文文案中混入 `action` 英文词。

## 根因
1. 自动化下拉选项标签主要来自 `schema`，语言切换时若仅做前端 i18n 刷新而不重新拉取 `schema`，选项标签可能保留旧语言。
2. 中文 i18n 字典中存在少量历史文案未完全本地化（`action`）。
3. 自动化组件首次挂载时默认 props 使用空 `i18n`，在某些渲染时序下会先落英文 fallback，后续语言同步不稳定。

## 修复
1. 在 `AutomationEditor.svelte` 中新增选项本地化层：
   - 按选项 value 映射到 i18n key（如 `auto_mouse_action_left_click`）。
   - 在组件 hydrate 和 i18n 变更时都执行本地化重标注。
2. 在 `i18n.js` 中补齐自动化选项键：
   - `auto_mouse_action_*`
   - `auto_gesture_button_*`
   - `auto_gesture_pattern_*`
3. 修正中文文案中残留英文：
   - `鼠标 action` -> `鼠标动作`
4. 强化自动化组件挂载时序：
   - `automation/api.js` 维护 `latestProps`，并在首次 `render/syncI18n` 时直接用最新 `schema/payload/i18n` 创建组件，避免“先空词典渲染再补词典”的窗口期。
5. 强化自动化组件文本响应式：
   - `AutomationEditor.svelte` 增加 `uiText`/`mousePanelTexts`/`gesturePanelTexts` 的显式刷新路径，`i18n` 变更时统一重算并驱动视图更新。
6. 在入口侧增加语言同步兜底：
   - `WebUI/app.js` 的 `reload()` 在 `MfxAutomationUi.render(...)` 后追加 `MfxAutomationUi.syncI18n(i18n)`，确保渲染时序下仍以当前语言覆盖展示文案。

## 影响范围
- 文件：
  - `MFCMouseEffect/WebUIWorkspace/src/automation/AutomationEditor.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/automation/api.js`
  - `MFCMouseEffect/WebUI/app.js`
  - `MFCMouseEffect/WebUI/i18n.js`
- 仅影响 Web 设置页自动化映射文案展示，不改变配置结构和后端协议。

## 验证
1. `node --check MFCMouseEffect/WebUIWorkspace/src/automation/AutomationEditor.svelte`（通过语法构建链间接校验）。
2. `pnpm run build`（`MFCMouseEffect/WebUIWorkspace`）成功。
3. 手动验证：
   - 中文模式下自动化映射区的下拉选项和提示文案应全部中文。
   - 在中文/英文间切换时，自动化选项标签随语言同步更新。

## 构建同步改进（避免“改了但运行仍旧包”）
- 问题：仅执行 `pnpm run build` 时，历史流程只会把产物写回 `MFCMouseEffect/WebUI`，若当前运行 `x64/Release/MFCMouseEffect.exe`，其读取的是 `x64/Release/webui`，可能仍是旧资源。
- 处理：增强 `MFCMouseEffect/WebUIWorkspace/scripts/copy-output.mjs`。
  - 仍保留 `dist -> MFCMouseEffect/WebUI` 的主同步。
  - 若存在 `x64/Debug/webui`、`x64/Release/webui`，额外自动镜像关键资源（Svelte bundles + `i18n.js` 等静态文件）到这两个运行目录。
- 结果：在本地开发阶段执行一次 `pnpm run build`，即可同步更新常用运行目录，减少中英文不一致的“假未生效”问题。

