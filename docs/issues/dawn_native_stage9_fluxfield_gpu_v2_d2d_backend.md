# Dawn Native Stage 9: FluxField GPU v2 Uses Real D2D GPU Backend

Date: 2026-02-13

## Goal
Turn `hold_fluxfield_gpu_v2` from a pure placeholder route into a real GPU-backed rendering path for immediate CPU/GPU A/B testing.

## What Changed
1. Route policy split by effect family:
   - `hold_neon3d_gpu_v2`: still gated by Dawn runtime probe.
   - `hold_fluxfield_gpu_v2`: now gated by Direct2D runtime probe (`d2d1.dll` + `D2D1CreateFactory`).
   - file: `MFCMouseEffect/MouseFx/Core/AppController.cpp`
2. `hold_fluxfield_gpu_v2` renderer now uses `ID2D1DCRenderTarget` drawing pass:
   - acquires HDC from current render surface
   - binds D2D DC render target
   - draws rings/spokes/particles via D2D APIs
   - on D2D failure, falls back to CPU renderer path
   - file: `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`

## Route Reason Semantics
- GPU-ready reason for flux route:
  - `d2d_gpu_renderer_ready`
- fallback reasons:
  - `d2d_runtime_dll_missing`
  - `d2d_create_factory_proc_missing`
  - `d2d_factory_create_failed`

## Why This Stage
- Current branch has no complete Dawn command-consumer source chain yet.
- This stage still provides a real GPU rendering route for the same effect family, so CPU/GPU contrast can be tested immediately while Dawn-native core reintegration continues.

