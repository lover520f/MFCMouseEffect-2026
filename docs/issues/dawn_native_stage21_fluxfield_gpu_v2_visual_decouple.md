# Dawn Native Stage21: FluxField GPU-v2 Visual Decouple from Charge Ring

## Why
`hold_fluxfield_gpu_v2` already ran a real GPU compute workload, but its on-screen output was still the temporary `ChargeRenderer` circle.  
This failed the expected FluxField visual identity and made CPU/GPU A/B verification less meaningful.

## Architecture Change
Keep the GPU route stable and explicit, but decouple visuals:
- **GPU workload path**: unchanged (`FluxFieldGpuV2ComputeEngine`, D3D11 hardware + UAV passes)
- **Visual path**: replace temporary ring with dedicated FluxField-like renderer (`FluxFieldHudGpuV2VisualRenderer`)

This preserves route semantics while restoring the intended visual effect category.

## Key Implementation
- Added `FluxFieldHudGpuV2VisualRenderer`:
  - segmented rotating ribbons (non-full-circle energy bands)
  - orbit particles with filament links
  - core shear rays and center glow
  - supports hold commands (`hold_ms`, `threshold_ms`, `hold_state`) for time-progress consistency
- Updated `FluxFieldHudGpuV2Renderer`:
  - switched `visualImpl_` from `ChargeRenderer` to `FluxFieldHudGpuV2VisualRenderer`
  - retained GPU compute tick + snapshot diagnostics flow

## Files Changed
- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2VisualRenderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`

## Validation Target
- `hold_fluxfield_gpu_v2` shows a FluxField-like complex effect (no longer only a circle ring)
- long press keeps GPU workload active and snapshot generation unchanged
- no fallback to CPU-only placeholder for this route
