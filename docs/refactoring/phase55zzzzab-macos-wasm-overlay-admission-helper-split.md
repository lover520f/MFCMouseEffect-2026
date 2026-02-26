# Phase 55zzzzab - macOS WASM Overlay Admission Helper Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmOverlayState.mm` mixed:
  - state API wrappers
  - admission/throttle decision logic
  - reset counters/timestamps logic
- This coupling increases risk when adjusting WASM overlay admission policy.

## What Changed
- Added overlay-state helper contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmOverlayState.InternalHelpers.h`
- Added admission/reset helper module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmOverlayState.Admission.mm`
  - owns slot admission decision and state-reset internals.
- Simplified state API file:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmOverlayState.mm`
  - now keeps mutex-guarded API wrappers and delegates admission/reset internals.
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `WASM` overlay admission/throttle state path.
- Not part of: automation matcher, input-indicator overlay, effect style rendering.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- No behavior contract change; admission/reset responsibilities were split only.
