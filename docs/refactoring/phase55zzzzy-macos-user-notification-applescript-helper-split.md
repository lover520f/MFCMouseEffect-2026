# Phase 55zzzzy - macOS User Notification AppleScript Helper Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosUserNotificationService.cpp` mixed:
  - warning notification entry logic
  - AppleScript escaping/command building/execution
  - test capture file append internals
- This coupling made notification backend replacement harder.

## What Changed
- Added notification helper contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosUserNotificationService.Internal.h`
- Added AppleScript/test-capture helper module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosUserNotificationService.AppleScript.cpp`
- Simplified notification entry:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosUserNotificationService.cpp`
  - now focuses on title/message normalization and fallback stderr logging.
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `键鼠指示/手势映射` degraded-permission warning notification path.
- Not part of: effects renderer path, WASM renderer path.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- No visible behavior contract changes; notification implementation responsibilities only were split.
