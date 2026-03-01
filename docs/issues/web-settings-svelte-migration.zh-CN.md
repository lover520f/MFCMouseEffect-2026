# Web 设置页 Svelte 渐进迁移（总核对）

## 目标
- 以“功能一项一项迁移”的方式，把 WebUI 从 legacy JS 拆到 Svelte 组件。
- 保持后端接口与配置结构稳定：`/api/schema`、`/api/state`、`/api/reload`、`/api/reset`、`/api/stop`。
- 每阶段都可独立回归，不做一次性大爆改。

## 阶段状态（截至本次）
1. 阶段 1：分区工作台（`Workspace`）迁移到 Svelte，已完成。
2. 阶段 2：Automation 编辑器迁移到 Svelte，已完成。
3. 阶段 3：General/Effects/Input/Text/Trail 表单迁移，已完成（3.1~3.5）。
4. 阶段 4：清理 legacy 入口并收敛 `app.js` 职责，已完成（4.1~4.4）。
5. 阶段 5：页面壳层与弹窗层迁移到 Svelte，已完成（5.1~5.3）。
6. 阶段 6：分区动态行为迁移到 Svelte 状态层，进行中（6.1~6.2 已完成）。

---

## 阶段 1（已完成）
- 范围：专注/全部视图切换、分区导航、摘要、hash 与视图模式持久化。
- 源码：
  - `MFCMouseEffect/WebUIWorkspace/src/WorkspaceSidebar.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/entries/main.js`
- 页面接入：
  - `MFCMouseEffect/WebUI/index.html` 加载 `section-workspace.svelte.js`

### 阶段 1 核对点
1. 默认进入专注视图，仅显示当前分区。
2. 切换“全部分区”后恢复多栏布局，不是单列堆叠。
3. 切换分区后 hash 同步，刷新后仍定位到该分区。
4. 视图模式刷新后保持。
5. 中英文切换后侧栏文案和摘要同步。

---

## 阶段 2（已完成）
- 范围：鼠标映射/手势映射列表、增删、录制、冲突校验、模板应用。
- 架构：
  - 保留 `window.MfxAutomationUi` 兼容 API（`render/read/validate/syncI18n`）。
  - legacy `MFCMouseEffect/WebUI/automation-ui.js` 已移除。
  - 新增 Svelte 模块并拆分逻辑文件：
    - `MFCMouseEffect/WebUIWorkspace/src/automation/AutomationEditor.svelte`
    - `MFCMouseEffect/WebUIWorkspace/src/automation/MappingPanel.svelte`
    - `MFCMouseEffect/WebUIWorkspace/src/automation/model.js`
    - `MFCMouseEffect/WebUIWorkspace/src/automation/shortcuts.js`
    - `MFCMouseEffect/WebUIWorkspace/src/automation/defaults.js`
    - `MFCMouseEffect/WebUIWorkspace/src/automation/api.js`
    - `MFCMouseEffect/WebUIWorkspace/src/entries/automation-main.js`
- 页面接入：
  - `MFCMouseEffect/WebUI/index.html` 自动化分区改为 `automation_editor_mount` 挂载点。
  - 页面脚本改为加载 `automation-ui.svelte.js`。

### 阶段 2 核对点
1. 自动化总开关、手势开关、手势参数可读写并随 Apply 保存。
2. 鼠标映射/手势映射支持新增、删除、启用禁用。
3. 录制按钮仅在捕获有效主键时写入组合键（如 `Ctrl+Shift+S`）。
4. 启用映射但快捷键为空时，行内提示并阻断 Apply。
5. 同一列表内启用重复 trigger 时，行内冲突提示并阻断 Apply。
6. 模板应用为 upsert（同 trigger 更新，不重复新增）。
7. 中英文切换后，动态按钮/空态/校验文案同步更新。

---

## 阶段 3（已完成）
- 范围：General / Effects / Input Indicator / Text / Trail 表单迁移到 Svelte。
- 兼容要求：`buildState()` 输出结构不变，后端无感知。

