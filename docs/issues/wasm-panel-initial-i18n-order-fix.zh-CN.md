# WASM 面板首屏 i18n 顺序修复

## 问题
- 中文模式下，WASM 分区首屏会残留部分英文。
- 点击一次“重载”后才会恢复成中文。

## 根因
- `applyI18n(uiLang)` 在分区组件挂载前执行。
- 首次 i18n 扫描时，Svelte 新插入的 `data-i18n` 节点尚未进入 DOM，导致沿用英文兜底文本。
- 另外存在初始化竞态：当 `MfxWasmSection` 脚本晚于首轮 `settings-form.render` 注册时，WASM 分区会被直接跳过渲染，直到手动重载才补渲染。
- 进一步确认还有“懒挂载”场景：分区 DOM 在首轮 `applyI18n` 之后才插入（例如工作区切换后挂载），即使脚本已注册，也会残留英文直到下一次全局重载。

## 修复
- 保留渲染前的 `applyI18n(uiLang)`。
- 在分区渲染/挂载完成后，再执行一次 `applyI18n(uiLang)`。
- 确保首屏首次加载就能覆盖到 WASM 分区的新节点，不再需要手动重载。
- `settings-form.js` 增加 WASM 分区“未就绪重试渲染”机制：首次 render 若检测到 `MfxWasmSection` 未注册，则缓存当次 `schema/state/i18n` 并短延时重试，直到分区可渲染。
- `i18n-runtime.js` 增加轻量 `MutationObserver`：监听新增节点与 i18n 属性变化，自动将当前语言应用到新插入的 `data-i18n`/`data-i18n-title`/`data-i18n-placeholder` 节点，从根本解决“需要手动重载才翻译”的问题。

## 文件
- `MFCMouseEffect/WebUI/app.js`
- `MFCMouseEffect/WebUI/settings-form.js`
- `MFCMouseEffect/WebUI/i18n-runtime.js`
