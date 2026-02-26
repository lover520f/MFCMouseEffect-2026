# Phase 55zzzzp - macOS Global Input Hook Dispatch Mouse/Key Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosGlobalInputHook.EventTapDispatch.mm` mixed:
  - tap-disabled recovery
  - mouse dispatch path
  - keyboard dispatch path
- Mixed ownership increases regression risk in the core input/automation path.

## What Changed
- Kept tap-disabled recovery in:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosGlobalInputHook.EventTapDispatch.mm`
- Added mouse dispatch module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosGlobalInputHook.EventTapDispatch.Mouse.mm`
- Added key dispatch module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosGlobalInputHook.EventTapDispatch.Key.mm`
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `手势映射/自动化` + `键鼠指示` input ingress path.
- Not part of: effect renderer style path, WASM runtime renderer path.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- No user-visible behavior contract changes; dispatch implementation ownership split only.