### 阶段 3.1（已完成）：General 表单迁移到 Svelte
- 迁移范围：语言、主题、长按跟随模式、长按 presenter backend 下拉字段渲染。
- 兼容策略：继续复用 `app.js` 的 `fillSelect`、`buildState`、`applyI18n`，保持字段 ID 不变。
- 文件：
  - `MFCMouseEffect/WebUIWorkspace/src/general/GeneralSettingsFields.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/entries/general-main.js`
  - `MFCMouseEffect/WebUI/index.html`
  - `MFCMouseEffect/WebUIWorkspace/vite.config.js`
  - `MFCMouseEffect/WebUIWorkspace/package.json`
  - `MFCMouseEffect/WebUIWorkspace/scripts/copy-output.mjs`

#### 阶段 3.1 核对点
1. reload 后 `ui_language/theme/hold_follow_mode/hold_presenter_backend` 选项与选中值正常回填。
2. Apply 后 `buildState()` 仍按原结构提交上述字段。
3. 切换语言后 General 分区 label 与 tooltip 文案正常更新。
4. 不影响分区导航/焦点视图/全分区模式。

### 阶段 3.2（已完成）：Active Effects 表单迁移到 Svelte
- 迁移范围：`click/trail/scroll/hold/hover` 五个特效通道下拉字段渲染。
- 兼容策略：保持字段 ID 与 legacy 完全一致，继续由 `app.js` 负责填充与提交。
- 文件：
  - `MFCMouseEffect/WebUIWorkspace/src/effects/ActiveEffectsFields.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/entries/effects-main.js`
  - `MFCMouseEffect/WebUI/index.html`
  - `MFCMouseEffect/WebUIWorkspace/vite.config.js`
  - `MFCMouseEffect/WebUIWorkspace/package.json`
  - `MFCMouseEffect/WebUIWorkspace/scripts/copy-output.mjs`

#### 阶段 3.2 核对点
1. reload 后 `click/trail/scroll/hold/hover` 下拉项与选中值正常回填。
2. Apply 后 `buildState().active` 结构与字段名保持不变。
3. 切换语言后 Effects 分区 label 文案同步更新。
4. 不影响 General 与 Automation 已迁移分区。

### 阶段 3.3（已完成）：Text Content 表单迁移到 Svelte
- 迁移范围：`text_font_size` 与 `text_content` 两个字段渲染。
- 兼容策略：保持字段 ID 与 placeholder i18n 键不变，继续复用 `app.js` 的读写逻辑。
- 文件：
  - `MFCMouseEffect/WebUIWorkspace/src/text/TextContentFields.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/entries/text-main.js`
  - `MFCMouseEffect/WebUI/index.html`
  - `MFCMouseEffect/WebUIWorkspace/vite.config.js`
  - `MFCMouseEffect/WebUIWorkspace/package.json`
  - `MFCMouseEffect/WebUIWorkspace/scripts/copy-output.mjs`

#### 阶段 3.3 核对点
1. reload 后 `text_font_size` 与 `text_content` 数值/文本正常回填。
2. Apply 后 `buildState().text_font_size` 与 `buildState().text_content` 结构不变。
3. 切换语言后 label、placeholder、hint 文案正常同步。
4. 不影响 General/Effects/Automation 已迁移分区。

### 阶段 3.4（已完成）：Trail Tuning 表单迁移到 Svelte
- 迁移范围：`trail_style`、`idle fade`、五组 profile 与三组 trail params 字段渲染。
- 兼容策略：保持所有输入 ID 与 legacy 完全一致，继续由 `app.js` 负责回填与提交。
- 文件：
  - `MFCMouseEffect/WebUIWorkspace/src/trail/TrailTuningFields.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/entries/trail-main.js`
  - `MFCMouseEffect/WebUI/index.html`
  - `MFCMouseEffect/WebUIWorkspace/vite.config.js`
  - `MFCMouseEffect/WebUIWorkspace/package.json`
  - `MFCMouseEffect/WebUIWorkspace/scripts/copy-output.mjs`

