# Phase 56b - Click Effect Compute/Render Command Split (macOS First)

## Goal
Make click effect path explicit as `shared compute -> platform render`, so settings changes are resolved once in core compute logic and then consumed by platform renderer.

## Design
- New shared compute module:
  - `MouseFx/Core/Effects/ClickEffectCompute.h`
  - `MouseFx/Core/Effects/ClickEffectCompute.cpp`
- Core output contract:
  - `ClickEffectRenderCommand`
  - carries normalized type, size/duration, opacity, palette, label, point.
- Platform role:
  - macOS renderer now executes `ClickEffectRenderCommand`.
  - Existing API (`ShowClickPulseOverlay(point, button, type, theme, profile)`) is preserved as compatibility wrapper that computes command then delegates.

## Behavior impact
- Settings/profile changes still flow to runtime as before.
- Rendering now depends on computed command values instead of re-deriving type/palette in renderer internals.
- No protocol/schema change.

## Files changed
- Added:
  - `MFCMouseEffect/MouseFx/Core/Effects/ClickEffectCompute.h`
  - `MFCMouseEffect/MouseFx/Core/Effects/ClickEffectCompute.cpp`
- Updated:
  - `MFCMouseEffect/Platform/macos/CMakeLists.txt`
  - `MFCMouseEffect/Platform/macos/Effects/MacosClickPulseEffect.mm`
  - `MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayRenderer.h`
  - `MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayRenderer.mm`
  - `MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.h`
  - `MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.mm`
  - `MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Internal.h`
  - `MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Plan.mm`
  - `MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Layers.mm`

## Validation
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
  - Build passed.
  - In constrained runtime, HTTP bind `EACCES` path is skipped by policy; regression gate reports pass.

## Next step
- Reuse the same pattern for trail/scroll/hold/hover:
  - shared compute profile + render command
  - per-platform renderer only executes command.
