# Web 设置页阶段 4.1：表单编排模块抽离（`app.js` 收敛）

## 背景
阶段 3 已把各分区表单迁移到 Svelte 渲染层，但 `app.js` 仍然保留大量表单填充/读取和 Input Indicator 动态逻辑，职责仍偏重。

本阶段先做“无协议变更”的收敛：把表单编排逻辑从 `app.js` 抽离到独立模块，`app.js` 仅保留 API 通信、状态栏反馈、连接状态管理和全局流程编排。

## 本次改动

### 1. 新增表单协调模块
- 新文件：`MFCMouseEffect/WebUI/settings-form.js`
- 对外 API（全局）：
  - `window.MfxSettingsForm.render({ schema, state, i18n })`
  - `window.MfxSettingsForm.read()`
  - `window.MfxSettingsForm.syncIndicatorPositionUi()`
- 职责：
  - 渲染 General / Effects / Text / Trail / Input Indicator 表单值
  - 读取上述分区表单值并输出提交结构
  - 管理 Input Indicator 的位置模式显示逻辑与 per-monitor overrides DOM

### 2. 页面接入顺序
- 文件：`MFCMouseEffect/WebUI/index.html`
- 变更：在 `app.js` 之前新增脚本
  - `<script src="/settings-form.js"></script>`

### 3. `app.js` 责任收敛
- 文件：`MFCMouseEffect/WebUI/app.js`
- 变更：
  - 删除本地表单工具与大段 DOM 读写函数（`fillSelect`、`num/getNum`、`setChecked/getChecked`、`buildPerMonitorUI/readPerMonitorUI` 等）
  - `reload()` 中改为调用 `MfxSettingsForm.render(...)`
  - `buildState()` 中改为调用 `MfxSettingsForm.read()` 并合并 `automation`
  - 移除 `ii_position_mode` / `ii_target_monitor` 在 `app.js` 内的直接事件绑定（改由 `settings-form.js` 统一绑定）

## 兼容性
- 不改变后端 API 和 payload 结构。
- `buildState()` 输出结构保持原样，`automation` 仍由 `MfxAutomationUi` 提供。
- Input Indicator 的动态显隐行为保持原语义。

## 验证记录
1. `node --check` 通过：
   - `MFCMouseEffect/WebUI/settings-form.js`
   - `MFCMouseEffect/WebUI/app.js`
2. `MSBuild x64 Debug` 通过，PostBuild 已复制 `settings-form.js` 到 `x64/Debug/webui`。
3. 页面产物目录包含：
   - `settings-form.js`
   - `general-settings.svelte.js`
   - `effects-settings.svelte.js`
   - `text-settings.svelte.js`
   - `trail-settings.svelte.js`
   - `input-indicator-settings.svelte.js`
   - `automation-ui.svelte.js`

## 后续（阶段 4.2+）
1. 继续把 `app.js` 中分散的状态刷新/错误处理流程做边界化（例如 form/render 与 automation/render 的统一调度接口）。
2. 评估将 i18n 字典与状态栏文案策略进一步模块化，减少 `app.js` 体积与认知负担。
