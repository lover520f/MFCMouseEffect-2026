# MFC 壳解耦阶段 33：Windows 托盘实现平台边界收口

## 1. 背景与目标

阶段 32 后，托盘功能仍存在边界反向依赖：

- `Platform/windows/Shell/Win32TrayService.h` 直接 include `UI/Tray/TrayHostWnd.h`
- 托盘菜单与托盘窗口实现仍放在 `UI/Tray/*`

这会让 `Platform` 层依赖 `UI` 目录，和跨平台分层目标冲突。

本阶段目标：

- 将托盘实现物理迁移到 `Platform/windows/Shell/Tray`；
- 统一托盘实现命名为 `Win32Tray*`；
- 让 `Win32TrayService` 头文件仅依赖抽象接口，移除对具体窗口实现的头文件依赖。

## 2. 判定

判定：`架构演进`（边界治理，行为保持不变）。

依据：

- 菜单命令、托盘事件、设置入口与退出流程均沿用原逻辑；
- 改动集中在目录/命名/依赖方向，不改变业务功能。

## 3. 实施内容

### 3.1 文件迁移与命名统一

从：

- `MFCMouseEffect/UI/Tray/TrayHostWnd.h/.cpp`
- `MFCMouseEffect/UI/Tray/TrayMenuBuilder.h/.cpp`
- `MFCMouseEffect/UI/Tray/TrayMenuCommands.h`

迁移到：

- `MFCMouseEffect/Platform/windows/Shell/Tray/Win32TrayHostWindow.h/.cpp`
- `MFCMouseEffect/Platform/windows/Shell/Tray/Win32TrayMenuBuilder.h/.cpp`
- `MFCMouseEffect/Platform/windows/Shell/Tray/Win32TrayMenuCommands.h`

并统一类名：

- `CTrayHostWnd` -> `Win32TrayHostWindow`
- `TrayMenuBuilder` -> `Win32TrayMenuBuilder`

### 3.2 Win32TrayService 头文件去耦

文件：

- `MFCMouseEffect/Platform/windows/Shell/Win32TrayService.h`
- `MFCMouseEffect/Platform/windows/Shell/Win32TrayService.cpp`

变更：

- `Win32TrayService.h` 移除 `UI/Tray/TrayHostWnd.h` include；
- 使用前置声明 + `std::unique_ptr<Win32TrayHostWindow>`（pimpl 样式）持有具体实现；
- 具体实现 include 下沉到 `Win32TrayService.cpp`。

### 3.3 工程项同步

文件：

- `MFCMouseEffect/MFCMouseEffect.vcxproj`
- `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

变更：

- 托盘编译单元与头文件路径改为 `Platform/windows/Shell/Tray/*`；
- 过滤器映射从 `UI/Tray` 收口到 `Platform`。

## 4. 验证

构建命令（VS 2026 x64）：

- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Debug;Platform=x64" /m`
- `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build "/p:Configuration=Release;Platform=x64" /m`

结果：

- `Debug|x64`：通过
- `Release|x64`：通过

## 5. 风险与边界

已控制：

- `git mv` 迁移，保留历史；
- 托盘菜单命令 ID 与处理逻辑未改变。

当前剩余：

- `Win32AppShell` 仍位于 `MouseFx/Core/Shell`；
- `InputTypesWin32.h` 仍位于 `MouseFx/Core/Protocol`。

下一阶段应继续收口上述 Windows 特定入口与类型适配位置。
