# Phase 55zzzzbj - macOS User Notification Swift Bridge

## Background
- macOS warning notification path previously used C++ + `std::system("osascript ...")`.
- The project direction is Swift-first for new macOS platform closure work.
- Notification service is a low-risk boundary with clear IO contract (`title/message -> notify/fallback`).

## Decision
- Keep C++ service interface unchanged (`IUserNotificationService`).
- Move notification execution to Swift via a C ABI bridge:
  - Swift exports: `mfx_macos_show_warning_notification(...)`
  - C++ calls bridge and preserves stderr fallback behavior.

## Implementation
1. New Swift bridge
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosUserNotificationBridge.swift`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosUserNotificationSwiftBridge.h`
- Swift implementation details:
  - uses `Process` to run `/usr/bin/osascript -e <script>`
  - returns success via termination status
  - keeps stdout/stderr silent

2. C++ service call path
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosUserNotificationService.cpp`
- Behavior:
  - still sanitizes title/message
  - still writes test capture (`MFX_TEST_NOTIFICATION_CAPTURE_FILE`)
  - now calls Swift bridge instead of C++ AppleScript execution helper
  - still falls back to `stderr` when notification execution fails

3. Helper cleanup
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosUserNotificationService.Internal.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosUserNotificationService.AppleScript.cpp`
- Removed obsolete C++ shell-command execution helper while preserving string escape + capture helpers.

4. Build wiring
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`
- On Apple host:
  - `enable_language(Swift)`
  - adds Swift bridge source to `mfx_shell_macos`

## Validation
- Build/regression checks (macOS):
  - `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
  - `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`

## Impact
- Capability: `键鼠指示/手势映射/特效/WASM 的公共壳层通知能力`
- User-visible behavior is unchanged (same warning semantics and fallback).
- Windows/Linux behavior is unchanged.

## Follow-up
- Native-center-first delivery refinement is tracked in:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbm-macos-user-notification-native-center.md`
