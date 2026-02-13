# Dawn Native Stage 15: FluxField D2D COM-Error Fuse + Settings-Only Gate

Date: 2026-02-13

## Goal
Stop `_com_error` storm and remove stale legacy gate interference so FluxField GPU-v2 behavior is deterministic.

## What Changed
1. Added hard fuse in D2D backend:
   - file: `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2D2DBackend.h`
   - wraps render path with `_com_error` / catch-all guard
   - on COM exception: immediately disables D2D path and globally blocks further attempts for this process
   - auto-falls back to stable CPU renderer path
2. Removed legacy env/file gate from runtime selection:
   - file: `MFCMouseEffect/MouseFx/Core/AppController.cpp`
   - `hold_fluxfield_gpu_v2` D2D enable decision now comes from settings only:
     - `flux_gpu_v2_d2d_experimental`
3. Removed legacy bootstrap detection in GPU-v2 renderer start:
   - file: `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`
   - renderer starts with D2D disabled, then receives explicit runtime command from `HoldEffect`.

## Why
- Prevent repeated COM first-chance exception loops from killing frame pacing.
- Avoid “settings says off but old marker file still forces on” confusion.
- Keep behavior explicit and controllable from web settings only.

## Validation Focus
1. Toggle `FluxField GPU-v2 D2D (Experimental)` in web settings.
2. Use `hold_fluxfield_gpu_v2` and hold mouse.
3. Verify:
   - no `_com_error` storm
   - no severe stall/no-effect regression
   - if D2D fails once, CPU fallback remains stable for current process.
