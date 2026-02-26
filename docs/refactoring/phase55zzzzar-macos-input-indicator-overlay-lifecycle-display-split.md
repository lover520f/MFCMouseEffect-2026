# Phase 55zzzzar - macOS Input Indicator Overlay Lifecycle/Display Split

## What Changed
- Split `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.mm` by responsibility:
  - Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.Lifecycle.mm`
    - `Initialize()`
    - `Shutdown()`
  - Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.Display.mm`
    - `ShowAt(...)`
    - `Hide()`
  - Kept `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.mm` as thin entry (`destructor`, config update, keyboard gate).
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Why
- Reduce coupling in the keyboard/mouse indicator pipeline.
- Keep lifecycle resource management separate from display scheduling/animation dispatch.

## Behavior Contract
- No behavior change intended.
- Indicator lifecycle, label rendering, and auto-hide timing semantics are preserved.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Four-Capability Mapping
- This change belongs to: `键鼠指示` (input indicator pipeline hardening).
