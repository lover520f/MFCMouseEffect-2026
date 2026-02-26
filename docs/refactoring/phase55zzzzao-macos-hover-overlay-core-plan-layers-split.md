# Phase 55zzzzao - macOS Hover Overlay Core Plan and Layers Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.mm` mixed:
  - render plan derivation (`type/size/frame`)
  - ring layer style setup
  - tubes extra layer setup and spin animation
  - orchestration flow
- This coupling increased change risk in hover effect evolution.

## What Changed
- Added internal contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Internal.h`
- Added plan/style module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Plan.mm`
- Added extra-layers module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Layers.mm`
- Simplified orchestration entry:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.mm`
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `特效` (macOS hover pulse rendering path).
- Not part of: WASM overlay renderers, automation matcher/injector, input-indicator rendering path.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
  - `./tools/docs/doc-hygiene-check.sh --strict`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- Behavior contract unchanged; responsibility split only.
