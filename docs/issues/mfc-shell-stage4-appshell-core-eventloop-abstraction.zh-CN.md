# MFC 壳解耦阶段 4：AppShellCore 与 EventLoop 抽象

## 1. 背景与目标

阶段 3 已把平台能力拆成分包接口，但主壳流程仍集中在 `Win32AppShell`。  
本阶段目标是进一步把“应用壳主流程”下沉到平台无关核心：

- 新增 `AppShellCore` 负责统一生命周期与业务编排。
- 新增 `IEventLoopService`，把消息循环也纳入平台抽象。
- `Win32AppShell` 退化为轻量入口封装（仅命令行参数解析 + 注入平台服务）。

## 2. 判定

判定：`架构演进`（非功能 Bug）。  
依据：若主流程继续留在 Win32 壳，后续 macOS/Linux 会重复实现同样逻辑并放大回归面。

## 3. 设计要点

- `AppShellCore`（平台无关）职责：
  - 单实例保护
  - DPI 初始化触发
  - 托盘服务启动/停止
  - `AppController` 与 `IpcController` 生命周期
  - Web 设置服务打开
  - 退出请求路由
- `IEventLoopService` 提供统一循环入口：
  - `Run()`
  - `RequestExit()`
- `IUserNotificationService` 统一警告提示，避免核心层直接依赖 `MessageBoxW`。

## 4. 实施内容

### 4.1 新增核心壳流程

- `MFCMouseEffect/MouseFx/Core/Shell/AppShellCore.h`
- `MFCMouseEffect/MouseFx/Core/Shell/AppShellCore.cpp`

### 4.2 新增接口

- `MFCMouseEffect/MouseFx/Core/Shell/IEventLoopService.h`
- `MFCMouseEffect/MouseFx/Core/Shell/IUserNotificationService.h`

### 4.3 平台服务聚合扩展

- `MFCMouseEffect/MouseFx/Core/Shell/ShellPlatformServices.h`
  - 新增：
    - `eventLoopService`
    - `notifier`

### 4.4 Windows 分包实现扩展

- `MFCMouseEffect/Platform/windows/Shell/Win32EventLoopService.h/.cpp`
- `MFCMouseEffect/Platform/windows/Shell/Win32UserNotificationService.h/.cpp`
- `MFCMouseEffect/Platform/PlatformShellServicesFactory.cpp`
  - 工厂新增上述服务注入。

### 4.5 Win32 壳入口瘦身

- `MFCMouseEffect/MouseFx/Core/Shell/Win32AppShell.h/.cpp`
  - 改为薄封装：
    - 解析 `-mode background`
    - 构造 `AppShellCore`
    - 转发 `Initialize/RunMessageLoop/Shutdown`

## 5. 工程接线

- 更新 `MFCMouseEffect/MFCMouseEffect.vcxproj`
- 更新 `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`
- 新增核心与平台实现文件编译项。

## 6. 验证结果

构建验证：

- `Release|x64`：通过
- `Debug|x64`：通过

运行依赖回归：

- `dumpbin /DEPENDENTS x64\\Release\\MFCMouseEffect.exe`
- 未出现 `mfc140*.dll` 回归

## 7. 风险与边界

已控制：

- 用户可见行为不变（托盘、设置打开、退出路径保持一致）。
- Windows 路径继续稳定可运行。

当前边界：

- macOS/Linux 仍是分包占位，尚未实现具体系统 API 适配。

## 8. 后续建议（阶段 5）

- 先实现 macOS/Linux 的 `EventLoopService + SettingsLauncher + SingleInstanceGuard`。
- 再逐步接入托盘实现，保持核心 `AppShellCore` 不变。
