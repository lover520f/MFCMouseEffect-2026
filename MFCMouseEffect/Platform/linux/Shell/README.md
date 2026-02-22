# Linux Shell Package

This folder mirrors Flutter-style platform package partitioning.

Implemented in this stage:
- `SettingsLauncher` (shell `xdg-open` command)
- `SingleInstanceGuard` (POSIX file lock in `/tmp`)
- `EventLoopService` (minimal polling loop)
- `TrayService` (shell layer stub; native appindicator integration pending)
- `UserNotificationService` (`notify-send` + stderr fallback)

Planned next:
- `DpiAwarenessService` equivalent
- Native appindicator/status notifier tray integration
- Native loop bridge (GLib/Qt loop adapter) to replace polling loop

The concrete classes should implement the cross-platform interfaces in:
- `MouseFx/Core/Shell/*`
