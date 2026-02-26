# Phase 55zzzza - macOS WASM Overlay Render Math Consolidation

## Why
- `MacosWasmImageOverlayRendererCore.mm` and `MacosWasmTextOverlay.mm` had duplicated clamp/color conversion math.
- Duplicated render math increases drift risk when tuning WASM overlay behavior.

## What Changed
- Added shared render-math module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmOverlayRenderMath.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmOverlayRenderMath.mm`
- Moved reusable helpers into shared module:
  - float clamp (`ClampFloat`)
  - scale clamp (`ClampScale`)
  - overlay lifetime clamp (`ClampLifeMs`)
  - ARGB -> `NSColor` conversion (`ColorFromArgb`)
- Rewired callers to shared helpers:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmTextOverlay.mm`
- Added new source to macOS build:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `WASM` (overlay command rendering path).
- Not part of: keyboard/mouse indicator, gesture mapping, native effect engine.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- Behavior intent is unchanged; only shared math extraction and call-site rewiring.
