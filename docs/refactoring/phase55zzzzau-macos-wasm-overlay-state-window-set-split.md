# Phase 55zzzzau - macOS WASM Overlay State Window-Set Split

## What Changed
- Split window-set ownership operations from overlay-state entry:
  - Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmOverlayState.Windows.mm`
    - `RegisterWasmOverlayWindowStateInternal(...)`
    - `TakeWasmOverlayWindowStateInternal(...)`
    - `ResetAndTakeAllWasmOverlayWindowsStateInternal()`
  - Kept `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmOverlayState.mm` focused on admission/in-flight/throttle wrappers.
- Extended internal contract in:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmOverlayState.Internals.h`
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Why
- Separate window-handle set mutation from quota/throttle state API.
- Lower coupling in WASM overlay runtime state maintenance.

## Behavior Contract
- No behavior change intended.
- Admission, in-flight count, and overlay-window lifecycle semantics stay unchanged.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Four-Capability Mapping
- This change belongs to: `WASM` (runtime overlay state hardening).
