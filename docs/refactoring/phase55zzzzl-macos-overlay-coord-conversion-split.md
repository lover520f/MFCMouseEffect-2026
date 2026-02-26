# Phase 55zzzzl - macOS Overlay Coord Conversion Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosOverlayCoordSpaceService.mm` mixed service state/origin logic with Quartz->Cocoa conversion internals.
- Mixing service and conversion internals increases coordinate-path maintenance risk.

## What Changed
- Added dedicated conversion module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosOverlayCoordSpaceConversion.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosOverlayCoordSpaceConversion.mm`
- Simplified service module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosOverlayCoordSpaceService.mm`
  - now focuses on overlay origin state + service API and delegates conversion to helper module.
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `特效` + `键鼠指示` shared overlay coordinate conversion path.
- Not part of: WASM runtime rendering path, automation mapping path.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- Behavior unchanged by design; conversion internals moved out of service file.
