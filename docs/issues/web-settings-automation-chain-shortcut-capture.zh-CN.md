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

## 四次回归修正（手动输入与自动识别分离）
问题反馈：
1. 快捷键输入框手动输入时会被自动识别逻辑接管，不符合手动编辑预期。
2. 录制状态按下已有页面快捷键（如刷新/导航类）可能被浏览器先处理，导致录制失败。

修正：
1. `MappingPanel.svelte`：
   - 将快捷键输入拆为两种模式：
     - 手动模式（默认）：输入框按普通文本编辑，`on:input` 直接回写字符串。
     - 自动识别模式（点击 `录制` 后）：仅在该状态下解析 `keydown -> shortcut`。
   - 录制状态下对按键统一 `preventDefault + stopPropagation`，优先拦截浏览器快捷键，避免抢占导致录制失败。
   - 录制状态输入框切为 `readonly`，防止手动输入和自动识别混杂。
2. `WebUI/i18n.js`：
   - 更新中英文提示文案，明确“手动输入”与“自动识别”是两条路径，并说明录制时会拦截页面快捷键。

## 五次回归修正（系统热键冲突导致录制失败）
问题反馈：
1. 当系统或其他应用已占用快捷键（例如 `Alt+A`）时，Web 输入框录制会先触发外部热键，前端拿不到 `keydown`。

根因：
1. 旧实现仅依赖网页层事件捕获；遇到系统级/全局热键抢占时，浏览器层不具备可靠捕获能力。

修正：
1. C++ 后端新增“原生快捷键录制会话”能力（基于已存在的 `WH_KEYBOARD_LL` 键盘钩子）：
   - 新增 `ShortcutCaptureSession`，提供 `start/poll/stop` 会话接口。
   - 录制期间由全局键盘事件直接产出标准快捷键文本（如 `Alt+A`），不再依赖网页 `keydown` 是否能收到。
2. Web API 新增路由：
   - `POST /api/automation/shortcut-capture/start`
   - `POST /api/automation/shortcut-capture/poll`
   - `POST /api/automation/shortcut-capture/stop`
3. 前端 `MappingPanel.svelte` 录制流程改为：
   - 点击 `录制` 后优先启动原生会话并轮询结果。
   - 若原生会话不可用，自动回退到现有网页 `keydown` 捕获。
4. `DispatchRouter` 键盘消息分发改造：
   - 键盘事件统一先进入 `AppController::OnGlobalKey`，同时服务于“输入指示器显示”和“快捷键录制会话”，避免两套逻辑割裂。

验证要点：
1. 在录制状态下按 `Alt+A`（即使该组合在系统中有既有行为），应在映射行里稳定写入 `Alt+A`。
2. 手动输入模式仍可直接编辑文本，不受原生录制流程干扰。

## 六次回归修正（设置页下拉选项空白）
问题反馈：
1. 一般、特效、键鼠指示器等区域的下拉框显示为空，选项无法加载。
2. 点击“恢复默认”后提示 `Reset failed: Cannot write to private field`。

根因：
1. `reload()` 流程中 `applyI18n()` 会调用 `syncConsumers`，其中任一消费者抛异常（本次为运行时 `Cannot write to private field`）会中断整个 `reload()`。
2. `reload()` 被中断后，`settingsForm.render(...)` 后续流程未完成，页面停留在初始空选项状态。

修正：
1. `WebUI/app.js`：为 i18n consumer 同步增加隔离保护（逐个 `try/catch`），任何单个消费者异常都不会阻断主流程。
2. `WebUI/i18n-runtime.js`：对 `syncConsumers(text)` 增加兜底 `try/catch`，确保 `applyI18n` 不因消费者异常失败。
3. 保持 `settingsForm` 作为分区渲染/读回的单一入口，避免重复渲染链引入额外耦合风险。
4. `reload()` 增加 i18n 应用防护：即使 `applyI18n` 或自动化分区 `syncI18n` 抛错，也不中断 schema/state 渲染链，确保下拉选项可继续加载。

验证要点：
1. 打开设置页后，一般/特效/键鼠指示器下拉应正常出现选项。
2. 点击“恢复默认”后不再出现 `Cannot write to private field`，并可自动刷新配置。

