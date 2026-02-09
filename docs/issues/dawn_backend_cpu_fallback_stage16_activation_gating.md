# Dawn Backend + CPU Fallback (Stage 16 Dawn Activation Gating)

## Goal
- Turn Dawn runtime success into a real backend switch condition when overlay bridge is available.
- Keep existing overlay host rendering path intact to avoid regressions before full Dawn compositor implementation lands.

## Changes

### 1) Dawn Runtime: Activation Gating
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- Logic:
  - After instance + adapter + device handshake succeeds, query `DawnOverlayBridgeStatus`.
  - If `bridge.available == true`:
    - return `ok=true`
    - return `backend="dawn"`
    - return `detail="dawn_overlay_bridge_ready"`
  - Otherwise keep CPU fallback detail:
    - `dawn_device_ready_cpu_bridge_pending`

### 2) Overlay Host Service Safety
- Updated:
  - `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
- Behavior change:
  - Removed Dawn-success early return in `Initialize()`.
  - Even when active backend is `dawn`, existing `OverlayHostWindow` creation path remains active.
- Reason:
  - Prevents rendering-path regressions while Dawn compositor bridge is still incremental.

### 3) State Mapping / API Schema
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Added state/action mappings:
  - state code: `overlay_bridge_ready`
  - action code: `enable_dawn_backend`
- Purpose:
  - Surface explicit "Dawn can be enabled" status in Web settings.

## Result
- Runtime now has a formal condition to become `backend=dawn`.
- Existing CPU overlay rendering remains stable until full GPU composition path is ready.
