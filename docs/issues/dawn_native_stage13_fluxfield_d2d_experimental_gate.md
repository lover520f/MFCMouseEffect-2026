# Dawn Native Stage 13: FluxField GPU-v2 D2D Experimental Gate

Date: 2026-02-13

## Goal
Reintroduce a real GPU draw path for `hold_fluxfield_gpu_v2` safely, without breaking the stable default route.

## What Changed
1. Added isolated D2D backend class:
   - `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2D2DBackend.h`
   - Uses `ID2D1DCRenderTarget` + `ID2D1SolidColorBrush` for GPU-accelerated ring rendering.
   - Handles failures with local disable-after-fail policy to avoid repeated failure storms.
2. `FluxFieldHudGpuV2Renderer` now supports dual runtime path:
   - default: stable CPU render fallback
   - optional: D2D experimental path when explicitly enabled
   - file: `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`
3. Route reason diagnostics now indicate experimental enablement:
   - `flux_gpu_v2_d2d_experimental_enabled`
   - file: `MFCMouseEffect/MouseFx/Core/AppController.cpp`

## Enable Switch (explicit opt-in)
- Environment variable:
  - `MFX_FLUX_GPU_V2_D2D=1` (or `true` / `on`)
- Or control file:
  - `<exe_dir>/.local/diag/flux_gpu_v2_d2d.on`

Without either switch, behavior remains the same stable CPU path.

## Why
- Keep production stability by default.
- Allow targeted GPU experiments on developer machines.
- Preserve rollback safety and deterministic diagnostics.
