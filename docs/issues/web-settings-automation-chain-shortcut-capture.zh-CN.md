# Web 设置页：自动化映射增强（快捷键直录 + 动作链触发）

## 问题背景
自动化映射在可用性上有两个核心痛点：
1. 快捷键录入依赖手工输入或显式录制按钮，交互不够直接。
2. 触发模型是“单动作 -> 快捷键”，无法表达连续动作链（例如 `left_click>scroll_down`）。

## 目标
1. 快捷键输入框聚焦后可直接按组合键录入，减少手工输入。
2. 映射触发升级为“链节点”模型，同时保持现有配置字段兼容。
3. 后端运行时支持链触发匹配，且不破坏旧单动作映射。

## 前端改动

### 1) 快捷键录入体验
- 文件：`MFCMouseEffect/WebUIWorkspace/src/automation/MappingPanel.svelte`
- 改动：
  - 快捷键输入框聚焦后直接捕获键盘组合（`keydown` -> `shortcutFromKeyboardEvent`）。
  - `Esc` 取消当前录入，`Backspace/Delete`（无修饰）清空快捷键。
  - 录入提示文案从静态按钮语义升级为“输入框直录”语义。

### 2) 动作链节点编辑器
- 新增：`MFCMouseEffect/WebUIWorkspace/src/automation/TriggerChainEditor.svelte`
- 新增：`MFCMouseEffect/WebUIWorkspace/src/automation/trigger-chain.js`
- 文件：`MFCMouseEffect/WebUIWorkspace/src/automation/MappingPanel.svelte`
- 改动：
  - 每条映射支持多个触发节点（首节点 + 追加节点）。
  - 通过 `添加链节点` 追加下一动作，支持节点删除。
  - 行内触发数据从单值改为 `triggerChain`（数组），保持与旧 `trigger` 文本兼容读写。

### 3) 自动化模型读写与校验
- 文件：`MFCMouseEffect/WebUIWorkspace/src/automation/model.js`
- 改动：
  - `normalize/read/evaluate` 全流程支持链触发。
  - 序列化格式：`action1>action2>...`。
  - 重复校验维度从“单 trigger”升级为“完整触发链”。

### 4) 自动化编辑器编排
- 文件：`MFCMouseEffect/WebUIWorkspace/src/automation/AutomationEditor.svelte`
- 改动：
  - 去掉旧全局录制监听流程，改为由面板输入框本地直录。
  - 模板应用与读回流程接入链触发数据结构。
  - 增补链编辑与录入提示 i18n 文案。

### 5) 快捷键解析覆盖增强
- 文件：`MFCMouseEffect/WebUIWorkspace/src/automation/shortcuts.js`
- 改动：
  - 增加符号键与 Numpad 键识别，减少“需要手输”的场景。

### 6) WebUI 文案与样式
- 文件：`MFCMouseEffect/WebUI/i18n.js`
  - 新增链编辑/录入提示文案键（中英）。
  - 自动化提示文案改为链触发格式说明。
- 文件：`MFCMouseEffect/WebUI/styles.css`
  - 增加链节点编辑器、录入提示态样式与响应式布局适配。

## 后端改动

### 1) 配置规范化支持链 trigger
- 新增：`MFCMouseEffect/MouseFx/Core/Automation/TriggerChainUtils.h`
- 文件：`MFCMouseEffect/MouseFx/Core/Config/EffectConfig.Internal.cpp`
- 改动：
  - `SanitizeInputAutomationConfig` 对 `binding.trigger` 按链 token 逐项规范化，再以 `>` 重组。
  - 兼容旧单 trigger 输入。

### 2) 运行时链匹配引擎
- 文件：`MFCMouseEffect/MouseFx/Core/Automation/InputAutomationEngine.h`
- 文件：`MFCMouseEffect/MouseFx/Core/Automation/InputAutomationEngine.cpp`
- 改动：
  - 新增鼠标/手势动作历史缓冲。
  - 按触发链进行后缀匹配，采用“最长链优先”命中策略。
  - 保留旧行为：单 trigger 仍可正常触发。

