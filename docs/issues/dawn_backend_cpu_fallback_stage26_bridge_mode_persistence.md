# Dawn Backend + CPU Fallback (Stage 26 Bridge Mode Persistence)

## Goal
- Persist `gpu_bridge_mode_request` into `config.json`.
- Restore bridge mode request on startup/reload/reset.
- Keep runtime switch API and persisted settings consistent.

## Changes

### 1) Config Schema Persistence
- Updated:
  - `MFCMouseEffect/MouseFx/Core/EffectConfig.h`
  - `MFCMouseEffect/MouseFx/Core/EffectConfig.cpp`
- Added field:
  - `gpuBridgeModeRequest` (`host_compat|compositor`, default `host_compat`)
- `Load/Save` now read/write:
  - `gpu_bridge_mode_request`
- Invalid values are normalized to `host_compat`.

### 2) Overlay Host Runtime Preference API
- Updated:
  - `MFCMouseEffect/MouseFx/Core/OverlayHostService.h`
  - `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
- Added methods:
  - `SetGpuBridgeModeRequest(mode)`
  - `GetGpuBridgeModeRequest()`
- Runtime behavior:
  - pushes mode into Dawn bridge runtime (`SetRequestedBridgeMode`)
  - refreshes GPU runtime probe for immediate diagnostics sync.

### 3) Controller Integration (Startup + Apply + Command)
- Updated:
  - `MFCMouseEffect/MouseFx/Core/AppController.h`
  - `MFCMouseEffect/MouseFx/Core/AppController.cpp`
- Added command handling:
  - `{"cmd":"set_gpu_bridge_mode","mode":"..."}`
- Startup/reload/reset now re-apply persisted bridge mode request before backend preference.
- `apply_settings` now accepts `gpu_bridge_mode_request` and persists it.

### 4) Web API Alignment
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `POST /api/gpu/bridge_mode` now routes through controller command when available, so mode changes are persisted.
- `/api/state` now returns persisted `gpu_bridge_mode_request` from config snapshot.
- `POST /api/state` no longer strips `gpu_bridge_mode_request`; it forwards to controller for unified persistence logic.

## Result
- Bridge mode request is now durable across restart.
- Runtime and config no longer diverge when user switches bridge mode from the settings UI.
