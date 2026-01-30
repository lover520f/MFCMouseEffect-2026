# UI 文件夹结构整理（2026-01-30）

## 目标
减少 `MFCMouseEffect/` 目录下的文件“铺一地”的观感，把**可以按职责归类**的 UI 文件放到子目录里，但不引入额外框架或过度分层。

## 目录划分
### 1) `MFCMouseEffect/UI/Tray/`
托盘宿主窗口与菜单构建逻辑（托盘 UI 相关）。
- `TrayHostWnd.*`
- `TrayMenuBuilder.*`
- `TrayMenuCommands.h`

### 2) `MFCMouseEffect/UI/Settings/`
设置窗口 UI 与 emoji 预览/格式化逻辑（设置 UI 相关）。
- `SettingsWnd.*`（含 `SettingsWnd.Emoji.cpp`）
- `EmojiPreviewWnd.*`

### 3) `MFCMouseEffect/UI/Frame/`
主框架/子框架（MFC Frame 相关）。
- `MainFrm.*`
- `ChildFrm.*`

### 4) `MFCMouseEffect/UI/Panes/`
Docking pane / 视图树等辅助面板（界面组件）。
- `ClassView.*`
- `FileView.*`
- `OutputWnd.*`
- `PropertiesWnd.*`
- `ViewTree.*`

### 5) `MFCMouseEffect/UI/DocView/`
SDI 的 Doc/View（文档/视图层）。
- `MFCMouseEffectDoc.*`
- `MFCMouseEffectView.*`

## 说明
- include 改为以工程根目录为基准的路径（例如 `UI/Frame/MainFrm.h`），避免“相对路径地狱”。
- 行为不变：仅做文件归档与 include/vcxproj 同步。

