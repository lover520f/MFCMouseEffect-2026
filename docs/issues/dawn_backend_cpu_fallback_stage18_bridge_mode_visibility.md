# Dawn Backend + CPU Fallback (Stage 18 Bridge Mode Visibility)

## Goal
- Expose Dawn overlay bridge mode explicitly (not just available/unavailable).
- Improve UI observability so users understand current GPU path is `host_compat` transitional mode.

## Changes

### 1) Bridge Status Model
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnOverlayBridge.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnOverlayBridge.cpp`
- Added field:
  - `mode` in `DawnOverlayBridgeStatus`
- Current values:
  - `none` when bridge is not compiled
  - `host_compat` when bridge feature is enabled in current stage

### 2) API Payload / Schema
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Added to `dawn_overlay_bridge` JSON:
  - `mode`
- Added to schema:
  - `gpu_status_schema.bridge_modes`
    - `none`
    - `host_compat`
    - `compositor` (reserved for future full GPU compositor stage)

### 3) UI Banner Readability
- Updated:
  - `MFCMouseEffect/WebUI/app.js`
- Banner now appends bridge mode text (en/zh wrapper):
  - `Bridge mode: host_compat` / `桥接模式: host_compat`

## Result
- Runtime status now communicates both bridge availability and bridge mode.
- Users can distinguish transitional GPU enablement from future full compositor mode.
