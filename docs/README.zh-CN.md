# MFCMouseEffect 文档

语言： [English](README.md) | [中文](README.zh-CN.md)

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

## 外观自定义
- 主要文件：`MFCMouseEffect/MouseFx/RippleStyle.h`、`MFCMouseEffect/MouseFx/RippleWindow.cpp`。
- 关键参数：
  - 时长：`RippleStyle::durationMs`
  - 半径：`startRadius`、`endRadius`
  - 窗口大小：`windowSize`
  - 按键配色：在 `RippleWindow::StartAt(...)` 的 switch 中调整 fill/stroke/glow

## 故障排查
- **完全没波纹（Debug）**：连自检波纹都没有，说明启动失败。看弹窗里的 `Stage/Error/Message`。
  - `Stage: dispatch window`、`Error: 1400 (无效的窗口句柄)`：代码已修复，Clean+Rebuild 后运行 `x64\Debug\MFCMouseEffect.exe`。
  - 其他错误：按提示，一般是权限或系统策略。
- **钩子报错**：弹窗或 VS Output 有 `MouseFx: global hook start failed. GetLastError=...`。如果点击的是管理员窗口，请用“以管理员身份运行”启动本程序；安全软件可能拦截钩子。
- **高 DPI 偏位**：启动时已启用 DPI 感知；确保用最新编译的二进制。
- **运行错了二进制**：曾经生成过 `MFCMouseEffect\x64\Debug\...` 的旧输出，现已改为 `x64\Debug\...`。请 Clean + Rebuild 后运行新路径。

## SDI/单窗口说明
- 项目已改为 SDI：一个顶层主窗口承载视图；波纹渲染仍在独立的透明分层窗口中，与主框架解耦。
- 如果需要多个窗口，可按需创建多个独立顶层框架（非 MDI 子窗），互不嵌套。
