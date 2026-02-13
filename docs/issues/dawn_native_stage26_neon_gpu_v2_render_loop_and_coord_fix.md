# Dawn Native Stage 26 - Neon GPU-V2 Render Loop and Coordinate Fix

## Symptom
- Long press for `hold_neon3d_gpu_v2` had no visible effect.
- Diagnostics showed:
  - `runtime_reason=ok`
  - `visual_gpu_available=true`
  - `visual_submit_count=0`
  - `visual_miss_count` continuously increasing

## Root Cause

### 1) Direct runtime thread lacked message pumping
- The direct GPU runtime worker created and used a presenter window (`D3D11 + DirectComposition`) but did not pump messages.
- Without regular message dispatch in that thread, frame submission path can fail repeatedly while startup still appears successful.

### 2) Hold start coordinates could be truncated path-wise
- `WM_MFX_BUTTON_DOWN` coordinates are packed via `MAKELPARAM`, which is 16-bit per axis.
- In complex virtual/multi-display setups, relying on packed coords can produce incorrect hold start placement.

## Changes

### 1) Add message pump in direct runtime worker
- File: `MFCMouseEffect/MouseFx/Effects/HoldNeonGpuV2DirectRuntime.h`
- Added:
  - `PeekMessage/DispatchMessage` per loop
  - `MsgWaitForMultipleObjects` wait path with `QS_ALLINPUT`
- Result:
  - worker thread can process window/composition-related messages while rendering.

### 2) Harden hold/scroll dispatch coordinates with cursor fallback
- File: `MFCMouseEffect/MouseFx/Core/AppController.cpp`
- Updated:
  - `WM_MFX_BUTTON_DOWN`: prefer `GetCursorPos`, fallback to packed `lParam`
  - `WM_MFX_SCROLL`: same fallback logic
- Result:
  - avoid incorrect positioning due to packed coordinate limitations.

## Validation
- Build:
  - `MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`
  - Result: success, 0 errors.

