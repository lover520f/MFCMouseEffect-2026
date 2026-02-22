# Linux Shell Package (Placeholder)

This folder mirrors Flutter-style platform package partitioning.

Planned implementations:
- `TrayService` (status notifier / appindicator)
- `SettingsLauncher` (xdg-open or embedded webview)
- `SingleInstanceGuard`
- `DpiAwarenessService` equivalent
- `EventLoopService`
- `UserNotificationService`

The concrete classes should implement the cross-platform interfaces in:
- `MouseFx/Core/Shell/*`
