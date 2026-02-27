# Phase 55zzzzbe - macOS Effect Duration Mapping (Size + Strength)

## What Changed
- Added shared duration scaling helper:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosOverlayRenderSupport.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosOverlayRenderSupport.mm`
- New contract:
  - `ScaleOverlayDurationBySize(baseDuration, overlaySize, baseReference, minDuration, maxDuration)`
- Applied to non-GPU one-shot effect plans:
  - click:
    - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Plan.mm`
    - `animationDuration` now scales by overlay size.
  - trail:
    - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Plan.mm`
    - `durationSec` and `closeAfterMs` now derive from scaled duration.
  - scroll:
    - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererSupport.h`
    - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererSupport.mm`
    - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Plan.mm`
    - `BuildPulseDuration` keeps existing strength mapping and then applies size scaling.
- Scroll helix decoration spin tempo now derives from resolved pulse duration (not fixed constant):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Layers.mm`

## Why
- Duration behavior previously mixed fixed and category-local mappings.
- After geometry scaling, keeping duration static can create inconsistent perceived tempo across sizes.
- Shared size-scaling with bounded clamps keeps perceived pace more stable while preserving existing strength semantics.

## Behavior Contract
- No API/schema changes.
- No platform capability changes.
- Scroll keeps strength-driven duration semantics; this change only adds bounded size normalization on top.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
  - PASS (scaffold lane, core lane smoke, core automation contract, macOS injection selfcheck, macOS wasm selfcheck, Linux compile gate, webui semantic tests)

## Four-Capability Mapping
- This change belongs to: `特效` (non-GPU duration mapping consistency).
