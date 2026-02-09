# Dawn Backend + CPU Fallback (Stage 24 Runtime Bridge Mode Switching)

## Goal
- Make Dawn bridge mode switchable at runtime from settings UI.
- Remove dependency on process restart / env-only workflows for bridge mode testing.

## Changes

### 1) Bridge Runtime Control API
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnOverlayBridge.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnOverlayBridge.cpp`
- Added APIs:
  - `SetRequestedBridgeMode(mode)`
  - `GetRequestedBridgeMode()`
- Behavior:
  - Runtime override is stored in-process (thread-safe), normalized to:
    - `host_compat`
    - `compositor`
  - If override is absent, env var path remains as fallback.

### 2) Web Server Endpoints + State
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Added endpoint:
  - `POST /api/gpu/bridge_mode` with `{ "mode": "host_compat|compositor" }`
- Added schema field:
  - `gpu_bridge_modes`
- Added state field:
  - `gpu_bridge_mode_request`
- `POST /api/state` now handles `gpu_bridge_mode_request` by applying bridge mode runtime override and refreshing GPU probe.

### 3) Settings UI Control
- Updated:
  - `MFCMouseEffect/WebUI/index.html`
  - `MFCMouseEffect/WebUI/app.js`
- Added General-section selector:
  - `GPU Bridge Mode`
- `reload()` binds from `state.gpu_bridge_mode_request`.
- `buildState()` includes `gpu_bridge_mode_request` so Apply triggers runtime switch.

## Result
- Bridge mode can now be switched from the UI without restart.
- Migration testing between host-compatible and compositor-request flows is faster and easier.
