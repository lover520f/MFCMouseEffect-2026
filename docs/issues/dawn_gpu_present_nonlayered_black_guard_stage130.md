# Dawn GPU Present Non-Layered Black Guard (Stage 130)

## Background
- In non-layered final-present mode, overlay host windows can stay visible even when no layer is visible on that surface.
- If the GPU present path cannot produce a valid frame immediately, users may observe transient or sustained black windows.

## Root Cause
- Non-layered windows do not use `UpdateLayeredWindow` alpha composition fallback semantics.
- Existing rollback only triggered after repeated present failures; startup-first failure windows could still expose a black surface briefly.
- Window visibility was not gated by per-surface effective layer visibility in non-layered GPU present mode.

## Changes
1. Added per-surface visibility gating in `OverlayHostWindow::RenderSurface` for non-layered GPU present path.
   - If no alive layer intersects the surface: hide the surface window and skip present.
   - If visible content returns: show the surface window before present.
2. Added startup fail-fast rollback for GPU final present.
   - If GPU present has never succeeded and first present attempt fails, immediately schedule layered CPU rollback cooldown.
   - Kept existing repeated-failure rollback path for post-startup regressions.
3. Added dedicated diagnostic detail for hidden surface path:
   - `gpu_present_surface_hidden_no_visible_layers`

## Verification
- Build: `x64 Release` via VS 2026 Professional MSBuild succeeded.
- Runtime expectation:
  - non-layered path no longer keeps black host windows visible when no surface-visible content exists;
  - startup-present first failure no longer waits for multiple failures before rollback.

## Impact
- Reduces black-screen risk while preserving GPU-first attempt semantics.
- Keeps CPU layered fallback as deterministic safety path.
