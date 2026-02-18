# WASM 设置页初始化误报：`wasm action handler unavailable`

## 现象

WASM 设置分区首次打开后，在未手动操作时就出现：
- 操作结果：`wasm action handler unavailable`

同时插件目录显示空，容易误判为接口不可用。

## 根因

`WasmPluginFields.svelte` 在组件初次挂载时会自动请求一次 `catalog`。

但在这一时刻，`wasm-main.js` 里的 `onAction` 还是默认占位函数（返回 `wasm action handler unavailable`），真实 action handler 尚未由 `app.js -> settings-form.js` 绑定完成，触发了“初始化时序竞态”。

## 修复

1. `MFCMouseEffect/WebUIWorkspace/src/entries/wasm-main.js`
- 将默认 `currentActionHandler` 从占位函数改为 `null`。

2. `MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
- 默认 `onAction` 改为 `null`。
- `runAction` 在 handler 未就绪时给出明确提示 `wasm_action_not_ready`，不再触发误导性的旧占位错误文案。

3. `MFCMouseEffect/WebUI/i18n.js`
- 新增中英文文案键：`wasm_action_not_ready`。

## 验证建议

1. 打开设置页，切到 WASM 分区，首次加载不应再自动出现 `wasm action handler unavailable`。
2. 点击“刷新插件目录”，应走真实 `/api/wasm/catalog` 接口响应。
3. 若运行时桥接 DLL 缺失，错误应聚焦为运行时回退原因（如 `Cannot load mfx_wasm_runtime.dll`），而不是 action handler 占位错误。
