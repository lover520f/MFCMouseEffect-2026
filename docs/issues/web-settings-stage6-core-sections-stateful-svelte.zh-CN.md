# 阶段 6.2：General/Effects/Text/Trail 状态化读写迁移到 Svelte

## 背景
- 阶段 6.1 已把 Input Indicator 的动态行为迁到 Svelte 状态层。
- 但 `General/Effects/Text/Trail` 仍是“静态 Svelte 模板 + `settings-form.js` 直接 DOM 填充/读取”模式：
  - 组件没有状态语义。
  - 读写逻辑集中在 `settings-form.js`，分区职责边界不清晰。

## 目标
1. 让四个分区与 Input Indicator 一样，形成统一的“分区状态组件 + 桥接 API + 表单编排委托”模式。
2. 保持 `buildState()` 输出 JSON 结构和字段名不变，后端无感知。
3. 保留 fallback，避免因为单个 bundle 加载异常导致整个设置页不可用。

## 方案
- 分区组件状态化：
  - `GeneralSettingsFields.svelte`：管理 `ui_language/theme/hold_follow_mode/hold_presenter_backend`。
  - `ActiveEffectsFields.svelte`：管理 `click/trail/scroll/hold/hover`。
  - `TextContentFields.svelte`：管理 `text_content/text_font_size`。
  - `TrailTuningFields.svelte`：管理 `trail_style/trail_profiles/trail_params`。
- 分区入口桥接：
  - 在 `general-main.js/effects-main.js/text-main.js/trail-main.js` 统一暴露 `render/read` API。
  - API 名称：
    - `window.MfxGeneralSection`
    - `window.MfxEffectsSection`
    - `window.MfxTextSection`
    - `window.MfxTrailSection`
- 表单编排委托：
  - `settings-form.js` 在 `render/read` 时优先委托四个分区 API。
  - API 缺失时继续走 legacy DOM 读写 fallback。

## 代码改动
- `MFCMouseEffect/WebUIWorkspace/src/general/GeneralSettingsFields.svelte`
  - 新增 props 与双向绑定状态。
  - 通过 `change` 事件输出快照。
- `MFCMouseEffect/WebUIWorkspace/src/effects/ActiveEffectsFields.svelte`
  - 新增选项 props 与 active 状态绑定。
  - 通过 `change` 事件输出当前 active 映射。
- `MFCMouseEffect/WebUIWorkspace/src/text/TextContentFields.svelte`
  - 新增 text 状态绑定并输出快照。
- `MFCMouseEffect/WebUIWorkspace/src/trail/TrailTuningFields.svelte`
  - 从静态表单升级为完整 trail 状态组件（profiles + params）。
  - 通过 `change` 事件输出结构化 trail 配置快照。
- `MFCMouseEffect/WebUIWorkspace/src/entries/general-main.js`
- `MFCMouseEffect/WebUIWorkspace/src/entries/effects-main.js`
- `MFCMouseEffect/WebUIWorkspace/src/entries/text-main.js`
- `MFCMouseEffect/WebUIWorkspace/src/entries/trail-main.js`
  - 四个入口统一维护 `currentState`，提供 `render/read`，并在未挂载时暴露 no-op fallback API。
- `MFCMouseEffect/WebUI/settings-form.js`
  - 新增四个 section helper 并在 `render/read` 优先委托分区 API。
  - 保留原 DOM 逻辑作为 fallback，确保兼容性。

## 验证
- 语法检查：
  - `node --check MFCMouseEffect/WebUIWorkspace/src/entries/general-main.js`
  - `node --check MFCMouseEffect/WebUIWorkspace/src/entries/effects-main.js`
  - `node --check MFCMouseEffect/WebUIWorkspace/src/entries/text-main.js`
  - `node --check MFCMouseEffect/WebUIWorkspace/src/entries/trail-main.js`
  - `node --check MFCMouseEffect/WebUI/settings-form.js`
- 前端构建：
  - 在 `MFCMouseEffect/WebUIWorkspace` 执行 `pnpm run build` 成功。
  - 产物 `general-settings.svelte.js`、`effects-settings.svelte.js`、`text-settings.svelte.js`、`trail-settings.svelte.js` 已更新。
- 工程构建：
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
  - 构建成功，`PostBuildEvent` 正常复制 WebUI 产物。

## 风险与后续
- 当前 `settings-form.js` 仍保留大量 fallback 分支，短期稳定但代码体积仍偏大。
- 后续可在阶段 6.x 继续把 fallback 逻辑按分区拆文件，进一步降低 `settings-form.js` 的复杂度。


