# Phase 55zzzzz - macOS Input Indicator Show Plan Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.mm` mixed:
  - overlay lifecycle/render dispatch
  - position/size/duration planning logic for each indicator show event
- This coupling increases change risk in indicator presentation tuning.

## What Changed
- Added show-plan contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.ShowPlan.h`
- Added show-plan implementation:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.ShowPlan.mm`
  - owns `positionMode` normalization and clamped size/duration planning.
- Simplified overlay entry:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.mm`
  - now consumes `IndicatorShowPlan` and keeps lifecycle/presentation flow only.
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `键鼠指示` overlay presentation path.
- Not part of: effects rendering, automation matcher, WASM runtime rendering.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- No behavior contract changes; indicator show-plan responsibilities only were split.
