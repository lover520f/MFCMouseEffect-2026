# Dawn Native Stage 10: Fix FluxField GPU-v2 Position/Follow Transform

Date: 2026-02-13

## Symptom
- `hold_fluxfield_gpu_v2` rendered near top-left of primary screen.
- effect did not follow mouse updates.

## Root Cause
- `RippleOverlayLayer` applies per-instance `Graphics::TranslateTransform(left, top)` before renderer call.
- GPU renderer switched from GDI+ primitives to HDC-bound D2D drawing.
- HDC path does not automatically preserve GDI+ world transform.
- Result: GPU draw used surface-origin coordinates instead of per-instance local origin.

## Fix
- In `FluxFieldHudGpuV2Renderer::Render(...)`:
  1. read current GDI+ world transform before `GetHDC()`
  2. extract translation `(tx, ty)`
  3. apply `ID2D1DCRenderTarget::SetTransform(Translation(tx, ty))`

File:
- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`

## Validation
- With same hold interaction path, GPU-v2 effect should align with mouse position and follow movement.

