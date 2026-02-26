# Phase 55zzzzaa - macOS Keyboard Injector EventPost Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosKeyboardInjector.mm` mixed:
  - chord flow orchestration
  - dry-run environment parsing and HID key-event posting internals
- This increases coupling in automation injection pipeline changes.

## What Changed
- Added keyboard-injector helper contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosKeyboardInjector.Internal.h`
- Added event-post helper module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosKeyboardInjector.EventPost.mm`
  - owns dry-run flag parsing and low-level key-event posting.
- Simplified injector entry:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosKeyboardInjector.mm`
  - now keeps parse/resolve/chord-flow orchestration and delegates event-post internals.
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `手势映射/自动化` (keyboard injection path).
- Not part of: effects renderer, input-indicator overlay, WASM renderer.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- No behavior contract changes; injector internals were split only.