## 七次回归修正（Debug 下拉仍为空：分区挂载时序竞争）
问题反馈：
1. Debug 环境中下拉框仍然为空，且多个分区同时出现“初始空选项”状态。

根因：
1. `settings-shell` 负责渲染分区挂载节点（如 `general_settings_mount`），各分区入口脚本在初始化时立即 `getElementById(...)`。
2. 在部分加载时序下，分区入口执行时挂载节点尚未就绪，组件实例创建失败后退化为空渲染分支，后续即使节点出现也不会自动恢复，导致下拉长期保持空数组。

修正：
1. 新增：`MFCMouseEffect/WebUIWorkspace/src/entries/lazy-mount.js`
   - 提供统一“延迟挂载桥接”能力：缓存最新 props、节点出现后自动挂载、挂载后自动 `$set`。
2. 接入分区入口：
   - `MFCMouseEffect/WebUIWorkspace/src/entries/general-main.js`
   - `MFCMouseEffect/WebUIWorkspace/src/entries/effects-main.js`
   - `MFCMouseEffect/WebUIWorkspace/src/entries/input-indicator-main.js`
   - `MFCMouseEffect/WebUIWorkspace/src/entries/text-main.js`
   - `MFCMouseEffect/WebUIWorkspace/src/entries/trail-main.js`
   - `MFCMouseEffect/WebUIWorkspace/src/automation/api.js`（自动化分区同样使用延迟挂载观察，避免同类时序问题）
3. 取消“未挂载即永久 no-op”的导出方式，改为可恢复的懒挂载渲染路径。

验证要点：
1. 首次打开设置页（无需手动重载）时，各分区下拉立即出现选项。
2. Debug/Release 环境下行为一致，不再依赖脚本加载先后顺序。

## 八次回归修正（首轮加载失败后不自动恢复）
问题反馈：
1. Debug 环境仍偶发全部下拉空白，重开后有时恢复、有时不恢复。

根因：
1. `app.js` 在页面启动时只做一次 `reload()`。
2. 若这一轮请求恰好失败（例如服务刚启动、短暂离线、端口握手未稳定），后续健康检查仅更新连接状态，不会重新拉取 schema/state。
3. 结果是页面停留在“组件初始空选项”状态，用户看到所有下拉为空。

修正：
1. `MFCMouseEffect/WebUI/app.js`
   - 增加加载状态机：`hasRenderedSettings`、`isReloading`、`reloadRetryTimer`。
   - 首轮加载失败后自动定时重试，不再一次失败就永久空白。
   - 连接状态从离线回到在线时，如果尚未成功渲染过配置，自动触发 `reload()` 补拉数据。
   - 首次失败时显示明确错误状态，避免静默失败误判为“前端渲染问题”。

验证要点：
1. 在服务启动瞬间打开设置页，即使首轮请求失败，1~2 秒后也会自动恢复并加载下拉选项。
2. 断网/服务短暂不可用后恢复，无需手动刷新页面即可恢复选项。

## 九次回归修正（Svelte 产物全局符号冲突导致全界面失效）
问题反馈：
1. 控制台持续报错：`TypeError: Cannot write to private field`。
2. 设置页“什么都点不了”，下拉、按钮等全部失效。

根因：
1. 多个 Svelte 分包文件（`*.svelte.js`）按普通 `<script>` 注入页面。
2. 每个分包在最外层都会生成一组 helper（如 `T/a/H`），但这些 helper 变量位于分包 IIFE 之外，落在全局作用域。
3. 后加载分包会覆盖先加载分包的 helper 实现，导致类私有字段访问函数错配，触发 `Cannot write to private field`，并中断后续渲染/交互流程。

修正：
1. 文件：`MFCMouseEffect/WebUIWorkspace/scripts/copy-output.mjs`
2. 对所有生成产物（`generatedFiles`）在复制阶段统一增加最外层作用域包装：
   - 写入标记：`/* mfx-scope-wrapped */`
   - 包装形式：`(() => { ...原始产物... })();`
