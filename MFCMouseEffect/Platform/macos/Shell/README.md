# macOS Shell Package

This folder mirrors Flutter-style platform package partitioning.

Implemented in this stage:
- `SettingsLauncher` (shell `open` command)
- `SingleInstanceGuard` (POSIX file lock in `/tmp`)
- `EventLoopService` (minimal polling loop)
- `TrayService` (shell layer stub; native menu-bar integration pending)
- `UserNotificationService` (`osascript` notification + stderr fallback)

Planned next:
- Native menu-bar icon + menu actions for tray service
- `DpiAwarenessService` equivalent
- Native loop bridge (`CFRunLoop`/AppKit) to replace polling loop

The concrete classes should implement the cross-platform interfaces in:
- `MouseFx/Core/Shell/*`
