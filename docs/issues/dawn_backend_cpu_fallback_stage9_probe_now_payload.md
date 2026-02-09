# Dawn Backend + CPU Fallback (Stage 9 probe_now Payload)

## Goal
- Make `/api/gpu/probe_now` return full diagnostics, not only a single detail string.
- Keep probe payload format consistent with `/api/state.dawn_probe`.

## Changes

### 1) Shared Dawn Probe JSON Builder
- Added helper in `WebSettingsServer.cpp`:
  - `BuildDawnProbeJson(const gpu::DawnRuntimeProbeInfo&)`
- This avoids duplicated mapping logic between endpoints.

### 2) Expanded `/api/gpu/probe_now` Response
- Endpoint now returns:
  - `detail`
  - `active_backend`
  - `backend_detail`
  - `gpu_hardware_available`
  - `dawn_available`
  - `dawn_probe` (full structured probe object)

### 3) `/api/state` Uses Same Probe Serializer
- `BuildStateJson()` now reuses `BuildDawnProbeJson(...)`.
- Prevents schema drift between `state` and `probe_now`.

## Result
- Frontend can directly render a complete GPU diagnostics panel from `probe_now`.
- Probe/debug payloads stay consistent across API entry points.
