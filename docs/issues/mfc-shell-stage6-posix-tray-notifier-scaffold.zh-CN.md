# MFC 壳解耦阶段 6：macOS/Linux Tray 与通知服务接线（基础版）

## 1. 背景与目标

阶段 5 已完成 macOS/Linux 的基础服务（`SettingsLauncher`、`SingleInstanceGuard`、`EventLoopService`）。
本阶段目标是补齐壳层服务集合中的另外两项：

- `TrayService`
- `UserNotificationService`

并保证退出链路在“托盘实现尚未原生化”时仍然稳定可用。

## 2. 判定

判定：`架构演进`（非 Bug 修复）。

依据：

- 现阶段重点是把跨平台服务边界补完整，先让依赖关系与生命周期闭环；
- 原生托盘 UI（macOS menu-bar、Linux appindicator）属于下一阶段的“平台增强”。

## 3. 实施内容

### 3.1 macOS 新增

目录：`MFCMouseEffect/Platform/macos/Shell/`

- `MacosTrayService.h/.cpp`
  - 提供 `ITrayService` 的壳层实现（当前为可启动 stub，后续替换为原生菜单栏实现）。
- `MacosUserNotificationService.h/.cpp`
  - 通过 `osascript` 触发通知，失败时回落到 `stderr`。

### 3.2 Linux 新增

目录：`MFCMouseEffect/Platform/linux/Shell/`

- `LinuxTrayService.h/.cpp`
  - 提供 `ITrayService` 的壳层实现（当前为可启动 stub，后续替换为 appindicator/status-notifier 实现）。
- `LinuxUserNotificationService.h/.cpp`
  - 通过 `notify-send` 触发通知，失败时回落到 `stderr`。

### 3.3 工厂接线扩展

文件：`MFCMouseEffect/Platform/PlatformShellServicesFactory.cpp`

- `__APPLE__` 分支新增注入：
  - `MacosTrayService`
  - `MacosUserNotificationService`
- `__linux__` 分支新增注入：
  - `LinuxTrayService`
  - `LinuxUserNotificationService`

### 3.4 退出链路稳态修正

文件：`MFCMouseEffect/MouseFx/Core/Shell/AppShellCore.cpp`

- `RequestExitFromShell()` 调整为：
  - 若有 tray 且非后台模式，先调用 `trayService_->RequestExit()`
  - 然后始终调用 `eventLoopService_->RequestExit()`

这样可以避免“托盘仍是 stub 时无法驱动事件循环退出”的问题。

### 3.5 平台包文档同步

- `MFCMouseEffect/Platform/macos/Shell/README.md`
- `MFCMouseEffect/Platform/linux/Shell/README.md`

更新为“已实现基础版 tray/notifier + 下一阶段原生化计划”。

## 4. 验证

Windows 回归构建：

- `Release|x64`：通过
- `Debug|x64`：通过

说明：

- 本阶段新增 macOS/Linux 文件不参与当前 Windows 产物编译；
- Windows 主线行为保持不变。

## 5. 风险与边界

已控制：

- 跨平台壳层接口已完整接线，后续可逐个把 stub 替换成原生实现。
- 通知服务在外部命令不可用时不会中断主流程（stderr fallback）。

当前边界：

- macOS/Linux tray 仍不是原生图标与菜单；
- macOS/Linux 事件循环仍是轮询实现，后续需对接原生循环。

## 6. 后续建议（阶段 7）

- macOS：基于 AppKit/NSStatusBar 落地原生托盘。
- Linux：基于 appindicator/status-notifier 落地原生托盘。
- 将 macOS/Linux `EventLoopService` 替换为平台原生主循环桥接。