#### 阶段 3.4 核对点
1. reload 后 `trail_style` 与全部 trail 数值字段正常回填。
2. Apply 后 `buildState().trail_profiles` 与 `buildState().trail_params` 结构不变。
3. 切换语言后 Trail 分区标签与提示文案同步更新。
4. 不影响 General/Effects/Text/Automation 已迁移分区。

### 阶段 3.5（已完成）：Input Indicator 表单迁移到 Svelte
- 迁移范围：`ii_*` 字段、目标屏幕选择与 per-monitor 容器渲染。
- 兼容策略：保持 `ii_enabled/ii_keyboard_enabled/ii_position_mode/ii_target_monitor` 等全部字段 ID 不变，继续由 `app.js` 负责动态行为（位置模式切换、per-monitor UI 生成、提交）。
- 文件：
  - `MFCMouseEffect/WebUIWorkspace/src/input-indicator/InputIndicatorFields.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/entries/input-indicator-main.js`
  - `MFCMouseEffect/WebUI/index.html`
  - `MFCMouseEffect/WebUIWorkspace/vite.config.js`
  - `MFCMouseEffect/WebUIWorkspace/package.json`
  - `MFCMouseEffect/WebUIWorkspace/scripts/copy-output.mjs`

#### 阶段 3.5 核对点
1. reload 后 `ii_*` 字段、`ii_target_monitor`、`ii_key_display_mode` 正常回填。
2. `ii_position_mode` / `ii_target_monitor` 变更仍能触发既有 `syncIndicatorPositionUi()` 行为。
3. per-monitor 区域仍由 `buildPerMonitorUI()` 生成并可被 `readPerMonitorUI()` 正确读取。
4. Apply 后 `buildState().input_indicator` 结构不变。

## 阶段 4（已完成）
- 范围：移除残余 legacy DOM 绑定，收敛重复状态同步路径。
- 结束标准：`app.js` 仅保留 API 通信与全局编排职责。

### 阶段 4.1（已完成）：表单协调模块抽离
- 迁移范围：把 General/Effects/Text/Trail/Input Indicator 的表单填充/读取/动态行为从 `app.js` 抽离到独立模块。
- 新模块：
  - `MFCMouseEffect/WebUI/settings-form.js`
- 页面接入：
  - `MFCMouseEffect/WebUI/index.html` 在 `app.js` 前加载 `settings-form.js`
- `app.js` 变更：
  - `reload()` 改为委托 `MfxSettingsForm.render({ schema, state, i18n })`
  - `buildState()` 改为委托 `MfxSettingsForm.read()` 后合并 automation
  - 删除重复的表单工具函数与 indicator 事件绑定逻辑
- 详细记录：已收敛到本文件（阶段子文档已清理）。

### 阶段 4.2（已完成）：i18n 字典抽离
- 迁移范围：将 `app.js` 内联的中英文 i18n 字典整体抽离到独立脚本，降低入口文件体积与编辑噪音。
- 新模块：
  - `MFCMouseEffect/WebUI/i18n.js`
- 页面接入：
  - `MFCMouseEffect/WebUI/index.html` 在 `app.js` 前加载 `i18n.js`
- `app.js` 变更：
  - 用 `window.MfxWebI18n` 取代内联字典定义
  - 为 i18n 读取补充空字典兜底（避免脚本加载异常时崩溃）
- 详细记录：已收敛到本文件（阶段子文档已清理）。

#### 阶段 4.2 核对点
1. 页面加载后中英文文案与阶段 4.1 前一致，无回归。
2. 切换 `ui_language` 后，分区标题、tooltip、placeholder 和样式预设文案仍同步更新。
3. `node --check` 对 `i18n.js` 与 `app.js` 均通过。
4. `MSBuild x64 Debug` 后，`i18n.js` 已随 WebUI 产物复制到运行目录。

