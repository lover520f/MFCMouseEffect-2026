# Web 设置页阶段 4.2：i18n 字典抽离（`app.js` 收敛）

## 背景
阶段 4.1 完成后，`app.js` 已去掉大量表单编排代码，但文件顶部仍内联完整中英文 i18n 大字典，导致入口文件阅读负担偏高、改文案时 diff 噪音大。

本阶段目标是把 i18n 数据从运行流程中解耦成独立资产文件，保持行为不变。

## 本次改动

### 1. 新增 i18n 独立模块
- 新文件：`MFCMouseEffect/WebUI/i18n.js`
- 结构：
  - 定义 `const I18N = { ... }`
  - 挂载 `window.MfxWebI18n = I18N`
- 职责：
  - 仅承载文案数据，不包含页面行为逻辑。

### 2. 页面加载顺序调整
- 文件：`MFCMouseEffect/WebUI/index.html`
- 变更：
  - 在 `app.js` 前新增 `<script src="/i18n.js"></script>`，保证入口脚本可直接读取全局字典。

### 3. `app.js` 精简与稳健性补强
- 文件：`MFCMouseEffect/WebUI/app.js`
- 变更：
  - 删除内联字典，改为 `const I18N = window.MfxWebI18n || {};`
  - `applyI18n/currentText/reload` 的 i18n 读取统一加空字典兜底，避免资源加载异常时直接抛错。

## 兼容性
- 不改后端 API、配置结构和字段语义。
- 中英文键名与内容保持原样，`data-i18n`/`data-i18n-title`/`data-i18n-placeholder` 机制不变。
- `MfxAutomationUi.syncI18n` 与 `MfxSectionWorkspace.syncI18n` 调用路径不变。

## 验证记录
1. `node --check` 通过：
   - `MFCMouseEffect/WebUI/i18n.js`
   - `MFCMouseEffect/WebUI/app.js`
2. `MSBuild x64 Debug` 通过，PostBuild 日志确认 `i18n.js` 已复制到 `x64/Debug/webui`。
3. 启动流程保持一致：`reload()` 能正常读取文案并完成各分区渲染。

## 后续（阶段 4.3 建议）
1. 继续拆分 `app.js` 的连接状态与 API 调用编排（在线/断线/未授权状态切换与通用错误处理）。
2. 维持“可回滚、小步提交”的节奏，每次只收敛一类职责。
