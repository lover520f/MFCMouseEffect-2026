# Automation Mapping TODO（P2）

## 目标
把当前“鼠标动作 / 手势 -> 快捷键注入”的自动化映射，逐步升级为可被普通用户理解、可被高级用户扩展、可和 BetterTouchTool / X-Mouse Button Control / SteerMouse 做正面对比的桌面输入自动化能力。

## 当前判断
- 当前不是空壳，已经具备可用骨架：
  - 鼠标触发：左键、右键、中键、滚轮上、滚轮下。
  - 手势触发：左 / 中 / 右键拖拽，以及 `none` 无按键纯手势。
  - 手势类型：预设方向手势 + 自定义手势。
  - 映射条件：全局 / 指定进程、修饰键 `any / none / exact`。
  - 触发链：支持 `action1>action2`，并带时间窗口。
- 输出动作：当前通过 `actions[]` 动作模型执行 `send_shortcut / delay / open_url / launch_app`，WebUI 已有第一版动作列表编辑器。
  - 诊断：`recognized / matched` 拆分、`recent_events`、候选窗口、分数、噪声过滤等观测能力已具备。
- 主要短板不在“能不能识别手势”，而在：
  - 输出动作类型太少。
  - 鼠标触发源不够完整。
  - 缺 Profile / Layer / 导入导出。
  - 条件系统还比较轻。
  - 模板数量和产品化引导不足。

## 对标差距
- BetterTouchTool：
  - 强项：多触发源、多动作序列、脚本、URL、窗口操作、条件、延迟、循环、全局 / App-specific triggers、绘图手势 scope。
  - 我们差距：动作系统、条件系统、Profile 化、手势多样本训练。
- X-Mouse Button Control：
  - 强项：更多鼠标按钮、button chording、长按分段动作、按下 / 释放 / 按住 / 重复 / sticky hold、运行程序、打开文件夹。
  - 我们差距：侧键、水平滚轮、长按、按下释放语义、重复发送、阻止原始输入。
- SteerMouse：
  - 强项：设备级 Profile、不同鼠标独立设置、滚轮 / 指针速度控制、App-specific profile。
  - 我们差距：设备识别、设备级 Profile、滚轮和指针控制。
- Karabiner-Elements：
  - 强项：复杂键鼠事件改写、条件、设备选择、JSON 生态。
  - 我们差距：底层事件重写能力、配置分享生态、复杂条件表达。

## 阶段路线

### M0：边界收口与用户预期校准
状态：已完成

- [x] README / WebSettings 文案明确当前是“自动化映射 / 快捷键注入”，避免暗示已经是完整宏平台。
- [x] 自动化设置页增加简短能力说明：当前支持触发源、条件、输出动作边界。
- [x] 在自动化文档中标明“触发链”和“动作链”的区别：
  - 触发链：多个输入动作组合后触发一个映射。
  - 动作链：一个触发后执行多个输出动作；当前尚未完整实现。

验收：
- [x] 新用户能在 README 和设置页理解当前能做什么、不能做什么。
- [x] 不再把“自动化映射”宣传成完整宏平台。

### M1：动作系统 v1
状态：进行中

- [x] 新增动作模型 `AutomationAction`，不再把输出长期绑定到单个 `keys` 字段。
- [x] 配置入口切换为 `automation.mouse_mappings[].actions[]` / `automation.gesture.mappings[].actions[]`。
- [x] 旧 `keys` 不作为当前目标；后续直接按新模型推进，减少迁移包袱和双模型维护成本。
- [ ] 支持第一批动作类型：
  - [x] `send_shortcut`
  - [x] `open_url`
  - [x] `launch_app`
  - [ ] `run_command`（默认需要安全提示 / 明确开关）
  - [ ] `type_text`
  - [x] `delay`
- [x] WebUI 增加动作列表编辑器，支持添加、删除、排序。
- [x] 后端动作链改为后台 worker 顺序执行，避免在输入回调里阻塞。
- [ ] 后端执行动作链需要继续补全每步错误诊断和用户可见状态。
  - [x] 首轮动作运行诊断已接入 `/api/state` 与自动化调试卡片，可看到最近一次动作执行状态、停止原因和步骤明细。
  - [ ] 还缺更明确的用户态失败提示、非 Debug 模式下的轻提示，以及针对 `run_command` 的安全/失败说明。

验收：
- [x] 单个鼠标动作的数据结构可以承载多个动作。
- [x] `send_shortcut` / `delay` / `open_url` / `launch_app` 已按新动作模型读取、保存、匹配、执行。
- [x] WebUI 已可编辑 `send_shortcut` / `delay` / `open_url` / `launch_app`，并保留动作顺序。
- [x] `delay` 最大 60000ms，配置更新 / reset 会清理未执行队列并中断等待。
- [ ] `run_command` 有安全边界，默认不静默执行不可信命令。
- [ ] 回归脚本覆盖快捷键、delay 顺序、URL / app / command dry-run。

### M2：鼠标触发源补全
状态：待开始

- [ ] 支持侧键：Button4 / Button5。
- [ ] 支持水平滚轮 / tilt wheel（平台支持时）。
- [ ] 支持按下 / 释放触发。
- [ ] 支持长按触发。
- [ ] 支持按住重复触发。
- [ ] 支持双击 / 三击条件。
- [ ] 支持 button chording，例如右键 + 滚轮。
- [ ] 明确是否阻止原始输入；若支持，需要按平台拆分权限和实现。

验收：
- [ ] 常见多键鼠标可以完成浏览器前进后退、标签切换、窗口操作。
- [ ] 长按、释放、重复不会误触发普通点击。
- [ ] Windows / macOS 行为差异在文档中明确。

