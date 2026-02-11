# Web Diag 集成 /api/state（Stage 74）

## 目标

把 `/api/state` 直接整合进 `diag=1` 页面，避免手工 fetch + token，降低排查门槛。

## 调整

1. Diag 面板新增操作
- `Dump /api/state`：立即拉取并展示最新状态 JSON
- `Copy State`：复制状态 JSON

2. Diag 面板新增状态区
- 新增 `#diagState` 只读文本框，显示格式化 JSON（`JSON.stringify(..., null, 2)`）

3. 与现有轮询联动
- 在 `diag` 轮询刷新和连接探活成功后，同步刷新 `#diagState`
- 在 `reload()` 首次加载时同步刷新 `#diagState`

4. 文案与状态提示
- 新增状态提示键：
  - `status_diag_state_loaded`
  - `status_diag_state_failed`
  - `status_diag_state_copied`
- 中英文均已补齐

## 变更文件

- `MFCMouseEffect/WebUI/index.html`
- `MFCMouseEffect/WebUI/styles.css`
- `MFCMouseEffect/WebUI/app.js`

## 备注

- 同步保持 WebUI 文件为 `CRLF`，避免 VS 行尾不一致弹窗再次出现。
