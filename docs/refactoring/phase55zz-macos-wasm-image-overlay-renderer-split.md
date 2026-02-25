# Phase 55zz: macOS WASM Image Overlay Renderer Split

## Capability
- WASM

## Why
- `MacosWasmTransientOverlay.mm` mixed public API facade and full image-overlay rendering internals.
- Color/math clamp, image loading, window composition, and motion scheduling were coupled in one file.

## Scope
- Keep wasm image overlay behavior unchanged.
- Extract image overlay rendering to dedicated module.
- Keep transient overlay file as API facade.

## Code Changes

### 1) New image overlay renderer module
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmImageOverlayRenderer.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmImageOverlayRenderer.mm`
- Owns:
  - overlay slot admission handling
  - ARGB/color/alpha/scale/lifetime normalization
  - native image/ring composition and animation scheduling
  - motion trajectory interpolation and deferred close

### 2) Transient overlay facade simplification
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmTransientOverlay.mm`
- Keeps:
  - `ShowWasmImageOverlay` entry delegation
  - pulse wrapper
  - close-all facade

### 3) Build wiring
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Contract Impact
- No API/schema behavior change.
- Existing wasm image overlay render and throttle semantics unchanged.
