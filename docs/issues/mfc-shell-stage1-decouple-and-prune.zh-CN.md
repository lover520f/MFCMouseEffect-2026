# MFC 壳解耦阶段 1：托盘壳接口封装与未使用 UI 编译剥离

## 1. 背景与目标

当前项目的特效计算与渲染主体已在 `MouseFx` 中实现，窗口也主要走 Win32 + D2D/D3D 路径。  
本阶段目标是先做低风险收敛：

- 将托盘层到应用壳的调用改为接口委托，去掉对 `CMFCMouseEffectApp` 的硬耦合。
- 移除已不参与运行路径的 MFC DocView/Panes/旧设置窗编译单元。
- 不改变现有用户可见行为（托盘、Web 设置、特效链路保持一致）。

## 2. 判定与设计

判定：`Bug/架构债`（非功能缺陷）。  
依据：托盘窗口此前通过 `dynamic_cast<CMFCMouseEffectApp*>(AfxGetApp())` 直接访问应用对象，且工程仍编译大量不再走运行路径的 MFC 模板代码，壳层耦合偏高。

设计要点：

- 引入跨壳接口 `mousefx::IAppShellHost` 作为托盘调用边界。
- `CTrayHostWnd` 只依赖接口，不再依赖 `CMFCMouseEffectApp` 具体类型。
- `CMFCMouseEffectApp` 实现该接口，负责设置打开与退出请求。
- 工程层面剥离旧 MFC UI 编译项，保留运行所需托盘壳与核心引擎。

## 3. 实施内容

### 3.1 新增壳接口

- 新增 `MFCMouseEffect/MouseFx/Core/Shell/IAppShellHost.h`
  - `AppControllerForShell()`
  - `OpenSettingsFromShell()`
  - `RequestExitFromShell()`

### 3.2 托盘窗口解耦

- 修改 `MFCMouseEffect/UI/Tray/TrayHostWnd.h`
  - `CreateHost` 新签名：`CreateHost(mousefx::IAppShellHost* shellHost, bool showTrayIcon = true)`
  - 新增 `shellHost_` 非拥有指针
- 修改 `MFCMouseEffect/UI/Tray/TrayHostWnd.cpp`
  - 移除 `AfxGetApp()` + `dynamic_cast<CMFCMouseEffectApp*>` 访问
  - 菜单动作改走 `IAppShellHost` 委托（打开设置、请求退出、获取 `AppController`）

### 3.3 应用入口收敛

- 修改 `MFCMouseEffect/MFCMouseEffect.h`
  - `CMFCMouseEffectApp` 实现 `mousefx::IAppShellHost`
  - 移除旧 `NotifySettingsWndDestroyed` 与 `settingsWnd_` 字段
  - 移除 `m_hRichEditModule` 字段
- 修改 `MFCMouseEffect/MFCMouseEffect.cpp`
  - 移除 `UI/Frame`、`UI/DocView`、旧 `SettingsWnd` 相关 include
  - 托盘创建统一为 `trayHost_->CreateHost(this, showTrayIcon)`（Debug/Release 不再分岔到 MDI 调试主框架）
  - IPC 退出路径统一复用 `RequestExitFromShell()`
  - 移除旧 `NotifySettingsWndDestroyed` 实现与 RichEdit 卸载逻辑

### 3.4 工程编译项剥离

- 修改 `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - 移除未使用 MFC UI 编译项：
    - `UI/Frame/*`
    - `UI/DocView/*`
    - `UI/Panes/*`
    - `UI/Settings/SettingsWnd*`
    - `UI/Settings/TrailTuningWnd*`
    - `UI/Settings/EmojiPreviewWnd*`
    - `Settings/SettingsBackend.cpp`
  - 移除对应 header 项
  - 新增 `MouseFx/Core/Shell/IAppShellHost.h`

## 4. 验证结果

构建验证：

- 命令：  
  `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect\MFCMouseEffect.vcxproj /t:Rebuild /p:Configuration=Release /p:Platform=x64 /m /nologo`
- 结果：`0 error, 0 warning`

产物观察（`x64/Release/MFCMouseEffect.exe`）：

- 变更后大小：`1,191,936` bytes
- 仍导入 `mfc140u.dll`（当前仍是 MFC 壳阶段，下一阶段才是去 `UseOfMfc`）

## 5. 风险与边界

已控制风险：

- 特效计算/渲染链路不改，仅调整壳层调用边界。
- 托盘菜单命令语义不变。

仍保留的边界：

- 应用入口仍为 `CWinAppEx`（MFC 动态库仍存在）。
- 仅完成“壳层解耦 + 编译剥离”；尚未进入“纯 Win32 壳替换”阶段。

## 6. 下一阶段建议（阶段 2）

- 新建纯 Win32 AppShell（`WinMain + message loop + tray host + single instance + IPC exit`）。
- 将 `UseOfMfc` 切到 `Use Standard Windows Libraries`，移除 `mfc140u.dll` 依赖。
- 保持 `MouseFx`、`WebSettingsServer`、`Wasm` 子系统接口不变，保证迁移可回滚。
