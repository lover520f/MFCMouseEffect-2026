# Phase 55zzzzk - macOS Input Indicator Probe Helper Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.Probes.mm` had duplicated probe setup/restore and capture logic.
- Duplication increases maintenance cost for probe contract changes.

## What Changed
- Added probe-helper module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.ProbeHelpers.mm`
- Extended overlay private contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.h`
  - added helper methods for probe config lifecycle and expected-label capture.
- Simplified probe entry file:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.Probes.mm`
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `键鼠指示` probe/diagnostics path.
- Not part of: WASM renderer path, automation mapping path, effect style path.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- Behavior unchanged by design; only probe helper extraction.
