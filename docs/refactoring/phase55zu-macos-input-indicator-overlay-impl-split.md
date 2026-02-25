# Phase 55zu: macOS Input Indicator Overlay Impl Split

## Capability
- Keyboard/Mouse indicator

## Why
- `MacosInputIndicatorOverlay.mm` mixed multiple concerns:
  - label formatting and string conversion
  - main-thread dispatch helpers
  - overlay lifecycle/render path
  - probe/debug contract path
- This made indicator-path changes expensive to review and easy to couple.

## Scope
- Keep runtime behavior and probe contracts unchanged.
- Split indicator implementation into focused units:
  - render/lifecycle
  - probe/debug and event entry
  - shared helper utilities

## Code Changes

### 1) Shared helper boundary
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.mm`
- Owns:
  - label formatting (`L/R/M`, scroll label, key label)
  - clamp helper
  - mac main-thread dispatch wrappers and UTF-8 -> `NSString` conversion

### 2) Overlay main file focused on lifecycle/render
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.mm`
- Keeps:
  - `Initialize/Shutdown/Hide/UpdateConfig`
  - `ShowAt`
  - `ShouldShowKeyboard`

### 3) Probe and event entry moved out
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.Probes.mm`
- Owns:
  - `OnClick/OnScroll/OnKey`
  - `ReadDebugState`
  - `RunMouseLabelProbe`
  - `RunKeyboardLabelProbe`

### 4) Build wiring
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`
- Added new overlay implementation units to `mfx_shell_macos`.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Contract Impact
- No API/schema change.
- No behavior change in indicator rendering or probe expectations (`L/R/M`, `A`, `Cmd+K9`, `K6`).
- Internal decoupling only, for safer future evolution on macOS indicator path.