### 阶段 4.3（已完成）：网络访问与健康检查模块抽离
- 迁移范围：将 `app.js` 内联的 API 请求与连接心跳逻辑迁移到独立 WebUI 模块。
- 新模块：
  - `MFCMouseEffect/WebUI/web-api.js`
- 页面接入：
  - `MFCMouseEffect/WebUI/index.html` 在 `app.js` 前加载 `web-api.js`
- `app.js` 变更：
  - 删除内联 `apiGet/apiPost/probeConnection/startHealthCheck` 细节
  - 改为通过 `MfxWebApi.create(...)` 注入 `token` 与回调（未授权、连接状态切换）
- 详细记录：已收敛到本文件（阶段子文档已清理）。

#### 阶段 4.3 核对点
1. reload/apply/reset/stop 流程与阶段 4.2 前一致。
2. `401` 仍触发未授权状态提示并阻断后续操作。
3. 在线/断线心跳探测与状态栏反馈行为保持一致。
4. `node --check` 对 `web-api.js` 与 `app.js` 均通过，`MSBuild x64 Debug` 通过。

### 阶段 4.4（已完成）：i18n 运行时模块抽离
- 迁移范围：将 `app.js` 中 i18n 的应用逻辑（文案替换、placeholder/title 同步、样式预设文案刷新）抽离为独立运行时模块。
- 新模块：
  - `MFCMouseEffect/WebUI/i18n-runtime.js`
- 页面接入：
  - `MFCMouseEffect/WebUI/index.html` 在 `app.js` 前加载 `i18n-runtime.js`
- `app.js` 变更：
  - 改为通过 `MfxI18nRuntime.create(...)` 创建 i18n 运行时
  - `applyI18n/pickLang/currentText` 改为调用运行时实例
  - `AutomationUi/SectionWorkspace` 的 i18n 同步改为通过运行时回调注入
- 详细记录：已收敛到本文件（阶段子文档已清理）。

#### 阶段 4.4 核对点
1. 中英文切换后，页面文案、tooltip、placeholder 与样式预设名称保持一致更新。
2. `Automation` 与 `Workspace` 的联动文案刷新行为保持不变。
3. `node --check` 对 `i18n-runtime.js` 与 `app.js` 均通过。
4. `MSBuild x64 Debug` 通过并确认 `i18n-runtime.js` 随 WebUI 产物复制。

## 阶段 5（已完成）
- 范围：把 `index.html` 中仍为手写静态 DOM 的壳层区域迁移到 Svelte，继续减少 legacy HTML 体积。
- 兼容要求：保留既有 DOM id/data-i18n 语义，不破坏 `app.js` 与各分区 bundle 的挂载点契约。

### 阶段 5.1（已完成）：页面壳层迁移到 Svelte（JS-only）
- 迁移范围：背景层、状态条、顶部品牌与按钮区、主设置网格卡片容器。
- 新增组件与入口（全部 JS，不使用 TS）：
  - `MFCMouseEffect/WebUIWorkspace/src/shell/WebSettingsShell.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/shell/TopBar.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/shell/SettingsGrid.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/shell/sections.js`
  - `MFCMouseEffect/WebUIWorkspace/src/entries/shell-main.js`
- 构建链更新：
  - `MFCMouseEffect/WebUIWorkspace/vite.config.js` 新增 `shell` 模式
  - `MFCMouseEffect/WebUIWorkspace/package.json` 新增 `build:shell` 并纳入总构建
  - `MFCMouseEffect/WebUIWorkspace/scripts/copy-output.mjs` 新增 `settings-shell.svelte.js` 拷贝目标
- 页面接入：
  - `MFCMouseEffect/WebUI/index.html` 简化为 `web_settings_shell_mount` 挂载点
  - 增加脚本 `settings-shell.svelte.js`，并确保在各分区 bundle 前加载
- 详细记录：已收敛到本文件（阶段子文档已清理）。

