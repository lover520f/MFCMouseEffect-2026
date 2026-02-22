# MFC 壳解耦阶段 3：跨平台接口与 Flutter 式平台分包骨架

## 1. 背景与目标

阶段 2 已完成去 MFC 壳，当前仍是 Windows 单实现。  
本阶段目标是先把平台能力抽成稳定接口，并按 `windows/macos/linux` 分包布局，为后续跨平台迁移预留结构。

## 2. 判定

判定：`架构演进`（非功能 Bug）。  
依据：托盘、URL 打开、单实例、DPI 感知属于典型平台相关能力，若不先抽象，后续跨平台会反复改壳层主流程并提高回归风险。

## 3. 设计原则

- 核心壳层流程（`Win32AppShell`）只依赖抽象接口，不直接依赖平台细节类型。
- 平台实现按分包放置，风格对齐 Flutter 的 `windows/macos/linux` 分目录。
- 先保证 Windows 行为不变，macOS/Linux 先建立占位包与契约，不引入假实现。

## 4. 实施内容

### 4.1 新增跨平台接口层（Core）

目录：`MFCMouseEffect/MouseFx/Core/Shell/`

- `ITrayService.h`
- `ISettingsLauncher.h`
- `ISingleInstanceGuard.h`
- `IDpiAwarenessService.h`
- `ShellPlatformServices.h`（平台服务聚合）

### 4.2 新增平台工厂与 Windows 分包实现

目录：`MFCMouseEffect/Platform/`

- `PlatformShellServicesFactory.h/.cpp`

目录：`MFCMouseEffect/Platform/windows/Shell/`

- `Win32TrayService.h/.cpp`
- `Win32SettingsLauncher.h/.cpp`
- `Win32SingleInstanceGuard.h/.cpp`
- `Win32DpiAwarenessService.h/.cpp`

### 4.3 macOS/Linux 分包占位

- `MFCMouseEffect/Platform/macos/Shell/README.md`
- `MFCMouseEffect/Platform/linux/Shell/README.md`

说明：当前仅建立目录与职责说明，不引入未实现代码路径，避免误用。

### 4.4 壳层主流程改造（不改行为）

文件：`MFCMouseEffect/MouseFx/Core/Shell/Win32AppShell.h/.cpp`

- 从“直接持有 Win32 细节对象”改为“持有接口对象”：
  - 托盘
  - URL 打开
  - 单实例
  - DPI 感知
- 通过 `platform::CreateShellPlatformServices()` 注入默认 Windows 实现。
- 运行流程（初始化、消息循环、IPC、退出）保持原语义。

### 4.5 托盘宿主小接口增强

文件：`MFCMouseEffect/UI/Tray/TrayHostWnd.h/.cpp`

- 新增 `RequestExit()`，避免外层依赖 HWND 细节。
- `Win32TrayService` 通过该方法触发退出请求。

## 5. 工程接线

- 更新 `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - 新增上述接口与平台实现文件编译项
- 更新 `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`
  - 新增 `头文件\\Platform`、`源文件\\Platform` 过滤器分组

## 6. 验证结果

构建验证：

- `Release|x64`：通过
- `Debug|x64`：通过

运行依赖回归：

- `dumpbin /DEPENDENTS x64\\Release\\MFCMouseEffect.exe`
- 未出现 `mfc140*.dll` 回归

## 7. 风险与边界

已控制：

- 当前功能行为不变，仅做职责下沉与结构重组。
- 平台接口已稳定，后续可按包补齐实现。

当前边界：

- 核心仍是 Windows 产物；macOS/Linux 仅完成目录与契约落位，未开始功能实现。

## 8. 后续建议（阶段 4）

- `platform/macos`：先落地 `SettingsLauncher + SingleInstanceGuard`，再接托盘。
- `platform/linux`：先落地 `SettingsLauncher + SingleInstanceGuard`，托盘根据发行版差异选实现。
- 引入 CI 维度的“平台包编译检查”（即使先是 stub 也保证接口一致性）。
