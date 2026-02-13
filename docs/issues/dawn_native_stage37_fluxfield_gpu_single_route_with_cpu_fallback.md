# Stage 37 - FluxField GPU Single-Route Rendering with CPU Fallback

## Problem
- FluxField GPU route could be perceived as CPU+GPU mixed rendering.
- Expected behavior: prioritize GPU route, fallback to CPU only when GPU route is unavailable/fails, and keep a single visible rendering path.

## Root cause
- Existing GPU route combined a GPU compute workload with a separate visual renderer path, which made route behavior ambiguous.
- Option labels did not clearly communicate route policy.

## Fix
1. Updated `hold_fluxfield_gpu_v2` renderer policy:
   - GPU visual backend first.
   - If GPU visual backend is unavailable/fails at runtime, switch to CPU renderer for the current hold session.
   - Never render GPU visual and CPU fallback simultaneously in one frame.
2. Kept route diagnostics in `flux_gpu_v2_compute_status_auto.json`:
   - `gpu_visual_active`
   - `cpu_fallback_active`
   - `route_reason`
3. Updated user-facing labels to make route semantics explicit:
   - CPU route: `FluxField HUD (CPU Only)`
   - GPU route: `FluxField HUD GPU (Auto Fallback CPU)`

## Files changed
- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`
- `MFCMouseEffect/Settings/SettingsOptions.h`
- `MFCMouseEffect/UI/Tray/TrayMenuBuilder.cpp`

## Validation
1. Select `FluxField HUD GPU (Auto Fallback CPU)` and long-press:
   - visible output should come from a single route at a time.
2. When GPU visual path is unavailable/fails:
   - renderer switches to CPU fallback for current session (no dual-layer render).
3. Check `flux_gpu_v2_compute_status_auto.json` for route state fields.
