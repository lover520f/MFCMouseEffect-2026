# SettingsWnd 表情处理拆分（2026-01-30）

## 目的
`SettingsWnd.cpp` 体积偏大且混合了多类职责（窗口、布局、绘制、设置绑定、表情处理/预览）。
这次先做最小拆分：把“表情相关（emoji formatting + 预览同步）”独立出去，降低主文件噪声，但不引入复杂框架。

## 变更点
- 新增：`MFCMouseEffect/Settings/EmojiUtils.h`
- 新增：`MFCMouseEffect/Settings/EmojiUtils.cpp`
  - 负责 UTF-16 codepoint 读取（含 surrogate pair）与 emoji 判断。
- 新增：`MFCMouseEffect/UI/Settings/SettingsWnd.Emoji.cpp`
  - 承载 `OnTextChange/Focus/KillFocus`、`ApplyEmojiFormatting`、`GetEditFontPx`、`UpdatePreviewFromEdit`。
- `MFCMouseEffect/UI/Settings/SettingsWnd.cpp` 删除了匿名命名空间里的 emoji 解析函数，只保留 UI 通用小工具（如 `ClampInt`）。

## 设计原则
- **SRP**：emoji 解析/字体格式化/预览同步是独立变化点，不应挤在窗口主逻辑里。
- **低耦合**：`MFCMouseEffect/UI/Settings/SettingsWnd.Emoji.cpp` 仍只依赖 `MFCMouseEffect/UI/Settings/SettingsWnd.h` 与 `Settings/EmojiUtils.*`。

## 回归验证（手动）
1. 设置窗口文本框输入 emoji（含肤色/ZWJ 组合），富文本字体切换仍生效。
2. 文本框失焦时，彩色预览仍显示；聚焦时预览仍隐藏。