## 兼容性说明
1. 配置字段未变，仍使用 `trigger` 字符串；链式只是将其扩展为 `a>b>c` 格式。
2. 未使用链配置的用户行为不变。
3. 后端 sanitize 与匹配逻辑均向后兼容旧数据。

## 验证
1. 前端构建：
   - `pnpm run build`（`MFCMouseEffect/WebUIWorkspace`）通过。
2. C++ 编译：
   - `MSBuild.exe MFCMouseEffect.slnx /t:Build /p:Configuration=Release /p:Platform=x64` 通过。
3. 手动验证建议：
   - 在自动化映射中点击快捷键输入框，直接按组合键，应即时写入。
   - 新增 `left_click>scroll_down` 链触发并绑定快捷键，按该动作序列应命中。
   - 同链重复启用时应触发冲突提示，阻断应用。

## 回归修正（交互缺陷）
问题反馈：
1. `添加链节点` 点击无反应。
2. `录制 / 请按快捷键...` 状态切换混乱。

修正：
1. 将链编辑器自定义事件从 `change` 改为 `chainchange`（避免与内部下拉变更事件语义混淆），并在 `MappingPanel.svelte` 使用 `on:chainchange` 接收。
2. 将录制状态从“输入框焦点驱动”改为“显式录制状态机”：
   - 仅点击 `录制` 按钮进入录制态。
   - 录制成功、Esc 取消、Backspace/Delete 清空、失焦时都正确退出录制态。
   - 输入框直接按键录入仍保留（无需手工输入）。

## 二次回归修正（链节点仍不生效）
问题反馈：
1. 点击 `添加链节点` 仍无反应（界面保持单节点）。

根因：
1. 链节点变更事件在不同迁移阶段存在两种格式（数组 / 字符串）与两种事件名（`change` / `chainchange`），父层接收和标准化不一致时会被回退为单 trigger。
2. 行更新逻辑对 `triggerChain` 只做字段覆盖，没有同步标准化写回 `trigger`，容易在校验/重算后被旧值覆盖感知为“没变化”。

修正：
1. `TriggerChainEditor.svelte`：
   - 统一将链值序列化为 `a>b>c` 字符串。
   - 同时派发 `chainchange` 与 `change`，兼容旧监听路径。
2. `MappingPanel.svelte`：
   - 同时监听 `on:chainchange` 和 `on:change`。
   - 事件值进入父层前统一做 `serializeTriggerChain` 标准化，消除数组/字符串差异。
3. `AutomationEditor.svelte`：
   - 增加 `normalizeRowPatch`，对 `trigger/triggerChain` 更新时统一重建：
     - `triggerChain`（规范链数组）
     - `trigger`（序列化文本）
   - 保证编辑态、校验态、读回态使用同一份标准化结果。
4. `trigger-chain.js`：
   - `normalizeTriggerChain` 增强为可处理数组、类数组、可迭代输入，降低运行时封装对象导致的误判风险。

## 三次回归修正（事件通道收敛 + 录制双态）
问题反馈：
1. `添加链节点` 依旧“点击后无变化”。
2. 录制按钮期望明确为两个状态：`录制` 与 `结束/保存`。

根因补充：
1. 链编辑器同时发 `chainchange/change` 双事件时，迁移阶段存在事件源混杂风险，可能被非预期 `change` 分支覆盖为旧值，体感为“点击无响应”。

修正：
1. `TriggerChainEditor.svelte`：
   - 仅保留 `chainchange` 单一事件通道。
   - 点击新增/删除节点时先更新组件内链状态，再上抛标准化链值，提升交互即时反馈。
2. `MappingPanel.svelte`：
   - 仅监听 `on:chainchange`。
   - 事件解析增加多形态兜底（`detail.value/detail.chain/target.value`），空事件直接忽略，避免误覆盖。
3. `AutomationEditor.svelte` + `WebUI/i18n.js`：
   - 新增 `btn_record_stop_save` 文案键。
   - 录制按钮改为双态显示：空闲 `录制`，录制中 `结束/保存`。