#### 阶段 5.1 核对点
1. `status`、`btnReload/btnReset/btnStop/btnSave`、`workspace_sidebar_mount` 等关键 DOM id 保持不变。
2. 各分区 mount 点（`general_settings_mount` 等）保持不变，原有 Svelte 分区仍可正常挂载。
3. `data-i18n` / `data-i18n-title` / `data-i18n-placeholder` 语义保持不变，语言切换无回归。
4. `pnpm run build` 成功生成并复制 `settings-shell.svelte.js`。

### 阶段 5.2（已完成）：Dialog 迁移到 Svelte（JS-only）
- 迁移范围：将原 `dialog.js` 主题弹窗迁移为 Svelte 组件实现，并保持 `window.MfxDialog` API 兼容。
- 新增组件与入口（全部 JS，不使用 TS）：
  - `MFCMouseEffect/WebUIWorkspace/src/dialog/DialogHost.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/entries/dialog-main.js`
- 构建链更新：
  - `MFCMouseEffect/WebUIWorkspace/vite.config.js` 新增 `dialog` 模式
  - `MFCMouseEffect/WebUIWorkspace/package.json` 新增 `build:dialog` 并纳入总构建
  - `MFCMouseEffect/WebUIWorkspace/scripts/copy-output.mjs` 新增 `dialog.svelte.js` 拷贝目标
- 页面接入：
  - `MFCMouseEffect/WebUI/index.html` 脚本由 `dialog.js` 切换到 `dialog.svelte.js`
  - 删除 legacy 文件：`MFCMouseEffect/WebUI/dialog.js`
- 详细记录：已收敛到本文件（阶段子文档已清理）。

#### 阶段 5.2 核对点
1. `window.MfxDialog.showNotice/showConfirm` API 保持可用。
2. `app.js` 里的断连提示与重置确认弹窗行为保持一致。
3. `pnpm run build` 生成并复制 `dialog.svelte.js`。
4. `MSBuild x64 Debug` 后，运行目录包含 `dialog.svelte.js`。

### 阶段 5.3（已完成）：壳层交互桥接到 Svelte 状态层（JS-only）
- 迁移范围：将顶部按钮交互与状态横幅显示从 `app.js` 的直接 DOM 操作切换为 Svelte 壳层状态驱动。
- 组件与入口变更（全部 JS，不使用 TS）：
  - `MFCMouseEffect/WebUIWorkspace/src/shell/TopBar.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/shell/WebSettingsShell.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/entries/shell-main.js`
- 核心机制：
  - `TopBar.svelte` 改为派发 `reload/save/reset/stop` 动作事件，并接受 `actionsDisabled`。
  - `WebSettingsShell.svelte` 接管状态横幅展示（`statusMessage/statusTone`）。
  - `shell-main.js` 暴露全局桥接 API：
    - `window.MfxWebShell.onAction(listener)`
    - `window.MfxWebShell.setStatus(message, tone)`
    - `window.MfxWebShell.setActionsEnabled(enabled)`
- `app.js` 变更：
  - 优先通过 `MfxWebShell` 绑定动作和更新状态/按钮可用性
  - 保留 legacy DOM fallback，确保降级兼容
- 详细记录：已收敛到本文件（阶段子文档已清理）。

#### 阶段 5.3 核对点
1. 顶部按钮动作（重载/应用/恢复默认/关闭监听）行为与 5.2 前一致。
2. 状态横幅文案与 tone 切换（`warn/ok/offline`）行为一致。
3. `setActionButtonsEnabled` 仍能正确控制按钮可点击状态。
4. `pnpm run build` 和 `MSBuild x64 Debug` 均通过。

---

## 阶段 6（进行中）
- 范围：把仍依赖 legacy DOM 组装/读取的分区动态行为迁移到 Svelte 组件内部状态层。
- 兼容要求：保持 `buildState().input_indicator` 结构不变，保留现有 DOM id 语义与 fallback 路径。

