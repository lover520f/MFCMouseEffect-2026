# Dawn Native Stage17: FluxField D2D BindDC Clip-Rect / Transform Mismatch Fix

## Symptom
- `hold_fluxfield_gpu_v2` selected and D2D experimental enabled
- GPU route loaded, but long-press had no visible effect

## Root Cause
D2D backend used world translation (`m[4]`, `m[5]`) for drawing, but `ID2D1DCRenderTarget::BindDC` clip rect was always `{0,0,size,size}`.

When translated drawing coordinates moved outside this fixed clip rect, content was clipped out and became invisible.

## Fix
Align `BindDC` clip rect with world translation:
- `rc = { tx, ty, tx + sizePx, ty + sizePx }`
- `tx/ty` derived from GDI+ world transform translation (`m[4]/m[5]`)

## Files Changed
- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2D2DBackend.h`

## Validation
- Build `Release|x64` succeeds.
- Expected: GPU-v2 D2D path should render at cursor-aligned location instead of being fully clipped.
