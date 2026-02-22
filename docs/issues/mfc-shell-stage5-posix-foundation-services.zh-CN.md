# MFC 壳解耦阶段 5：macOS/Linux 基础服务落位（EventLoop/Settings/SingleInstance）

## 1. 背景与目标

阶段 4 已将壳主流程抽象为 `AppShellCore`，并完成 Windows 分包接线。  
本阶段目标是先补齐 macOS/Linux 的基础服务能力（可运行最小集），为后续托盘与通知实现铺路。

## 2. 判定

判定：`架构演进`（非功能 Bug）。  
依据：跨平台早期应先打通基础生命周期能力，再逐步补平台 UI 能力，能降低一次性迁移风险。

## 3. 实施内容

### 3.1 macOS 平台包新增

目录：`MFCMouseEffect/Platform/macos/Shell/`

- `MacosEventLoopService.h/.cpp`
  - 最小轮询事件循环（`Run/RequestExit`）。
- `MacosSettingsLauncher.h/.cpp`
  - 通过 `open "<url>"` 打开设置 URL（含基础命令注入字符校验）。
- `MacosSingleInstanceGuard.h/.cpp`
  - 使用 `/tmp` 锁文件 + `flock` 做单实例保护。

### 3.2 Linux 平台包新增

目录：`MFCMouseEffect/Platform/linux/Shell/`

- `LinuxEventLoopService.h/.cpp`
  - 最小轮询事件循环（`Run/RequestExit`）。
- `LinuxSettingsLauncher.h/.cpp`
  - 通过 `xdg-open "<url>"` 打开设置 URL（含基础命令注入字符校验）。
- `LinuxSingleInstanceGuard.h/.cpp`
  - 使用 `/tmp` 锁文件 + `flock` 做单实例保护。

### 3.3 平台工厂接线扩展

文件：`MFCMouseEffect/Platform/PlatformShellServicesFactory.cpp`

- 新增 `__APPLE__` 与 `__linux__` 分支：
  - 注入 `settingsLauncher`
  - 注入 `singleInstanceGuard`
  - 注入 `eventLoopService`

### 3.4 核心壳流程兼容调整

文件：`MFCMouseEffect/MouseFx/Core/Shell/AppShellCore.cpp`

- 初始化必需项由“必须有托盘服务”调整为：
  - 必须：`settingsLauncher + singleInstanceGuard + eventLoopService`
  - 可选：`trayService + dpiAwareness + notifier`
- 当托盘服务缺失时自动进入后台模式，不阻断启动流程。

### 3.5 平台包文档更新

- `Platform/macos/Shell/README.md`
- `Platform/linux/Shell/README.md`

补充“已实现基础服务 + 后续计划”状态说明。

## 4. 验证

Windows 回归构建（保证主线不退化）：

- `Release|x64`：通过
- `Debug|x64`：通过

说明：

- 本阶段新增的 macOS/Linux 文件未纳入当前 Windows 工程编译清单，不影响现有发布产物。

## 5. 风险与边界

已控制：

- 仅补基础能力，不触碰现有 Windows 功能链路。
- 对 URL 启动命令加入基本危险字符过滤，降低 shell 注入风险。

当前边界：

- macOS/Linux 仍未实现托盘服务与用户通知服务。
- 事件循环为最小轮询实现，后续需替换为平台原生循环桥接。

## 6. 后续建议（阶段 6）

- 落地 `macOS/Linux TrayService`（状态栏/托盘）并接通设置入口。
- 落地 `macOS/Linux UserNotificationService`。
- 将 `EventLoopService` 从轮询替换为平台原生主循环适配层。