3. 这样每个分包的顶部 helper 都限定在独立函数作用域，不再互相覆盖。
4. 包装在构建拷贝链路执行，`WebUI` 与 `x64/Debug|Release/webui` 同步得到同样修复产物，无需手动二次处理。

验证要点：
1. `pnpm run build` 完成后，`MFCMouseEffect/WebUI/*.svelte.js` 文件首行均为 `/* mfx-scope-wrapped */`。
2. 打开设置页不再出现 `Cannot write to private field`。
3. 各分区控件（下拉、按钮、输入）恢复可交互。

## 十次回归修正（录制期间快捷键仍被前台应用触发）
问题反馈：
1. 快捷键录制已成功，但按键同时继续对当前前台应用生效（例如触发已有热键行为）。
2. 预期是录制期间这些按键应被“只采集、不执行”。

根因：
1. 旧实现在 `WH_KEYBOARD_LL` 钩子中仅上报 `WM_MFX_KEY` 到应用，不拦截系统键盘链路。
2. 因此录制逻辑能拿到按键，但同一按键仍会继续传给前台应用/系统快捷键链。

修正：
1. 文件：`MFCMouseEffect/MouseFx/Core/System/GlobalMouseHook.h`
   - 新增 `SetKeyboardCaptureExclusive(bool enabled)`。
   - 新增 `keyboardCaptureExclusive_` 原子标志。
2. 文件：`MFCMouseEffect/MouseFx/Core/System/GlobalMouseHook.cpp`
   - 录制独占打开时，`WM_KEYDOWN/WM_SYSKEYDOWN` 在上报后返回 `1`，阻断后续系统分发。
   - 录制独占打开时，同时拦截 `WM_KEYUP/WM_SYSKEYUP`，防止残余按键状态泄漏。
3. 文件：`MFCMouseEffect/MouseFx/Core/Automation/ShortcutCaptureSession.h/.cpp`
   - 增加 `IsActive()` 用于同步录制会话状态到键盘钩子。
4. 文件：`MFCMouseEffect/MouseFx/Core/Control/AppController.cpp/.h`
   - `Start/Stop/PollShortcutCaptureSession` 全流程同步 `SetKeyboardCaptureExclusive(...)`。
   - `OnGlobalKey` 在录制态下仅供捕获逻辑消费，跳过常规键盘指示器路径，避免“录制键仍生效”的副作用。
   - `Stop()` 时显式关闭独占标志，确保退出后不残留键盘拦截状态。

验证要点：
1. 点击“录制”后按任意快捷键（如 `Ctrl+S` / `Alt+A`），映射可被采集。
2. 录制期间前台应用不再执行该快捷键对应动作。
3. 结束/取消录制后，系统快捷键行为恢复正常。

## 十一次回归修正（录制组合键丢失修饰符）
问题反馈：
1. 录制期按 `Alt+A`，结果仅录入 `A`（修饰符丢失）。

根因：
1. 录制独占拦截开启后，键盘事件在低层钩子被吞掉，`GetAsyncKeyState` 对修饰键状态的可见性不稳定。
2. 旧实现依赖 `GetAsyncKeyState(VK_MENU/VK_CONTROL/VK_SHIFT/...)` 组装 `KeyEvent`，因此出现“只剩主键”的回归。

修正：
1. 文件：`MFCMouseEffect/MouseFx/Core/System/GlobalMouseHook.h`
   - 新增 `keyboardModifierMask_`，用于在钩子内部维护修饰键状态。
2. 文件：`MFCMouseEffect/MouseFx/Core/System/GlobalMouseHook.cpp`
   - 在 `KeyboardHookProc` 中按 `keydown/keyup` 更新 `keyboardModifierMask_`。
   - 组装 `KeyEvent` 时从掩码读取 `ctrl/shift/alt/win`，不再依赖 `GetAsyncKeyState`。
   - `alt` 同时兜底 `LLKHF_ALTDOWN` 标志，保证系统键路径一致。
   - `Start/Stop` 时重置修饰键掩码，避免跨会话残留状态。

