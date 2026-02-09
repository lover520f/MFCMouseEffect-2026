# Dawn Backend + CPU Fallback (Stage 7 Probe Refresh)

## Goal
- Make Dawn runtime probing refreshable at runtime (no process restart required).
- Add thread safety around probe cache.
- Expose probe generation to help verify refresh execution.

## Changes

### 1) DawnRuntime Thread-Safe Probe Cache
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- Added:
  - `uint64_t generation` in `DawnRuntimeProbeInfo`
  - `ResetDawnRuntimeProbe()`
  - `GetDawnRuntimeProbeInfo()` now returns a copy (not reference)
- Internal probe state is now guarded by `std::mutex`.

### 2) OverlayHostService Refresh API
- Added:
  - `OverlayHostService::RefreshGpuRuntimeProbe()`
- Behavior:
  - resets Dawn probe cache
  - if backend preference is `auto` or `dawn`, re-runs initialization probe and updates `render_backend_detail`

Files:
- `MFCMouseEffect/MouseFx/Core/OverlayHostService.h`
- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`

### 3) Web API: Runtime Probe Refresh
- Added endpoint:
  - `POST /api/gpu/probe_refresh`
- Returns:
  - `{"ok": true}`
- `BuildStateJson` now includes `dawn_probe.generation`.

File:
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`

## Result
- You can replace Dawn DLLs / update runtime environment and trigger refresh immediately.
- Probe output can be compared by generation id to confirm the refresh happened.
- CPU fallback behavior remains unchanged.
