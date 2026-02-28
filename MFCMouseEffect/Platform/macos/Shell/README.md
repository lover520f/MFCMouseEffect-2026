# macOS Shell Package

This folder mirrors Flutter-style platform package partitioning.

Implemented in this stage:
- `DpiAwarenessService` (no-op adapter; macOS already uses device-independent screen coordinates by default)
- `SettingsLauncher` (shared POSIX spawn runner + macOS `open` command)
- `SingleInstanceGuard` (POSIX file lock in `/tmp`; cross-host scaffolding build uses process-local stub)
- `EventLoopService` (`CFRunLoopSource` + task queue dispatch on Apple host; non-Apple cross-host scaffolding build falls back to POSIX blocking loop)
- `TrayService` (Apple host: Swift bridge `NSStatusBar` + menu actions `Settings` / `Exit`, shortcuts `Cmd+,` and `Cmd+Q`; non-Apple cross-host scaffolding build keeps stub)
- `Tray menu localization` (Swift bridge; auto-select Chinese labels when macOS preferred language is `zh*`)
- `Event loop bridge` (Swift bridge owns `NSApplication` lifecycle operations)
- `UserNotificationService` (Swift bridge dispatch via `osascript` + stderr fallback)
- smoke executables for host validation:
  - `MacosShellSmokeMain.cpp` (event-loop/task/exit chain)
  - `MacosTraySmokeMain.cpp` (tray bootstrap + host exit callback)

Planned next:
- Tray icon asset/theme adaptation (currently text button `MFX`)
- Tray icon and shortcut refinements

The concrete classes should implement the cross-platform interfaces in:
- `MouseFx/Core/Shell/*`

Package scaffold target (from `Platform/CMakeLists.txt`):
- `mfx_shell_macos`
- `mfx_shell_macos_smoke` (host-only smoke executable)
- `mfx_shell_macos_tray_smoke` (host-only tray smoke executable)
- `mfx_entry_posix_host` (host-only `PlatformEntryRunner` executable, supports `-mode=background|tray`)

Package source wiring:
- `Platform/macos/CMakeLists.txt`
- `Platform/macos/Shell/MacosShellServicesFactory.*`
- `Platform/macos/Shell/MacosDpiAwarenessService.*`
- `Platform/macos/Shell/MacosEventLoopService.*`
- `Platform/macos/Shell/MacosTrayService.cpp` (Apple host thin adapter + cross-host fallback)
- `Platform/macos/Shell/MacosTrayMenuFactory.cpp` + `MacosTrayMenuBridge.swift` (Swift-owned tray menu create/release/actions)
- `Platform/macos/Shell/MacosEventLoopBridge.cpp` + `MacosEventLoopBridge.swift` (Swift-owned app lifecycle bridge)
- `Platform/macos/Shell/MacosTrayMenuLocalization.*` (tray menu text localization policy)
- shared scaffold settings modules for macOS/Linux entry host:
  - `Platform/posix/Shell/PosixScaffoldAppShell.*` (scaffold shell lifecycle + stdin exit monitor)
  - `Platform/posix/Shell/ScaffoldSettingsRuntime.*` (lifecycle)
  - `Platform/posix/Shell/ScaffoldSettingsRequestHandler.*` (HTTP dispatch + state guard)
  - `Platform/posix/Shell/ScaffoldSettingsRouteConfig.*` (route parse)
  - `Platform/posix/Shell/ScaffoldSettingsApi.*` (JSON payload + patch validation)
  - `Platform/posix/Shell/ScaffoldSettingsWebUiAssets.*` (static WebUI loading)

Quick host checks:
- event loop smoke: `/tmp/mfx-platform-macos-build/platform_macos/mfx_shell_macos_smoke`
- tray smoke: `/tmp/mfx-platform-macos-build/platform_macos/mfx_shell_macos_tray_smoke`
- entry runner background mode: `printf 'exit\n' | /tmp/mfx-platform-macos-build/mfx_entry_posix_host -mode=background`
- one-command regression from repo root: `./tools/platform/regression/run-posix-scaffold-regression.sh --platform macos`

Scaffold settings route convention:
- default URL opened by tray `Settings`: `http://127.0.0.1:9527/?token=scaffold` (served by shared `HttpServer` scaffold settings server)
- static page serving prefers existing Svelte WebUI assets (`WebUI/index.html` + `*.svelte.js`) following Windows WebUI pattern
- no HTML fallback in scaffold mode; missing WebUI assets will return `503`
- optional override for WebUI disk root: `MFX_SCAFFOLD_WEBUI_DIR=/abs/path/to/WebUI`
- scaffold JSON routes:
  - `GET /api/health?token=...`
  - `GET /api/schema?token=...`
  - `GET /api/state?token=...`
  - `POST /api/state?token=...` (in-memory state patch for scaffold UI linkage, no persistence)
- override URL via env: `MFX_SCAFFOLD_SETTINGS_URL=http://127.0.0.1:18080/?token=dev /tmp/mfx-platform-macos-build/mfx_entry_posix_host -mode=tray`
