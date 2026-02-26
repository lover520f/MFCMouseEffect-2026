# Phase 55zzzzap - macOS Trail Overlay Core Plan and Layers Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.mm` mixed:
  - render plan derivation (`type/size/frame/duration/close-delay`)
  - core layer path/style setup
  - glow layer setup
  - animation scheduling and orchestration
- This coupling increased change risk in trail effect evolution.

## What Changed
- Added internal contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Internal.h`
- Added plan module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Plan.mm`
- Added layers/animation module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Layers.mm`
- Simplified orchestration entry:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.mm`
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `特效` (macOS trail pulse rendering path).
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
