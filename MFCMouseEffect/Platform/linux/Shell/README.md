# Linux Shell Package (Placeholder)

This folder mirrors Flutter-style platform package partitioning.

Implemented in this stage:
- `SettingsLauncher` (shell `xdg-open` command)
- `SingleInstanceGuard` (POSIX file lock in `/tmp`)
- `EventLoopService` (minimal polling loop)

Planned next:
- `TrayService` (status notifier / appindicator)
- `DpiAwarenessService` equivalent
- `UserNotificationService`

The concrete classes should implement the cross-platform interfaces in:
- `MouseFx/Core/Shell/*`
