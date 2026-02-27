# Phase 55zzzzbb - macOS Effect Overlay Contents Scale Consistency

## What Changed
- Added shared overlay contents-scale helpers:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosOverlayRenderSupport.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosOverlayRenderSupport.mm`
- New shared contracts:
  - `ResolveOverlayContentsScale(overlayPt)`
  - `ApplyOverlayContentScale(content, overlayPt)`
- Wired all non-GPU effect overlay creation paths to apply per-screen contents scale:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Start.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Update.mm`
- Removed `mainScreen` hard-coupling in click text rendering:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Layers.mm`
  - text layer now uses current overlay root layer scale.

## Why
- On multi-screen mixed-DPI setups, `mainScreen` may not match the actual overlay target display.
- This caused potential text sharpness/style inconsistency for click text and drift risk across effect categories.
- Shared per-screen scale application keeps non-GPU effect rendering behavior consistent with actual target display.

## Behavior Contract
- No API/schema changes.
- No effect category availability changes.
- Rendering policy update only: overlay content/layers use target-screen scale, not global `mainScreen`.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
  - PASS (scaffold lane, core lane smoke, core automation contract, macOS injection selfcheck, macOS wasm selfcheck, Linux compile gate, webui semantic tests)

## Four-Capability Mapping
- This change belongs to: `特效` (non-GPU overlay rendering consistency on high DPI / multi-screen).
