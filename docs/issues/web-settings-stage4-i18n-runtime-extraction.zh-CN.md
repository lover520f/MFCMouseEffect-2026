# Web 设置页阶段 4.4：i18n 运行时模块抽离（`app.js` 收敛）

## 背景
阶段 4.3 后，`app.js` 仍包含 i18n 应用细节（DOM 文案替换、语言选择推断、样式预设文案同步）。
为了进一步降低入口文件耦合度，本阶段把 i18n 运行时职责抽离为独立模块。

## 本次改动

### 1. 新增 i18n 运行时模块
- 新文件：`MFCMouseEffect/WebUI/i18n-runtime.js`
- 对外 API：
  - `window.MfxI18nRuntime.create(options)`
- 实例能力：
  - `pickLang()`
  - `currentText()`
  - `apply(lang)`
- 职责：
  - 统一执行 `data-i18n`、`data-i18n-title`、`data-i18n-placeholder` 的文案更新
  - 同步 `trail_style` 预设名称
  - 通过回调同步下游模块（Automation / Workspace）的 i18n 刷新

### 2. 页面接入
- 文件：`MFCMouseEffect/WebUI/index.html`
- 变更：
  - 在 `app.js` 前新增 `<script src="/i18n-runtime.js"></script>`

### 3. `app.js` 责任收敛
- 文件：`MFCMouseEffect/WebUI/app.js`
- 变更：
  - 移除内联 i18n DOM 更新细节
  - 改为创建 `i18nRuntime` 实例并调用 `apply/pickLang/currentText`
  - 保留业务编排层逻辑（状态栏、连接状态、按钮行为）不变

## 兼容性
- 不改 i18n 字典键值与文本内容。
- 不改语言切换交互与状态栏提示语义。
- 不改后端协议、配置结构与表单提交结构。

## 验证记录
1. `node --check` 通过：
   - `MFCMouseEffect/WebUI/i18n-runtime.js`
   - `MFCMouseEffect/WebUI/app.js`
2. `MSBuild x64 Debug` 通过，PostBuild 日志确认 `i18n-runtime.js` 已复制到 `x64/Debug/webui`。
3. 关键交互回归：
   - 切换 `ui_language` 后，页面主文案与下游模块文案同步更新。
   - 首次加载与 reload 后文案渲染行为保持一致。

## 后续（阶段 4.5 建议）
1. 继续拆分 `app.js` 的按钮动作处理器（reload/apply/reset/stop）到独立模块。
2. 把初始化流程（模块依赖检查 + 启动顺序）做成显式初始化器，进一步降低入口文件复杂度。
