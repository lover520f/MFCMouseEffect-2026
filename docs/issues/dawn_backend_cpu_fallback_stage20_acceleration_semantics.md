# Dawn Backend + CPU Fallback (Stage 20 Acceleration Semantics)

## Goal
- Distinguish `GPU active` from actual acceleration depth.
- Prevent users from assuming host-compatible bridge equals full GPU compositor acceleration.

## Changes

### 1) New Structured Acceleration Payload
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Added helper:
  - `BuildGpuAccelerationJson(activeBackend, bridgeStatus)`
- Added response field:
  - `gpu_acceleration` with:
    - `level`: `none | partial | full`
    - `label_en` / `label_zh`
- Included in:
  - `/api/state`
  - `/api/gpu/probe_now`

### 2) Banner Semantics Upgrade
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `gpu_status_banner` now embeds `acceleration` payload.
- Tone and text changes:
  - `full` -> tone `ok`, compositor wording
  - `partial` (host_compat) -> tone `info`, transitional wording

### 3) UI Rendering
- Updated:
  - `MFCMouseEffect/WebUI/app.js`
- Banner now appends acceleration text:
  - `Acceleration: ...` / `加速级别: ...`

## Result
- GPU status is now explicit about acceleration depth (`partial` vs `full`).
- Users get correct expectation about performance and rendering path maturity.
