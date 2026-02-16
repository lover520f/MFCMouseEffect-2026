# Web 设置页阶段 5.2：Dialog Svelte 化（JS-only）

## 背景
Web 设置页在断连拦截与重置确认中依赖 `window.MfxDialog`。此前实现位于 `MFCMouseEffect/WebUI/dialog.js`，仍是手写 DOM 逻辑。

本阶段把弹窗层迁移到 Svelte，并保持既有 API 契约不变，避免影响 `app.js` 调用链。

## 本次改动

### 1. 新增 Svelte Dialog 组件与入口
- `MFCMouseEffect/WebUIWorkspace/src/dialog/DialogHost.svelte`
- `MFCMouseEffect/WebUIWorkspace/src/entries/dialog-main.js`

实现要点：
- 支持 `notice` / `confirm` 两种模式。
- 保留 Esc 关闭、遮罩点击关闭、主按钮自动聚焦。
- 继续使用 `body.mfx-modal-open` 控制页面滚动锁定。

### 2. 全局 API 兼容保持
- `window.MfxDialog.showNotice(opts)`
- `window.MfxDialog.showConfirm(opts)`

兼容策略：
- 对 `title/message/okText/cancelText/onClose` 参数保持一致。
- `showConfirm` 继续返回 `Promise<boolean>`。

### 3. 构建与接入更新
- `MFCMouseEffect/WebUIWorkspace/vite.config.js`：新增 `dialog` 模式，产物 `dialog.svelte.js`
- `MFCMouseEffect/WebUIWorkspace/package.json`：新增 `build:dialog` 并纳入 `build`
- `MFCMouseEffect/WebUIWorkspace/scripts/copy-output.mjs`：新增 `dialog.svelte.js` 拷贝
- `MFCMouseEffect/WebUI/index.html`：脚本由 `/dialog.js` 切换为 `/dialog.svelte.js`
- 删除 legacy 文件：`MFCMouseEffect/WebUI/dialog.js`

## 兼容性
- `app.js` 中对 `MfxDialog` 的调用无需改动。
- 样式继续复用 `styles.css` 的 `.mfx-modal-*` 样式类。
- 本阶段仅使用 JS，不引入 TypeScript。

## 验证记录
1. `pnpm run build`（`WebUIWorkspace`）通过，输出并复制 `dialog.svelte.js`。
2. `MSBuild x64 Debug` 通过，PostBuild 日志包含 `dialog.svelte.js`。
3. 关键行为回归：
   - 断连点击按钮时弹窗正常显示并阻断操作。
   - 重置确认弹窗确认/取消行为正确。
   - Esc 与遮罩点击关闭行为与预期一致。


