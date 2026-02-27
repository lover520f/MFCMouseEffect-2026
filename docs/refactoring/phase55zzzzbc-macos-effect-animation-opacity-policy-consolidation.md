# Phase 55zzzzbc - macOS Effect Animation/Opacity Policy Consolidation

## What Changed
- Added shared animation/opacity helpers in:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosOverlayRenderSupport.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosOverlayRenderSupport.mm`
- New shared contracts:
  - `ClampOverlayOpacity(value)`
  - `CreateScaleFadeAnimationGroup(fromScale, toScale, fromOpacity, duration)`
- Consolidated click/trail/scroll pulse scale+fade animation creation onto shared helper:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Layers.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Layers.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Layers.mm`
- Normalized opacity clamp policy across non-GPU categories:
  - click/scroll/trail/hover/hold render layers now use shared clamp entry.
  - hover/hold breathing target opacity now uses shared clamp entry.
- Updated internal signatures to pass profile context for decoration opacity mapping:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Internal.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Internal.h`

## Why
- Animation curves and opacity handling were previously duplicated and drift-prone across categories.
- Shared helper entry avoids per-file micro-divergence while keeping each effect’s style-specific parameters intact.

## Behavior Contract
- No API/schema changes.
- No platform capability changes.
- Visual behavior remains category-specific, but curve/opacity policy is now applied through one reusable path.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
  - PASS (scaffold lane, core lane smoke, core automation contract, macOS injection selfcheck, macOS wasm selfcheck, Linux compile gate, webui semantic tests)

## Four-Capability Mapping
- This change belongs to: `特效` (animation/opacity policy consistency).