### M3：Profile / Layer / 导入导出
状态：待开始

- [ ] 引入 Profile：
  - [ ] 默认全局 Profile。
  - [ ] App-specific Profile。
  - [ ] 可启用 / 禁用 Profile。
- [ ] 引入 Layer：
  - [ ] 临时 Layer（按住某按钮或快捷键期间生效）。
  - [ ] Toggle Layer（再次触发关闭）。
- [ ] 支持导入 / 导出自动化配置。
- [ ] 支持模板包：
  - [ ] 浏览器导航。
  - [ ] 窗口管理。
  - [ ] 文档阅读。
  - [ ] IDE / 编辑器。
  - [ ] 演示录屏。

验收：
- [ ] 用户可以分享一个自动化配置文件。
- [ ] App-specific 配置不会污染全局配置。
- [ ] Layer 有清晰状态提示和退出方式。

### M4：手势产品化增强
状态：待开始

- [ ] 自定义手势支持多个样本变体。
- [ ] 手势编辑器增加“测试当前手势”入口，显示：
  - [ ] recognized gesture
  - [ ] matched gesture
  - [ ] best score
  - [ ] runner-up score
  - [ ] 是否会触发
- [ ] 支持同一手势在不同 trigger button / scope 下执行不同动作。
- [ ] 为复杂手势提供推荐阈值提示。
- [ ] 增加误触保护预设：保守 / 平衡 / 灵敏。

验收：
- [ ] 用户不看日志也能判断“为什么这个手势没触发 / 触发错了”。
- [ ] W / V / 斜线 / 自定义多笔画手势有稳定演示样例。

### M5：条件系统
状态：待开始

- [ ] App scope 增加排除模式。
- [ ] 支持窗口标题 / bundle id / executable name 条件（按平台能力分层）。
- [ ] 支持鼠标位置条件：
  - [ ] 屏幕边缘。
  - [ ] 指定屏幕。
  - [ ] 菜单栏 / Dock / 任务栏区域（平台支持时）。
- [ ] 支持左右修饰键区分（平台支持时）。
- [ ] 支持设备条件：
  - [ ] 指定鼠标。
  - [ ] 指定键盘。
- [ ] 条件表达保持 UI 友好，不直接暴露复杂 JSON 为主路径。

验收：
- [ ] 可以配置“只在浏览器生效，且排除某些 app”的映射。
- [ ] 可以配置“鼠标在屏幕边缘滚轮切换桌面 / 标签”的场景。

### M6：生态与可分享能力
状态：待开始

- [ ] 自动化模板文档化，说明模板结构和贡献方式。
- [ ] 提供内置模板预览图 / GIF。
- [ ] 支持导出为 gist / 文件包友好的 JSON。
- [ ] README 增加自动化示例区，但保持真实边界。
- [ ] 准备 Reddit / GitHub Discussion 可展示的自动化 demo。

验收：
- [ ] 新用户 3 分钟内可以套用一个模板。
- [ ] 贡献者能提交一个模板 PR。
- [ ] README 中自动化部分既有吸引力，也不夸大。

## 技术约束
- 新配置以动作数组为唯一长期模型：
  - `automation.mouse_mappings[].actions[]`
  - `automation.gesture.mappings[].actions[]`
  - `actions[].type`
  - `actions[].shortcut`（当 `type=send_shortcut` 时）
  - `actions[].delay_ms`（当 `type=delay` 时，1-60000）
- 映射触发与作用域字段继续保留：
  - `trigger`
  - `app_scope / app_scopes`
- 旧 `keys` 字段不作为当前兼容目标；如果后续确实需要导入旧配置，应以一次性迁移工具处理，而不是在运行时长期维护双路径。
- 高风险动作（例如 `run_command`）必须有显式安全边界。
- 所有行为变化必须同步：
  - WebUI 文案
  - docs
  - contract regression
- macOS / Windows 行为不能默认假设一致，平台差异必须在 schema / docs 中暴露。

## 推荐下一个开发切口
继续收口 M1，优先级建议如下：

1. 补动作执行诊断，让用户能看见哪一步失败、失败原因是什么。
2. 再推进 `run_command`，但必须先设计安全边界和显式开关。
3. 再评估 `type_text` / `switch_profile` 这类高频动作是否进入 M1.5。

理由：
- `actions[]` 模型、后台 worker 和 WebUI 动作编辑器已经形成第一版闭环。
- 现在继续补执行可观测性和高频动作类型，收益比继续堆触发源更高。
- `launch_app` 已打通后，接着做 `run_command` 前先补诊断，会更容易把安全边界和失败提示设计清楚。

## 参考文档
- 本项目行为细节：`docs/automation/automation-mapping-notes.md`
- 手势匹配细节：`docs/automation/gesture-matching.md`
- 手势调试 UI：`docs/automation/gesture-debug-ui-notes.md`
- POSIX 自动化合同：`docs/architecture/posix-core-automation-contract-workflow.md`
- BetterTouchTool 文档：
  - `https://docs.folivora.ai/docs/trackpad-mouse/magic-mouse-trackpad/`
  - `https://docs.folivora.ai/docs/701_drawings.html`
  - `https://docs.folivora.ai/docs/actions/control-flow/`
- X-Mouse Button Control 用户手册：
  - `https://www.highrez.co.uk/downloads/x-mouse%20button%20control%20user%20guide.pdf`
- Karabiner-Elements 鼠标按钮说明：
  - `https://karabiner-elements.pqrs.org/docs/help/how-to/mouse-button/`
- SteerMouse：
  - `https://steermouse.com/`
