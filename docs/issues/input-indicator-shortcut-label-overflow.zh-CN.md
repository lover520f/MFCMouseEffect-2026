# 输入指示器：快捷键文本被 `...` 截断问题（根因修复）

## 问题现象
1. 键盘指示器展示组合键时，文本长度稍长就被显示为 `...`。
2. 实际效果是用户看不到完整快捷键，识别成本高。

## 根因
1. `IndicatorRenderer::RenderKeyAction` 使用了固定字符上限策略：
   - `TruncateLabel(label, 12)`。
2. 该策略与实际字体宽度、窗口尺寸无关，属于硬编码截断。

## 方案（本次落地）
1. 移除固定字符截断逻辑（删除 `TruncateLabel` 路径）。
2. 引入“可配置布局策略”，由用户决定如何处理长快捷键：
   - `fixed_font`：固定字体大小，必要时自动扩展指示器宽度（默认）。
   - `fixed_area`：固定显示区域，必要时自动缩小字体。
3. 渲染侧统一关闭字符串省略语义：
   - `StringFormatFlagsNoWrap`
   - `StringTrimmingNone`
4. `fixed_font` 模式下不再挤压文本，而是先测量文本宽度再扩展窗口宽度。
5. `fixed_area` 模式下继续使用测量驱动的字号收敛，确保在固定区域内可读。

## 修改文件
1. `MFCMouseEffect/MouseFx/Renderers/Indicator/IndicatorRenderer.cpp`
2. `MFCMouseEffect/MouseFx/Renderers/Indicator/IndicatorRenderer.h`
3. `MFCMouseEffect/MouseFx/Core/Overlay/InputIndicatorOverlay.cpp`
4. `MFCMouseEffect/MouseFx/Core/Overlay/InputIndicatorOverlay.h`
5. `MFCMouseEffect/MouseFx/Core/Config/EffectConfig.h`
6. `MFCMouseEffect/MouseFx/Core/Config/EffectConfig.Internal.cpp`
7. `MFCMouseEffect/MouseFx/Core/Config/EffectConfigJsonKeys.Input.h`
8. `MFCMouseEffect/MouseFx/Core/Config/EffectConfigJsonCodec.Parse.Input.cpp`
9. `MFCMouseEffect/MouseFx/Core/Config/EffectConfigJsonCodec.Serialize.Input.cpp`
10. `MFCMouseEffect/MouseFx/Core/Control/CommandHandler.ApplySettings.cpp`
11. `MFCMouseEffect/MouseFx/Server/SettingsSchemaBuilder.cpp`
12. `MFCMouseEffect/MouseFx/Server/SettingsStateMapper.cpp`
13. `MFCMouseEffect/WebUIWorkspace/src/input-indicator/InputIndicatorFields.svelte`
14. `MFCMouseEffect/WebUIWorkspace/src/entries/input-indicator-main.js`
15. `MFCMouseEffect/WebUI/settings-form.js`
16. `MFCMouseEffect/WebUI/i18n.js`

## 结果
1. 不再主动生成 `...`。
2. 默认策略改为“固定字体 + 扩展显示区域”，长快捷键仍保持可读字号。
3. 用户可切换到“固定显示区域 + 缩小字体”，满足不同偏好。

## 验证
1. `Debug|x64` 编译通过：
   - `MSBuild MFCMouseEffect/MFCMouseEffect.vcxproj /p:Configuration=Debug /p:Platform=x64`
2. 手动验证建议：
   - 设置“快捷键文本布局策略”为“固定字体大小（自动扩展显示区域）”。
   - 依次触发 `Alt+A`、`Ctrl+Shift+Alt+T`、`Ctrl+Shift+Alt+Win+F12`，观察字号基本一致且宽度自动扩展。
   - 再切换为“固定显示区域（自动缩小字体）”，观察宽度保持不变、文本随长度缩小。
