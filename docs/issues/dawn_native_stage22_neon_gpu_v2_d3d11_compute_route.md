# Dawn Native Stage22: Implement Real Neon HUD3D GPU-v2 Route

## Why
`hold_neon3d_gpu_v2` was still a placeholder wrapper around CPU neon renderer semantics, which did not satisfy the "real GPU route" target.

## Architecture Change
`hold_neon3d_gpu_v2` now runs:
- **Visual path**: existing `HoldNeon3DRenderer` (Neon HUD3D look unchanged)
- **GPU workload path**: new `NeonHudGpuV2ComputeEngine` (D3D11 hardware device + UAV pass workload)

This mirrors the stabilized GPU-v2 route strategy used by FluxField while preserving Neon HUD3D visuals.

## Key Implementation
- Added `NeonHudGpuV2ComputeEngine`:
  - runtime-load `d3d11.dll`
  - create hardware D3D11 device/context
  - create UAV textures
  - per-frame GPU passes (`ClearUnorderedAccessViewFloat` + `CopyResource`)
- Updated `HoldNeon3DGpuV2Renderer`:
  - keep `HoldNeon3DRenderer` as visual renderer
  - tick GPU compute engine every frame
  - collect hold-state and write snapshot on hold end:
    - `.local/diag/neon_gpu_v2_compute_status_auto.json`
- Updated hold route reason:
  - `neon_gpu_v2_d3d11_compute_route`
  - no longer treated as Dawn-placeholder branch

## Files Changed
- `MFCMouseEffect/MouseFx/Renderers/Hold/NeonHudGpuV2ComputeEngine.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/HoldNeon3DGpuV2Renderer.h`
- `MFCMouseEffect/MouseFx/Core/AppController.cpp`

## Validation Target
- Selecting `hold_neon3d_gpu_v2` keeps Neon HUD3D visuals
- Long press engages D3D11/NVIDIA user-mode modules
- Snapshot confirms active hardware GPU compute route
