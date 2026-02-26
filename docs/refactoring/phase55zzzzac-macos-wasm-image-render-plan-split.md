# Phase 55zzzzac - macOS WASM Image Render Plan Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.mm` mixed:
  - render orchestration
  - show-plan computation (`position/size/duration/delay/alpha`)
- This coupling increases risk when tuning image overlay behavior.

## What Changed
- Added image-render plan contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Internal.h`
- Added plan helper module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Plan.mm`
  - owns screen->overlay mapping and clamped render-plan computation.
- Simplified renderer core:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.mm`
  - now consumes `ImageOverlayRenderPlan` and keeps rendering orchestration.
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `WASM` image-overlay render path.
- Not part of: automation matcher, input-indicator overlay rendering, effect style rendering.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- No behavior contract change; render-plan responsibilities were split only.
