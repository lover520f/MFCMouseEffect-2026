# Web 设置页：自动化映射编辑器（鼠标 Action / 手势 -> 键盘）

## 背景
此前自动化能力（`automation` 配置）已具备后端与运行时链路，但 Web 设置页没有对应的可视化入口，用户需要手改 `config.json`。

本次在不破坏既有设置架构的前提下，将自动化映射能力正式接入 WebUI。

## 本次改动

### 1. 新增独立前端模块（避免 `app.js` 继续膨胀）
- 新增文件：`MFCMouseEffect/WebUI/automation-ui.js`
- 责任：
  - 渲染鼠标映射/手势映射列表
  - 新增/删除映射项
  - 读取映射状态回传给 `buildState()`
  - 跟随语言切换同步动态文案

### 2. 设置页新增自动化分区
- 文件：`MFCMouseEffect/WebUI/index.html`
- 新增导航锚点：`#automation`
- 新增配置项：
  - 自动化总开关
  - 鼠标 action 映射列表
  - 手势映射开关
  - 手势触发键（left/middle/right）
  - 手势识别参数（最小距离、采样步长、最大方向段）
  - 手势映射列表

### 3. `app.js` 只保留编排职责
- 文件：`MFCMouseEffect/WebUI/app.js`
- 主要变更：
  - i18n 新增自动化分区文案（中/英）
  - `reload()` 中调用 `MfxAutomationUi.render(...)`
  - `buildState()` 中合并 `MfxAutomationUi.read()` 的结果到 `automation`
  - `applyI18n()` 中调用 `MfxAutomationUi.syncI18n(...)`

### 4. 样式补齐
- 文件：`MFCMouseEffect/WebUI/styles.css`
- 新增：
  - `card-automation` 区块布局
  - 自动化映射列表行样式（启用态/禁用态）
  - 空状态提示样式
  - 小屏布局规则

## 兼容性
- 未改动任何后端 API 路由协议。
- `automation` 字段结构保持既有定义。
- 不会影响未启用自动化的用户行为。

## 验证点
1. 打开设置页可见“自动化映射”分区和导航入口。
2. 新增鼠标映射并应用，`/api/state` 回写后仍存在。
3. 新增手势映射并应用，参数可持久化。
4. 切换中/英文后，自动化区域动态按钮与空状态文案同步变化。
5. 构建后输出目录 `x64/Debug/webui` 包含 `automation-ui.js`。

---

## 增量改动（阶段 2：可用性与防误操作）

### 1. 快捷键录制（Record）
- 文件：`MFCMouseEffect/WebUI/automation-ui.js`
- 改动：
  - 每条映射新增 `Record` 按钮，进入录制态后捕获一次键盘事件并自动写入快捷键输入框。
  - 录制期间支持中断收敛（再次点击、点击行外、切换/重渲染时自动退出录制态）。
  - 录制仅在捕获到“主键”后生效（不接受仅 `Ctrl/Shift/Alt/Win` 的误录入）。
  - 统一输出组合键文本（如 `Ctrl+Shift+S`、`Win+R`、`F5`）。

### 2. 行内校验与冲突高亮
- 文件：`MFCMouseEffect/WebUI/automation-ui.js`、`MFCMouseEffect/WebUI/styles.css`
- 改动：
  - 启用状态下未填写快捷键：行内提示并标记冲突样式。
  - 同一类型（mouse/gesture）中触发项重复且都启用：所有冲突行提示重复触发。
  - 新增样式：冲突边框/背景、提示文案区、录制态按钮样式。

### 3. Apply 前置校验（阻断无效提交）
- 文件：`MFCMouseEffect/WebUI/app.js`
- 改动：
  - `btnSave` 触发时先调用 `MfxAutomationUi.validate()`。
  - 若校验失败，直接在状态栏给出本地化错误提示并阻断 `/api/state` 提交。
  - 校验阻断遵循开关状态：仅在自动化总开关开启时校验鼠标映射；仅在手势开关开启时校验手势映射。
  - 避免“前端看起来可编辑，但后端拿到无效映射”的配置污染。

### 4. i18n 文案补齐
- 文件：`MFCMouseEffect/WebUI/app.js`
- 新增中英文键：
  - `btn_record_shortcut`、`btn_recording`
  - `auto_missing_shortcut`、`auto_conflict_trigger`
  - `auto_validation_missing_shortcut`
  - `auto_validation_mouse_duplicate`
  - `auto_validation_gesture_duplicate`

