# Phase 55zzzzas - macOS WASM Image Overlay Window Split

## What Changed
- Split macOS WASM image-overlay core responsibilities:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.mm`
    - keeps admission + render-plan + main-thread scheduling orchestration
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Window.mm`
    - owns NSWindow/layer/image composition, motion animation, and timed teardown
- Added shared declaration in:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Internal.h`
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Why
- Reduce coupling between request admission/scheduling and Objective-C window rendering details.
- Keep WASM overlay core entry stable when tuning rendering implementation.

## Behavior Contract
- No behavior change intended.
- Existing WASM image overlay render/admission/throttle semantics stay unchanged.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Four-Capability Mapping
- This change belongs to: `WASM` (runtime overlay rendering path hardening).
