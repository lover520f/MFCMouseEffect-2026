# Dawn Native Stage 25 - Neon GPU-V2 Direct Runtime Visibility Fix

## Symptom
- `hold_neon3d_gpu_v2` long-press has no visible image on screen.
- Stutter improved, but rendering output is missing.

## Root Cause (high probability)
- Direct runtime worker thread did not explicitly initialize COM before creating DirectComposition pipeline.
- `WS_EX_NOREDIRECTIONBITMAP` reduced compatibility for this runtime path on some systems.

## Changes

### 1) COM initialization in direct runtime worker thread
- File: `MFCMouseEffect/MouseFx/Effects/HoldNeonGpuV2DirectRuntime.h`
- Added:
  - `CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)` before GPU presenter startup
  - `CoUninitialize()` on worker exit when initialized
- Added runtime reason writing to diagnostics:
  - `runtime_reason` in `neon_gpu_v2_compute_status_auto.json`
  - includes `coinit_failed_0x...` / `presenter_start_failed` / `presenter_not_ready` etc.

### 2) Presenter window style compatibility adjustment
- File: `MFCMouseEffect/MouseFx/Renderers/Hold/NeonHudGpuV2Presenter.h`
- Removed `WS_EX_NOREDIRECTIONBITMAP` from overlay window extended styles.

## Validation
- Build:
  - `MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`
  - Result: success, 0 errors.

## Runtime check
1. Select hold effect `hold_neon3d_gpu_v2`.
2. Long-press for ~1 second.
3. Release and check:
   - visible neon HUD appears during hold;
   - `neon_gpu_v2_compute_status_auto.json` contains:
     - `visual_gpu_available=true`
     - `runtime_reason` not starting with `coinit_failed_`.

