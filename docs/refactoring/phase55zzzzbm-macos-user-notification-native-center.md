# Phase 55zzzzbm - macOS User Notification Native Center Path

## Background
- Swift bridge already replaced C++ `std::system(...)` notification dispatch.
- Current Swift bridge still shells out to `osascript`, which can surface non-app icon identity and extra process side effects.

## Decision
- Keep the existing C ABI bridge contract unchanged:
  - `mfx_macos_show_warning_notification(title, message) -> bool`
- Change bridge internals to:
  1. Prefer native macOS notification center delivery (`UserNotifications` on macOS 11+).
  2. Use legacy `NSUserNotificationCenter` on older macOS.
  3. Fallback to AppleScript (`osascript`) only when native paths are unavailable.

## Implementation
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosUserNotificationBridge.swift`
- Behavior:
- Native path:
    - macOS 11+: `UNUserNotificationCenter` + `UNMutableNotificationContent`
    - older macOS: `NSUserNotificationCenter`
    - for unbundled host binaries (for example `/tmp/.../mfx_entry_posix_host`), `UNUserNotificationCenter` is explicitly bypassed via bundle-identifier guard to avoid process crash on startup
  - Fallback path:
    - preserve existing AppleScript dispatch behavior
    - keep escaped title/message handling before script execution
- C++ call sites and test-capture semantics are unchanged.

## Validation
- `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`

## Impact
- Capability: `特效/键鼠指示/手势映射/WASM 公共壳层告警通知`
- User-visible:
  - warning notifications keep existing trigger timing/message semantics
  - notification delivery path is now native-first on macOS
- Windows/Linux behavior unchanged.
