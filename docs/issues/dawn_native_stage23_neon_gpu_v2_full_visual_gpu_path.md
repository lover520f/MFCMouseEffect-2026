# Dawn Native Stage23: Neon HUD3D GPU-v2 Full Visual GPU Path

## Why
Stage22 enabled real GPU compute workload for `hold_neon3d_gpu_v2`, but final on-screen visuals were still CPU/GDI+ rendering.  
That kept latency close to CPU mode under heavy Neon HUD3D visuals.

## Architecture Change
`hold_neon3d_gpu_v2` now uses dual GPU paths:
- **GPU compute path**: `NeonHudGpuV2ComputeEngine` (D3D11 UAV workload)
- **GPU visual path**: `HoldNeon3DGpuV2D2DBackend` (D2D DCRenderTarget drawing)

CPU Neon renderer is retained only as fallback when GPU visual backend is unavailable/fused.

## Key Implementation
- Added `HoldNeon3DGpuV2D2DBackend`:
  - BindDC with per-surface clip clamp
  - D2D-rendered Neon ring/spokes/progress/head layers
  - COM-exception fuse + failure threshold fallback guard
- Updated `HoldNeon3DGpuV2Renderer`:
  - prefer D2D GPU visual render each frame
  - fallback to `HoldNeon3DRenderer` only when needed
  - hold-end snapshot now records visual GPU status and fallback count:
    - `visual_gpu_rendered_last_frame`
    - `visual_gpu_available`
    - `visual_fallback_count`
- Updated route reason:
  - `neon_gpu_v2_d3d11_compute_plus_d2d_visual_route`

## Files Changed
- `MFCMouseEffect/MouseFx/Renderers/Hold/HoldNeon3DGpuV2D2DBackend.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/HoldNeon3DGpuV2Renderer.h`
- `MFCMouseEffect/MouseFx/Core/AppController.cpp`

## Validation Target
- `hold_neon3d_gpu_v2` long press should remain smooth with reduced CPU pressure
- D3D11/NVIDIA modules load during hold
- `neon_gpu_v2_compute_status_auto.json` shows visual GPU flags and low/zero fallback count
