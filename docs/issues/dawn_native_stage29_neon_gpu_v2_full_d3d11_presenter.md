# Stage 29 - Neon GPU-v2 Full D3D11 Presenter (No D2D Surface Bridge)

## Problem
- `hold_neon3d_gpu_v2` route was selected correctly, but long-press still had no visible effect.
- Runtime diagnostics repeatedly showed:
  - `runtime_reason=render_frame_false_d2d_create_bitmap_from_surface_failed_0x80070057`
- This means the previous visual path failed at `ID2D1DeviceContext::CreateBitmapFromDxgiSurface`, so render loop never reached present.

## Root Cause
- The previous presenter depended on a `D2D-on-DXGI-swapchain` bridge (`CreateBitmapFromDxgiSurface`).
- On some driver/runtime combinations, that bridge returned `E_INVALIDARG (0x80070057)` even after option/format fallbacks.
- Result: GPU route was active, but visual output stayed empty.

## Solution
- Replaced Neon GPU-v2 presenter visual path with a pure D3D11 shader pipeline:
  - `D3D11 device/context` for rendering
  - `DXGI swapchain (composition)` for buffers
  - `DirectComposition visual/target` for presenting transparent overlay
- Removed runtime dependency on D2D surface bitmap creation.
- New file:
  - `MFCMouseEffect/MouseFx/Renderers/Hold/NeonHudGpuV2ShaderPipeline.h`
- Updated file:
  - `MFCMouseEffect/MouseFx/Renderers/Hold/NeonHudGpuV2Presenter.h`

## Design Notes
- Presenter keeps ownership of window + device + swapchain + DComp.
- Shader pipeline class owns render-target and shader resources (single responsibility).
- One fullscreen-triangle draw call per frame; effect is generated procedurally in pixel shader.
- Output is premultiplied alpha to match composition swapchain alpha mode.

## Performance Notes
- No GDI drawing path in this presenter.
- No D2D surface interop call in frame loop.
- Rendering cost reduced to:
  - constant buffer update
  - clear + one draw call
  - present + dcomp commit

## Verification
1. Build `Release|x64`.
2. Run app and set hold effect to `hold_neon3d_gpu_v2`.
3. Long-press left mouse button.
4. Expected:
   - visible neon HUD effect around cursor
   - no heavy stutter from old failure loop
   - no `render_frame_false_d2d_create_bitmap_from_surface_failed_...` in `neon_gpu_v2_compute_status_auto.json`

## Risks / Follow-up
- Visual style now comes from shader implementation (not previous D2D stroke path), so appearance differs slightly.
- If specific GPUs still fail, add a small startup self-check and route-level fallback classification by failure stage (`shader compile`, `rtv create`, `present`, `dcomp commit`).
