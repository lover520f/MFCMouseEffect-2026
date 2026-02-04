# MFCMouseEffect 文档

语言： [English](README.md) | [中文](README.zh-CN.md)

## 文档索引
- 市场/README 展示：`docs/marketing/readme_language_switch.md`（语言切换文案的文本渲染）
- 问题记录：`docs/issues/emoji-support.md`（设置输入框与文本点击特效的表情渲染）
- 问题记录：`docs/issues/text-effect-motion.md`（切换到 DWrite 后的飘散路径一致性）
- 问题记录：`docs/issues/settings-emoji-preview.md`（设置页表情彩色预览叠层）
- 问题记录：`docs/issues/web-settings-server-lifecycle.zh-CN.md`（浏览器设置页：idle 重启崩溃 + token 轮换）
- 架构整理：`docs/architecture/tray-and-appcontroller-refactor.zh-CN.md`（托盘菜单表驱动 + AppController 解耦）
- 架构整理：`docs/architecture/settingswnd-emoji-split.zh-CN.md`（SettingsWnd 表情处理拆分）
- 架构整理：`docs/architecture/ui-folder-structure.zh-CN.md`（UI 文件夹结构整理）
- 架构整理：`docs/architecture/mousefx-folder-structure.zh-CN.md`（MouseFx 文件夹结构整理）
- 架构整理：`docs/architecture/trail-effects-differentiation.zh-CN.md`（拖尾差异化：历史配置 + renderer 拆分）
- 架构整理：`docs/architecture/trail-profiles-config.zh-CN.md`（config.json：trail_profiles + reload_config）
- 架构整理：`docs/architecture/trail-tuning-settings-ui.zh-CN.md`（设置窗口：拖尾预设 + 高级调参）
- 架构整理：`docs/architecture/web-settings-ui.zh-CN.md`（浏览器设置页：内置 loopback HTTP server）
- 安装包：`docs/install/installer-packaging-20260204.md`（Inno Setup 更新，2026-02-04）

## 功能简介
- 全局鼠标点击可视化（Windows）：低级鼠标钩子 `WH_MOUSE_LL` + GDI+ 分层窗口波纹。
- 点击穿透：不会阻挡下层窗口。

## 构建与运行
1. 用 Visual Studio 打开 `MFCMouseEffect.slnx`（或生成的 `.sln`）。
2. 选择 `x64` + `Debug`（推荐测试时用）。
3. 编译并运行 `MFCMouseEffect`，输出在 `x64\Debug\MFCMouseEffect.exe`（Release 在 `x64\Release\...`）。
4. Debug 启动后 ~250ms 会在当前光标位置自动打一发“自检波纹”，之后任意点击都会有波纹。
5. 如果要点击管理员/提权窗口，最好也以管理员身份运行本程序，保证完整的钩子权限。

## 发布版无窗口（托盘常驻）
Release 版本已改为“完全不创建主框架窗口”，只创建隐藏宿主窗口用于托盘图标，因此启动不会闪出管理窗口。
- 入口：`x64\Release\MFCMouseEffect.exe`
- 托盘图标：右键菜单“退出”（双击也可退出）
- Debug 仍保留主窗口便于调试（不影响波纹渲染）
说明：托盘菜单现在会根据当前语言显示中文或英文（不再中英混排）。

## 设置（浏览器页，非 background 模式）
托盘菜单适合快速切换。复杂配置（包含高级调参）通过托盘 **设置...** 打开浏览器设置页（程序内置 loopback HTTP server）。
- 配置持久化：写入同目录 `config.json`
- 立即生效：保存后立刻应用
- 详细说明：`docs/architecture/web-settings-ui.zh-CN.md`

## 外观自定义
- 主要文件：`MFCMouseEffect/MouseFx/Styles/RippleStyle.h`、`MFCMouseEffect/MouseFx/Windows/RippleWindow.cpp`。
- 关键参数：
  - 时长：`RippleStyle::durationMs`
  - 半径：`startRadius`、`endRadius`
  - 窗口大小：`windowSize`
  - 按键配色：在 `RippleWindow::StartAt(...)` 的 switch 中调整 fill/stroke/glow

## 运行说明（宏观点）
- **UAC/管理员窗口**：如果需要在管理员应用（高权限窗口）里也完整生效，建议本程序同样“以管理员身份运行”。
- **托盘 vs 后台模式**：托盘模式可交互；后台模式不提供托盘 UI（完全由父进程/IPC 控制）。
- **IPC 控制**：后台模式通过 stdin JSON 接收父进程命令；stdin 关闭时会自动退出（用于父进程生命周期托管）。
- **配置持久化**：`config.json` 与 exe 同目录；主题与当前各分类特效会在 tray/IPC 修改时写回。
- **安全软件影响**：部分安全软件可能会拦截全局钩子或分层窗口渲染，导致不生效或间歇性失效。

## 故障排查
- **完全没波纹（Debug）**：连自检波纹都没有，说明启动失败。看弹窗里的 `Stage/Error/Message`。
  - `Stage: dispatch window`、`Error: 1400 (无效的窗口句柄)`：代码已修复，Clean+Rebuild 后运行 `x64\Debug\MFCMouseEffect.exe`。
  - 其他错误：按提示，一般是权限或系统策略。
- **钩子报错**：弹窗或 VS Output 有 `MouseFx: global hook start failed. GetLastError=...`。如果点击的是管理员窗口，请用“以管理员身份运行”启动本程序；安全软件可能拦截钩子。
- **高 DPI 偏位**：启动时已启用 DPI 感知；确保用最新编译的二进制。
- **运行错了二进制**：曾经生成过 `MFCMouseEffect\x64\Debug\...` 的旧输出，现已改为 `x64\Debug\...`。请 Clean + Rebuild 后运行新路径。
- **虚拟副屏/平板副屏偏移**：部分虚拟显示器软件/驱动会导致坐标映射异常或 DPI 坐标系不一致，表现为特效偏移/消失。参见：`docs/issues/virtual-display-coordinates.zh-CN.md`。
  - 2026-01：已默认启用坐标归一化，大多数虚拟副屏场景已恢复正常；若仍有偏移，请附上副屏软件/驱动与 DPI 信息反馈。

## SDI/单窗口说明
- 项目已改为 SDI：一个顶层主窗口承载视图；波纹渲染仍在独立的透明分层窗口中，与主框架解耦。
- 如果需要多个窗口，可按需创建多个独立顶层框架（非 MDI 子窗），互不嵌套。
