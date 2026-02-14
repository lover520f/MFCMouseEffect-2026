# Dawn Native Stage19: FluxField GPU-v2 Multi-Surface Offscreen BindDC Guard

## Symptom
GPU-v2 route stayed unstable on long-press (invisible output / stutter), especially in multi-monitor paths.

## Root Cause
Overlay host renders once per monitor surface each frame. GPU backend attempted `BindDC` and draw for every surface, including surfaces where cursor anchor is offscreen. Offscreen `BindDC`/clip failures could accumulate and trip backend failure fuse, destabilizing rendering.

## Fix
In D2D backend render:
- read current DC size (`HORZRES`, `VERTRES`)
- when cursor anchor is outside current surface bounds (with margin), skip this surface render as successful no-op
- clamp `BindDC` clip rect to current DC bounds
- if clipped rect becomes empty, treat as successful no-op

This prevents offscreen surfaces from falsely contributing failure counts.

## Files Changed
- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2D2DBackend.h`

## Validation
- Build `Release|x64` passes.
- Expected: multi-surface rendering no longer self-disables GPU path due to offscreen BindDC failures.
