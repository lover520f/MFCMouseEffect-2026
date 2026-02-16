# 阶段 6.1：Input Indicator 状态化迁移到 Svelte

## 背景
- 当前 Web 设置页已经完成壳层与分区挂载迁移，但 `Input Indicator` 仍有一段关键行为依赖 `settings-form.js` 的 legacy DOM 拼装：
  - `buildPerMonitorUI(...)` 动态创建 per-monitor override 行。
  - `readPerMonitorUI(...)` 从 DOM 反向读取覆盖配置。
- 这段逻辑和组件渲染分离，导致维护成本高，也不利于继续推进“按功能块一次性迁移”。

## 目标
1. 把 `Input Indicator` 的动态行为迁到 Svelte 组件内部，减少 legacy DOM 操作。
2. 保持 `buildState().input_indicator` 结构与后端契约不变。
3. 保留 fallback 路径，确保在异常加载场景下仍可降级。

## 方案
- 在 `InputIndicatorFields.svelte` 内部建立表单状态与 per-monitor 行状态：
  - `form`: 管理 `enabled/position_mode/offset/absolute/target_monitor/key_display_mode/size/duration`。
  - `monitorRows`: 管理每个显示器 override 的 `enabled/absolute_x/absolute_y`。
- 通过 `change` 事件持续派发快照，并在 `input-indicator-main.js` 维护 `currentState`。
- 暴露兼容桥接 API：
  - `window.MfxInputIndicatorSection.render({ schema, indicator, texts })`
  - `window.MfxInputIndicatorSection.read()`
  - `window.MfxInputIndicatorSection.syncIndicatorPositionUi()`
- `settings-form.js` 增加委托入口：
  - 优先调用 `MfxInputIndicatorSection` 渲染/读取。
  - API 不可用时继续保留 legacy fallback 逻辑。

## 代码改动
- `MFCMouseEffect/WebUIWorkspace/src/input-indicator/InputIndicatorFields.svelte`
  - 从静态字段模板升级为状态化组件。
  - 接入 schema options、monitors、overrides、i18n 文本。
  - 在组件内渲染 per-monitor overrides 行，替代 legacy 动态 DOM 拼装。
- `MFCMouseEffect/WebUIWorkspace/src/entries/input-indicator-main.js`
  - 新增状态归一化与桥接 API。
  - 监听组件 `change` 事件，维护可读取的当前配置快照。
- `MFCMouseEffect/WebUI/settings-form.js`
  - 新增 `inputIndicatorSection()` helper。
  - `renderInputIndicator/read/syncIndicatorPositionUi/bindIndicatorEvents` 优先委托 Svelte API。
  - 保留 legacy 读写/事件绑定逻辑作为 fallback。
- `MFCMouseEffect/WebUI/input-indicator-settings.svelte.js`
  - 由 `pnpm run build` 生成并覆盖，产物体积上升符合状态化迁移预期。

## 验证
- 语法检查：
  - `node --check MFCMouseEffect/WebUIWorkspace/src/entries/input-indicator-main.js`
  - `node --check MFCMouseEffect/WebUI/settings-form.js`
- 前端构建：
  - 在 `MFCMouseEffect/WebUIWorkspace` 执行 `pnpm run build` 成功。
- 工程构建：
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
  - 构建成功，`PostBuildEvent` 正常复制 WebUI 产物。

## 风险与后续
- 当前 `settings-form.js` 仍保留 Input Indicator 的 fallback 代码，短期保证稳定，后续可在阶段 6.x 继续收敛。
- 下一步建议按同样方式推进其它仍含 legacy DOM 动态行为的分区，逐项迁移并验证。