## 阶段 2 验证建议
1. 新增映射后点击 `Record`，按组合键，输入框应自动填充并退出录制态。
2. 启用映射但清空快捷键，行内应出现错误提示，点击“应用”应被阻断。
3. 同一列表内启用两个相同 trigger，冲突行应高亮，点击“应用”应被阻断。
4. 切换中/英文后，录制按钮、校验提示、阻断提示文案应同步切换。

---

## 增量改动（阶段 3：快速模板）

### 1. 模板模块拆分（保持职责单一）
- 新增文件：`MFCMouseEffect/WebUI/automation-templates.js`
- 责任：
  - 维护鼠标/手势映射模板目录
  - 提供模板列表（用于下拉框）
  - 提供模板绑定集合（用于应用）

### 2. 设置页模板入口
- 文件：`MFCMouseEffect/WebUI/index.html`
- 改动：
  - 鼠标映射区新增模板下拉框和“应用模板”按钮
  - 手势映射区新增模板下拉框和“应用模板”按钮
  - 引入脚本顺序：`automation-templates.js` -> `automation-ui.js` -> `app.js`

### 3. 模板应用逻辑（upsert，不破坏已有映射）
- 文件：`MFCMouseEffect/WebUI/automation-ui.js`
- 改动：
  - 新增模板下拉填充（支持中英文动态切换）
  - 新增模板应用逻辑：同 trigger 先更新，缺失 trigger 才新增（upsert）
  - 自动过滤 schema 中不存在的 trigger，避免注入无效项

### 4. 文案与样式
- 文件：`MFCMouseEffect/WebUI/app.js`、`MFCMouseEffect/WebUI/styles.css`
- 改动：
  - 新增模板相关 i18n 键（中/英）
  - 新增 `automation-actions`、模板下拉样式及移动端适配

## 阶段 3 验证建议
1. 鼠标模板选择“浏览器标签”，点击“应用模板”后应生成/更新 `scroll_up`、`scroll_down`、`middle_click` 映射。
2. 手势模板选择“窗口快速排版”，点击“应用模板”后应生成/更新 `left/right/up/down` 映射。
3. 若已有同 trigger 映射，模板应用应更新该行而不是新增重复行。
4. 切换语言后，模板下拉项和按钮文案应即时切换。

---

## 增量改动（阶段 4：编辑器迁移到 Svelte）

### 1. 保持 API 兼容，替换底层实现
- legacy 文件：`MFCMouseEffect/WebUI/automation-ui.js`（已移除）
- 新实现产物：`MFCMouseEffect/WebUI/automation-ui.svelte.js`
- 对外仍使用：`window.MfxAutomationUi`
  - `render(payload)`
  - `read()`
  - `validate()`
  - `syncI18n(i18n)`

### 2. 组件与逻辑拆分（降低耦合）
- 组件：`MFCMouseEffect/WebUIWorkspace/src/automation/AutomationEditor.svelte`
- 列表子组件：`MFCMouseEffect/WebUIWorkspace/src/automation/MappingPanel.svelte`
- 录制逻辑：`MFCMouseEffect/WebUIWorkspace/src/automation/shortcuts.js`
- 数据归一化/校验/模板 upsert：`MFCMouseEffect/WebUIWorkspace/src/automation/model.js`
- 默认值定义：`MFCMouseEffect/WebUIWorkspace/src/automation/defaults.js`
- API 适配层：`MFCMouseEffect/WebUIWorkspace/src/automation/api.js`
- 入口：`MFCMouseEffect/WebUIWorkspace/src/entries/automation-main.js`

### 3. 页面接入
- `MFCMouseEffect/WebUI/index.html`
  - 自动化分区改为 `automation_editor_mount` 挂载点。
  - 脚本从 `automation-ui.js` 切换为 `automation-ui.svelte.js`。

### 4. 构建输出
- `MFCMouseEffect/WebUIWorkspace/package.json`
  - `pnpm run build` 先后构建 workspace 与 automation 两个入口。
- `MFCMouseEffect/WebUIWorkspace/vite.config.js`
  - 通过 `--mode workspace|automation` 输出不同 bundle。
- `MFCMouseEffect/WebUIWorkspace/scripts/copy-output.mjs`
  - 复制 `section-workspace.svelte.js` 与 `automation-ui.svelte.js` 到 `MFCMouseEffect/WebUI`。

## 阶段 4 验证建议
1. reload 后自动化分区能读取已有配置，不丢失鼠标/手势映射。
2. 录制、冲突校验、模板应用行为与旧版一致。
3. Apply 前置校验仍生效（空快捷键/重复 trigger 时阻断）。
4. 切换中英文后，自动化分区动态文案即时更新。


