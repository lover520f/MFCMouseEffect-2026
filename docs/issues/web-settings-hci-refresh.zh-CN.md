# Web 设置界面 HCI 重构（信息架构 + 可用性）

## 背景
原有 Web 设置页功能完整，但存在以下问题：
- 信息密度高且分区弱，用户难以快速定位目标设置。
- 控件视觉层级不清晰，主要操作和次要操作区分不明显。
- 多屏覆盖配置是动态内联样式，阅读与维护成本都偏高。
- 移动端体验不稳定，字段在窄屏下可读性不足。

本次改造目标：在不改变现有设置 API 与字段语义的前提下，使界面符合基础人机交互设计原则。

## 设计原则落地

### 1. 可见性与信息层级
- 顶部保留统一操作区（Reload/Reset/Stop/Apply），并强化主按钮视觉优先级。
- 增加分区导航（General / Effects / Input Indicator / Text / Trail），减少长页面查找成本。
- 每个卡片增加说明副标题，帮助用户理解该区块用途。

### 2. 一致性与可预测性
- 全局统一间距、圆角、边框、输入高度、焦点样式。
- 统一按钮语义：
  - 主操作：`Apply`
  - 次要操作：`Reload`、`Stop`
  - 风险操作：`Reset`
- 保留原有 `id`，确保脚本绑定和数据通道不受影响。

### 3. 反馈与状态
- 状态条改为更高对比度、可读性更强的提示样式（ok/warn/offline）。
- 分区导航增加 active 状态，用户滚动时可识别当前位置。
- 离线/未授权行为拦截逻辑保持不变，提示语义更清晰。

### 4. 可达性与易操作
- 增强键盘焦点可见性（`focus-visible`）。
- 输入控件 hover/focus 态统一并提高对比度。
- 移动端改为单列布局，分区导航支持横向滚动。
- 动效增加 `prefers-reduced-motion` 兼容，减少视觉负担。

## 代码改动范围
- `MFCMouseEffect/WebUI/index.html`
  - 重构页面结构：顶部操作、分区导航、卡片化信息布局
  - 移除主要内联样式，改为 class 驱动
- `MFCMouseEffect/WebUI/styles.css`
  - 建立统一设计变量（颜色/阴影/间距/圆角）
  - 实现浅色高可读主题与响应式布局
  - 补齐导航 active、表单焦点、动态多屏列表样式
- `MFCMouseEffect/WebUI/app.js`
  - 新增卡片说明文案 i18n（中/英）
  - 多屏覆盖列表从内联 `style` 改为 class 渲染
  - 新增分区导航 active 同步逻辑（含 hash 与滚动观察）

## 兼容性说明
- 所有原有表单控件 `id` 保持不变。
- `/api/schema`、`/api/state`、`/api/reload`、`/api/reset`、`/api/stop` 请求协议不变。
- 配置读写字段完全兼容，不影响现有 `config.json`。

## 验证清单
1. 打开设置页，确认五个分区都能正常显示并导航跳转。
2. 修改任意配置并 `Apply`，确认状态提示正常、配置可持久化。
3. 切换语言（中/英），确认标题、分区、副标题与按钮文案正确切换。
4. 切换 `Input Indicator` 的 position mode 与 target monitor，确认多屏覆盖区域显示/隐藏正确。
5. 窄窗口（移动端宽度）下检查：布局为单列、输入可读可点、导航可横向滚动。

---

## 增量改动（阶段 2：分区工作台，解决“所有设置堆一页”）

### 1. 信息架构调整
- 从“整页堆叠卡片”改为“左侧导航 + 右侧工作面板”。
- 默认启用 `专注视图`：每次只展示一个分区，降低视觉噪音和误操作概率。
- 保留 `全部分区` 模式：用于对照多个分区参数。

### 2. 交互模块拆分（避免继续膨胀 `app.js`）
- 新增：`MFCMouseEffect/WebUI/section-workspace.js`
- 责任：
  - 维护当前分区激活状态
  - 管理专注/全部两种视图模式切换
  - 同步 hash、左侧导航 active、右侧“当前分区”摘要

