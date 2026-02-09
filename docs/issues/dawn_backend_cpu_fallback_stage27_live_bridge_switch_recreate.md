# Dawn Backend + CPU Fallback (Stage 27 Live Bridge Switch Recreate)

## Goal
- Ensure bridge mode switch takes effect immediately in runtime rendering path.
- Avoid "state changed but old pipeline still running" confusion.

## Changes

### 1) Live Recreate on Bridge Mode Update
- Updated:
  - `MFCMouseEffect/MouseFx/Core/AppController.cpp`
- Behavior of `SetGpuBridgeModeRequest(mode)`:
  - persists when value changed
  - always applies runtime bridge request to `OverlayHostService`
  - when backend preference is not `cpu`, triggers `RecreateActiveEffects()` so OverlayHost pipeline is re-initialized with latest bridge mode.

## Result
- Bridge mode switching from settings UI now updates actual effect pipeline immediately.
- User-observable behavior is aligned with diagnostics/banner state.
