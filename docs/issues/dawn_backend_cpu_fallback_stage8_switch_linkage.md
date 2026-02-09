# Dawn Backend + CPU Fallback (Stage 8 Switch Linkage)

## Goal
- Link backend switching with Dawn probe refresh automatically.
- Add one-shot runtime probe API for direct diagnosis.

## Changes

### 1) Backend Switch Auto-Refresh
- Updated:
  - `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
- `SetRenderBackendPreference(...)` now calls `RefreshGpuRuntimeProbe()` before shutdown.
- This avoids stale probe cache after changing `render_backend`.

### 2) Direct Probe API in Service
- Added:
  - `OverlayHostService::ProbeDawnRuntimeNow(bool refreshProbe)`
- Behavior:
  - optional probe reset
  - immediate `TryInitializeDawnRuntime()`
  - sync updates `render_backend_detail`

Files:
- `MFCMouseEffect/MouseFx/Core/OverlayHostService.h`
- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`

### 3) Web Endpoint for One-Shot Probe
- Added endpoint:
  - `POST /api/gpu/probe_now`
- Request body (optional):
  - `{"refresh": true|false}`
- Response:
  - `{"ok": true, "detail": "..."}`

File:
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`

## Result
- Backend preference changes now naturally re-evaluate Dawn readiness.
- You can trigger probe on demand from web UI tooling without app restart.
