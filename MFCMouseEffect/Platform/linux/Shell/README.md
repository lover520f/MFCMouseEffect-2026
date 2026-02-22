# Linux Shell Package

This folder mirrors Flutter-style platform package partitioning.

Implemented in this stage:
- `SettingsLauncher` (shell `xdg-open` command)
- `SingleInstanceGuard` (POSIX file lock in `/tmp`)
- `EventLoopService` (POSIX blocking loop, no polling, supports `PostTask`)
- `TrayService` (shell layer stub; native appindicator integration pending)
- `UserNotificationService` (`notify-send` + stderr fallback)

Planned next:
- `DpiAwarenessService` equivalent
- Native appindicator/status notifier tray integration
- Native loop bridge (GLib/Qt loop adapter) to replace generic POSIX blocking loop

The concrete classes should implement the cross-platform interfaces in:
- `MouseFx/Core/Shell/*`
