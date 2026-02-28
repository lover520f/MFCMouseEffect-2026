# Phase 45: macOS Shell Host Loop + Tray Implementation

## Top Decisions (Important)
- Keep macOS shell package boundaries intact: event loop, tray, DPI, smoke checks stay under `Platform/macos/Shell`.
- Use native Apple host implementations (`CFRunLoopSource` and `NSStatusBar`) while preserving cross-host fallback stubs.
- Keep POSIX entry/scaffold behavior unchanged; this stage focuses on shell host service reliability.

## Code Changes
1. `MacosEventLoopService`:
   - Apple host path switched to `CFRunLoop` + `CFRunLoopSource` with task queue drain.
   - supports `PostTask`, `RequestExit`, and run-loop wake-up semantics in one service.
   - non-Apple path keeps `PosixBlockingEventLoop` fallback.
2. `MacosTrayService`:
   - Apple host implementation moved to Objective-C++ file (`MacosTrayService.mm`).
   - native tray menu with `Settings` / `Exit` actions bridged through `IAppShellHost`.
   - cross-host fallback remains in `MacosTrayService.cpp`.
3. Added macOS shell package support units:
   - `MacosDpiAwarenessService.*` (no-op host adapter)
   - `MacosShellSmokeMain.cpp`
   - `MacosTraySmokeMain.cpp`

## Compatibility
- No Windows path changes.
- POSIX scaffold API/static route behavior unchanged.
- Cross-host build behavior preserved via fallback `.cpp` implementations.

## Validation
- Executed:
  - `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto --build-dir /tmp/mfx-platform-macos-build`
- Result:
  - build passed
  - macOS smoke checks passed
  - scaffold HTTP checks passed

## Follow-up
- Completed in Phase 47:
  - extracted tray menu localization policy into `MacosTrayMenuLocalization.*` and wired localized menu labels for `zh*` preferred language.