### 3. 页面结构与样式
- `MFCMouseEffect/WebUI/index.html`
  - 新增 `settings-shell` 双栏结构
  - 新增视图模式按钮（专注视图/全部分区）
  - 新增当前分区摘要区（标题 + 说明）
- `MFCMouseEffect/WebUI/styles.css`
  - 新增侧栏、工作面板、摘要区样式
  - 设置卡片默认隐藏，仅显示 active 卡片；`全部分区` 模式恢复全部显示
  - 补齐 1080/760 断点行为：平板与手机自动降级为纵向布局

### 4. `app.js` 角色收敛
- 移除旧的滚动观察导航逻辑（`IntersectionObserver` + `scrollToHash`）。
- 改为调用 `MfxSectionWorkspace` 的 `init/refresh/syncI18n`，让 `app.js` 聚焦状态编排与 API 通道。

## 阶段 2 验证清单
1. 默认进入设置页时，仅显示一个分区卡片。
2. 点击左侧分区项，右侧卡片切换且摘要区标题/说明同步更新。
3. 切换到“全部分区”后，所有卡片显示；点击导航会滚动到目标分区。
4. 切回“专注视图”后恢复单分区显示。
5. 中英文切换后，模式按钮、提示文案、分区摘要文本保持一致。

---

## 增量改动（阶段 3：状态提示位置调整）

### 改动目的
- 将顶部状态提示（如“就绪 / 应用成功 / 警告”）从右上角移动到左上角。
- 与当前主操作布局和阅读路径保持一致，减少跨屏视线跳转。

### 改动文件
- `MFCMouseEffect/WebUI/styles.css`
  - `.status` 默认定位从 `right: 16px` 调整为 `left: 16px`。
  - 窄屏下仍保持左右边距拉伸显示（`left: 10px; right: 10px;`）。

---

## 增量改动（阶段 4：修复分栏回退 + Workspace 逻辑重整）

### 1. 修复“全部分区模式”分栏丢失
- 文件：`MFCMouseEffect/WebUI/styles.css`
- 问题：
  - 之前切到 `全部分区` 时卡片被线性堆叠，原本的分栏信息结构退化。
- 处理：
  - 恢复 `is-all-mode` 下的 12 栏网格布局。
  - 恢复分区占栏规则：`general/effects=6`、`input=7`、`text=5`、`automation/trail=12`。
  - 在 `1080px` 断点以下自动回落到单列，保证移动端可读性。

### 2. 重整分区工作台 JS（可维护性）
- 文件：`MFCMouseEffect/WebUI/section-workspace.js`
- 处理：
  - 收敛状态机（`mode/sections/activeId`）和渲染入口（`render`）。
  - 去掉重复分支，统一 `setActive` / `setMode` 调度路径。
  - 增加本地视图模式记忆（`localStorage`），避免每次打开都丢失用户偏好。
  - 保持与 `app.js` 的边界：`app.js` 只调用 `init/refresh/syncI18n`。

## 阶段 4 验证清单
1. 切换到 `全部分区` 后，桌面端应恢复多栏卡片排布，不再全纵向堆叠。
2. 切换到 `专注视图` 后，只显示当前分区卡片。
3. 刷新页面后应保留上次选择的视图模式（专注/全部）。
4. 1080px 以下窗口宽度时，`全部分区` 自动改为单列排版。

---

## 增量改动（阶段 5：工作台迁移到 Svelte 组件）

### 说明
- 在保持页面行为与 API 不变的前提下，将“分区工作台”交互层迁移到 Svelte。
- 渐进迁移总清单见：`docs/issues/web-settings-svelte-migration.zh-CN.md`。

### 变更范围
- `MFCMouseEffect/WebUI/index.html`：加载 `section-workspace.svelte.js`。
- `MFCMouseEffect/WebUIWorkspace/*`：新增 Svelte 源码与构建链。
- `MFCMouseEffect/WebUI/section-workspace.svelte.js`：编译产物（部署使用）。
- `MFCMouseEffect/WebUI/app.js`：`syncI18n` 向工作台模块透传 i18n 文案对象。
