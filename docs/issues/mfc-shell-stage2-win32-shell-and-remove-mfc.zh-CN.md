# MFC 壳解耦阶段 2：纯 Win32 AppShell 替换与 MFC 依赖移除

## 1. 背景与目标

阶段 1 已完成托盘与应用壳接口解耦，并清理了一批历史 MFC UI 编译项。  
本阶段目标是把入口壳彻底改为纯 Win32，去掉 `UseOfMfc`，让主程序不再依赖 `mfc140u.dll`。

## 2. 判定与设计

判定：`架构演进 + 技术债清理`（非功能缺陷）。  
依据：特效链路与设置服务已主要运行在 `MouseFx` 内核和 Win32 路径，MFC 仅残留在入口与托盘承载层，导致运行时依赖和包体开销持续存在。

设计要点：

- 新增 `Win32AppShell` 作为唯一壳层入口（单实例、DPI、OLE、IPC、Web 设置、消息循环）。
- `TrayHostWnd` / `TrayMenuBuilder` 完全改为 Win32 API，不再依赖 `CWnd/CMenu/CString`。
- 工程层面切换为标准 Win32 库，移除 MFC 关键字与使用标记。
- 保持 `MouseFx`、`WebSettingsServer`、Wasm 主链路接口不变，保证迁移风险可控。

## 3. 实施内容

### 3.1 入口改造

- 改写 `MFCMouseEffect/MFCMouseEffect.cpp`
  - 使用 `wWinMain` 直接启动 `mousefx::Win32AppShell`。
  - 删除 `CWinAppEx` 入口依赖。
- 更新 `MFCMouseEffect/MFCMouseEffect.h`
  - 移除旧 MFC App 类声明，保留最小兼容头占位。

### 3.2 壳层实现

- 新增 `MFCMouseEffect/MouseFx/Core/Shell/Win32AppShell.h`
- 新增 `MFCMouseEffect/MouseFx/Core/Shell/Win32AppShell.cpp`
  - 提供：初始化、消息循环、退出回收、托盘交互回调、Web 设置打开。
  - 内置 UTF-8 URL 到 UTF-16 转换，避免 `ShellExecuteW` 路径编码错误。
  - 处理单实例互斥量、OLE 生命周期、IPC 退出联动。

### 3.3 托盘层 Win32 化

- 重写 `MFCMouseEffect/UI/Tray/TrayHostWnd.h`
- 重写 `MFCMouseEffect/UI/Tray/TrayHostWnd.cpp`
  - 用 `RegisterClassExW/CreateWindowExW` + 静态 `WndProc` 承载托盘消息。
  - 托盘右键菜单、命令分发、窗口关闭全走 Win32 消息机制。
- 重写 `MFCMouseEffect/UI/Tray/TrayMenuBuilder.h`
- 重写 `MFCMouseEffect/UI/Tray/TrayMenuBuilder.cpp`
  - 统一使用 `HMENU` + `std::wstring`，移除 `CMenu/CString` 依赖。

### 3.4 工程与资源配置

- 修改 `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - `<Keyword>`：`MFCProj` -> `Win32Proj`
  - `<UseOfMfc>`：全部配置切为 `false`
  - 新增 `Win32AppShell` 源文件编译项
- 修改 `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`
  - 增加新文件过滤器映射
- 修改 `MFCMouseEffect/framework.h`
  - 改为 Win32/COM 头集合，去掉 MFC 头链路
- 修改 `MFCMouseEffect/MFCMouseEffect.rc`
  - 精简为最小图标资源定义，去掉 `afxres` 路径

### 3.5 迁移过程中的编译问题修复

- `WebSettingsServer` 前置声明 + `unique_ptr` 构造期不完整类型问题
  - 将 `Win32AppShell` 构造函数改为 `.cpp` 外置定义，避免头文件中触发析构路径实例化。
- `WebSettingsServer::Url()` 返回 `std::string` 与 `ShellExecuteW` 的 `std::wstring` 不匹配
  - 增加 UTF-8 -> UTF-16 转换后再打开设置页。
- `DispatchRouter.h` 中 `uint8_t` 解析失败
  - 显式补 `#include <cstdint>`，消除编译器解析歧义。

## 4. 验证结果

构建验证：

- `Release|x64`
  - `MSBuild.exe MFCMouseEffect\\MFCMouseEffect.vcxproj /t:Build /p:"Configuration=Release;Platform=x64" /m /nologo /v:minimal`
  - 结果：通过
- `Debug|x64`
  - `MSBuild.exe MFCMouseEffect\\MFCMouseEffect.vcxproj /t:Build /p:"Configuration=Debug;Platform=x64" /m /nologo /v:minimal`
  - 结果：通过

运行时依赖验证：

- `dumpbin /DEPENDENTS x64\\Release\\MFCMouseEffect.exe`
- 结果：不再出现 `mfc140u.dll`

产物体积（阶段 2 完成后）：

- `x64/Release/MFCMouseEffect.exe`：`1,075,200` bytes

## 5. 风险与边界

已控制风险：

- 特效计算/渲染主链路未改，仍由 `MouseFx` 负责。
- 托盘与 Web 设置行为保持原语义。

当前边界：

- 程序仍是 Windows 专用（Win32、D2D/D3D、Hook、托盘 API）。
- 本阶段只完成“去 MFC 壳”；跨平台目标需要在后续阶段继续抽象 OS 适配层。

## 6. 后续建议（阶段 3）

- 以 `IAppShellHost` 为边界继续抽象平台层（托盘、窗口、输入钩子、进程单实例）：
  - `PlatformShell`（生命周期）
  - `TrayService`（托盘与菜单）
  - `InputCaptureService`（全局输入）
  - `SettingsLauncher`（浏览器/内置 WebView 启动）
- 先在 Windows 内部完成“接口稳定 + Win32 实现下沉”，再扩展到非 Windows 目标，避免跨平台重写时牵动核心渲染逻辑。
