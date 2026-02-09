# Dawn Backend + CPU Fallback (Stage 17 Host-Compat Bridge Enablement)

## Goal
- Enable Dawn overlay bridge availability in a controlled host-compatible mode.
- Allow runtime backend to switch to `dawn` when handshake succeeds, without replacing existing layered-window rendering path yet.

## Changes

### 1) Bridge Capability Switch
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnOverlayBridge.cpp`
- Behavior:
  - Under `MOUSEFX_ENABLE_DAWN_OVERLAY_BRIDGE`:
    - `compiled = true`
    - `available = true`
    - `detail = "bridge_enabled_host_compat"`
  - Without the macro:
    - keep `bridge_not_compiled` fallback.

### 2) Build Flag Enablement
- Updated:
  - `MFCMouseEffect/MFCMouseEffect.vcxproj`
- Added preprocessor define in all build configs:
  - `MOUSEFX_ENABLE_DAWN_OVERLAY_BRIDGE`

### 3) API Schema Sync
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Added bridge code:
  - `bridge_enabled_host_compat`

## Result
- When Dawn runtime reaches adapter/device success, Stage16 gating can now return `ok=true, backend="dawn"`.
- Existing effect rendering architecture remains stable because host window rendering path is still active.
- This is an incremental bridge step before full Dawn compositor replacement.
