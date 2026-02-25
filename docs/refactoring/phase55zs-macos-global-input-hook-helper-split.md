# Phase 55zs: macOS Global Input Hook Helper Split

## Why
- `/Platform/macos/System/MacosGlobalInputHook.mm` had mixed responsibilities:
  - event classification/point conversion
  - permission simulation parsing and trust checks
  - hook lifecycle and dispatch integration
- This increased file size and change-coupling risk.

## Scope
- Keep runtime behavior unchanged.
- Split reusable helper logic into dedicated modules:
  - input event utility helpers
  - permission simulation/trust helpers
- Keep hook lifecycle and dispatch flow in `MacosGlobalInputHook`.

## Code Changes

### 1) New event utility module
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosInputEventUtils.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosInputEventUtils.mm`
- Extracted:
  - mouse event-type classification
  - mouse button mapping
  - screen-point conversion

### 2) New permission state module
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosInputPermissionState.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosInputPermissionState.mm`
- Extracted:
  - simulation-file parsing (`trusted=...`)
  - runtime trust determination (`AXIsProcessTrusted` + simulation override)

### 3) Hook implementation rewired to helpers
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosGlobalInputHook.mm`
- Behavior remains the same; only helper ownership moved.

### 4) Build wiring
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`
- Added new helper `.mm` files to `mfx_shell_macos` sources.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Contract Impact
- No API/schema/behavior changes.
- Internal structure only:
  - lower coupling in macOS global-input hook path,
  - easier future maintenance for permission logic and event mapping.