### 阶段 6.1（已完成）：Input Indicator 状态化迁移到 Svelte（JS-only）
- 迁移范围：
  - `ii_*` 字段改为组件内双向绑定状态驱动。
  - per-monitor overrides 从 `settings-form.js` 动态 DOM 拼装迁移到 `InputIndicatorFields.svelte` 内部渲染与读取。
  - 新增 `window.MfxInputIndicatorSection` 桥接 API，供 `settings-form.js` 统一委托渲染/读取。
- 关键文件：
  - `MFCMouseEffect/WebUIWorkspace/src/input-indicator/InputIndicatorFields.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/entries/input-indicator-main.js`
  - `MFCMouseEffect/WebUI/settings-form.js`
  - `MFCMouseEffect/WebUI/input-indicator-settings.svelte.js`
- 详细记录：已收敛到本文件（阶段子文档已清理）。

#### 阶段 6.1 核对点
1. reload 后 `ii_*` 字段和 per-monitor overrides 仍按配置正确回填。
2. Apply 后 `buildState().input_indicator` JSON 结构与字段名保持不变。
3. `settings-form.js` 在 Svelte API 可用时优先走状态化读写，在缺失时仍可走 legacy fallback。
4. `node --check`、`pnpm run build`、`MSBuild x64 Debug` 全部通过。

### 阶段 6.2（已完成）：General/Effects/Text/Trail 状态化读写迁移到 Svelte（JS-only）
- 迁移范围：
  - `General`、`Active Effects`、`Text`、`Trail` 四个分区从“静态字段 + `settings-form.js` 直接 DOM 读写”迁移为“组件状态驱动 + 全局桥接 API”。
  - `settings-form.js` 对四个分区新增优先委托路径，减少 legacy DOM 直接耦合。
- 关键文件：
  - `MFCMouseEffect/WebUIWorkspace/src/general/GeneralSettingsFields.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/effects/ActiveEffectsFields.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/text/TextContentFields.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/trail/TrailTuningFields.svelte`
  - `MFCMouseEffect/WebUIWorkspace/src/entries/general-main.js`
  - `MFCMouseEffect/WebUIWorkspace/src/entries/effects-main.js`
  - `MFCMouseEffect/WebUIWorkspace/src/entries/text-main.js`
  - `MFCMouseEffect/WebUIWorkspace/src/entries/trail-main.js`
  - `MFCMouseEffect/WebUI/settings-form.js`
  - 对应 bundle：`general-settings.svelte.js`、`effects-settings.svelte.js`、`text-settings.svelte.js`、`trail-settings.svelte.js`
- 新增桥接 API：
  - `window.MfxGeneralSection.render/read`
  - `window.MfxEffectsSection.render/read`
  - `window.MfxTextSection.render/read`
  - `window.MfxTrailSection.render/read`
- 详细记录：已收敛到本文件（阶段子文档已清理）。

#### 阶段 6.2 核对点
1. reload 后四个分区字段按 `/api/state` 正确回填。
2. Apply 后 `buildState()` 的 `ui/theme/hold_*`、`active`、`text_*`、`trail_*` 结构保持不变。
3. `settings-form.js` 在 API 可用时优先走分区桥接，API 缺失时仍可 fallback 到 legacy DOM。
4. `node --check`、`pnpm run build`、`MSBuild x64 Debug` 全部通过。

---

## 构建命令
在 `MFCMouseEffect/WebUIWorkspace` 执行：

```bash
pnpm install
pnpm run build
```

构建会复制以下产物到 `MFCMouseEffect/WebUI`：
- `dialog.svelte.js`
- `settings-shell.svelte.js`
- `section-workspace.svelte.js`
- `general-settings.svelte.js`
- `effects-settings.svelte.js`
- `text-settings.svelte.js`
- `trail-settings.svelte.js`
- `input-indicator-settings.svelte.js`
- `automation-ui.svelte.js`

---

## 风险与回退
- 风险：迁移阶段中，组件状态与 legacy DOM 语义可能偏差。
- 回退方式：
  1. 每阶段独立提交，可按提交粒度回退。
  2. 保留历史提交可快速回到 legacy JS 版本。

