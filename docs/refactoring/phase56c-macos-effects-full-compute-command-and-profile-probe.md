# Phase 56c - macOS Effects Full Compute Command Path + Probe Contract

## Goal
Complete macOS effect-path convergence to:

`EffectConfig -> Shared Compute -> RenderCommand -> macOS Renderer`

for all five categories (`click/trail/scroll/hover/hold`), while keeping Windows behavior stable.

## Scope
- Shared compute modules added/used for:
  - `ClickEffectCompute`
  - `TrailEffectCompute`
  - `ScrollEffectCompute`
  - `HoverEffectCompute`
  - `HoldEffectCompute`
- macOS render wrappers/core now consume commands as primary execution input.
- Windows `EffectFactory` keeps existing render backends and aligns type normalization through shared compute normalizers.
- Test profile route now exposes command observability:
  - `/api/effects/test-render-profiles` adds `command_samples`
  - includes `trail_emission` sample for throttle-contract visibility.

## Design Notes
- Renderer no longer performs second-pass type normalization or key parameter derivation.
- Command derivation is single-source in compute layer.
- Existing renderer compatibility overloads are retained to avoid API breakage and reduce migration risk.

## Contract Changes
- `GET /api/effects/test-render-profiles` response now includes:
  - `command_samples.sample_input`
  - `command_samples.click`
  - `command_samples.trail`
  - `command_samples.trail_emission`
  - `command_samples.scroll`
  - `command_samples.hover`
  - `command_samples.hold.start`
  - `command_samples.hold.update`
- Core effects regression now asserts presence of these fields and key command parameters (`size_px`, `duration_sec`, `stroke_argb`).

## Validation
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
  - Build passed.
  - In constrained runtime, HTTP bind `EACCES` path is skipped by existing policy and reported as explicit skip/pass.

## Risk
- Low-medium: command sample schema expands test route payload.
- Mitigation: route remains test-gated (`MFX_ENABLE_EFFECT_OVERLAY_TEST_API`), production route compatibility unchanged.