验证要点：
1. 录制期按 `Alt+A`，应稳定录入 `Alt+A`。
2. 录制期按 `Ctrl+Shift+T`，应完整录入 `Ctrl+Shift+T`。
3. 独占拦截行为保持不变：录制期间不触发前台应用快捷键。

## 十二次回归修正（已撤销）
说明：
1. 本轮曾尝试通过“请求超时兜底”解决 loading 长驻问题。
2. 你确认该方向不是根因（请求已成功返回），因此该方案已撤销，不纳入最终代码。

## 十三次回归修正（重载成功但状态文案不更新）
问题反馈：
1. 接口已重载成功，但顶部状态仍停留在“正在加载...”。

根因：
1. `reload()` 成功后调用 `markConnection('online')`。
2. 当当前连接状态本来就是 `online` 时，`markConnection` 的“同状态短路返回”触发，导致不会再次写入 `Ready` 文案。
3. 若这次返回里没有 `gpu_route_notice`，状态条就保持在上一条 loading 文案。

修正：
1. 文件：`MFCMouseEffect/WebUI/app.js`
   - 将 `reload()` 成功路径里的 `markConnection('online')` 改为 `markConnection('online', true)`，强制刷新在线状态文案。
2. 这样即使连接状态未变化，也会在每次成功重载后收敛状态，不残留 loading。

验证要点：
1. 点击“重载”后请求成功时，状态应从“正在加载...”切到“就绪/Ready”。
2. 若存在 `gpu_route_notice`，仍会按原逻辑显示该提示文案。

## 十四次回归修正（中文模式首屏状态仍显示 Ready）
问题反馈：
1. 中文配置下，页面刚启动左上角仍显示英文 `Ready.`，手动点击“重载”后才变成 `就绪。`。

根因：
1. 状态文案读取走 `currentText()`，旧实现每次都通过 `pickLang()` 读取 `ui_language` 下拉框值。
2. 启动阶段下拉框尚未稳定到配置语言时，会回退到浏览器语言，导致状态文案被英文覆盖。

修正：
1. 文件：`MFCMouseEffect/WebUI/i18n-runtime.js`
   - 新增 `activeLang`，由 `apply(lang)` 在每次应用语言时更新。
   - `currentText()` 优先读取 `activeLang`，仅在未应用语言时才回退 `pickLang()`。
2. 同步产物到运行目录：
   - `x64/Debug/webui/i18n-runtime.js`
   - `x64/Release/webui/i18n-runtime.js`

验证要点：
1. 中文配置下，首次打开页面后状态文案应直接显示 `就绪。`，不再出现英文 `Ready.`。
2. 手动“重载”前后状态语言保持一致。

## 十五次回归修正（链式鼠标动作缺少时间窗口，隔很久仍会触发）
问题反馈：
1. 鼠标动作映射链（如 `left_click>left_click`）在两次点击间隔很久后仍会触发。

根因：
1. 旧实现仅匹配“动作序列”，历史记录只保存动作 ID，不保存动作发生时间。
2. 只要顺序匹配，哪怕第一步与最后一步跨越很久，也会被判定为有效链。

修正：
1. 文件：`MFCMouseEffect/MouseFx/Core/Automation/InputAutomationEngine.h`
   - 将历史项从 `std::string` 升级为 `ActionHistoryItem{actionId, timestamp}`。
   - 新增 `ChainTimingLimit`，支持“相邻步最大间隔”和“整链最大总时长”。
2. 文件：`MFCMouseEffect/MouseFx/Core/Automation/InputAutomationEngine.cpp`
   - 鼠标链默认时间窗：单步最大间隔 `900ms`，整链最大总时长 `1800ms`。
   - 手势链默认时间窗：单步最大间隔 `2200ms`，整链最大总时长 `5000ms`。
   - `AppendActionHistory` 按总时长窗口清理过旧历史。
   - `FindEnabledBinding` 在序列匹配后新增时间窗口校验，不满足窗口则不触发。

验证要点：
1. `left_click>left_click`：两次点击间隔明显超过 1 秒，不应触发映射。
2. `left_click>left_click`：两次快速点击（< 900ms）应正常触发映射。
3. 单动作映射（如仅 `left_click`）不受该改动影响。
