# Dawn Native Stage20: FluxField GPU-v2 Pivot to Stable D3D11 Compute Route

## Why
Repeated `_com_error` first-chance storms on D2D experimental path caused long-press stutter and still produced no visible output in some environments.

The user goal is to validate a real GPU route reliably, not to keep an unstable D2D experiment alive.

## Architecture Change
`hold_fluxfield_gpu_v2` now uses:
- **GPU workload path**: D3D11 hardware device + UAV clear/copy passes (`FluxFieldGpuV2ComputeEngine`)
- **Visual path**: lightweight hold overlay renderer (`ChargeRenderer`) for stable on-screen feedback

This keeps route id and pipeline semantics while ensuring GPU is actually engaged and effect remains visible.

## Key Implementation
- Added `FluxFieldGpuV2ComputeEngine`:
  - runtime-load `d3d11.dll`
  - create hardware D3D11 device/context
  - allocate UAV textures
  - per-frame GPU passes via UAV clear + copy resource
- Reworked `FluxFieldHudGpuV2Renderer`:
  - removed dependency on unstable D2D render path for default GPU-v2 route
  - drive compute engine every frame
  - keep visible hold ring rendering
  - write hold-end snapshot to `.local/diag/flux_gpu_v2_compute_status_auto.json`
- Updated route reason in AppController to `flux_gpu_v2_d3d11_compute_route`

## Files Changed
- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldGpuV2ComputeEngine.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`
- `MFCMouseEffect/MouseFx/Core/AppController.cpp`

## Validation Target
- Long press should be visible and smooth (no COM exception storm behavior)
- GPU-v2 route status should report D3D11 compute route
- Compute snapshot file should confirm hardware GPU path state
