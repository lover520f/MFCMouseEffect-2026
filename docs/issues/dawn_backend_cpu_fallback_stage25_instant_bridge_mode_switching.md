# Dawn Backend + CPU Fallback (Stage 25 Instant Bridge Mode Switching)

## Goal
- Make GPU bridge mode selector apply immediately without requiring full settings Apply.
- Keep state/banner synchronized after runtime switch.

## Changes

### 1) Bridge Mode API Response Sync
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `POST /api/gpu/bridge_mode` response now includes:
  - `gpu_bridge_mode_request`

### 2) UI Instant Switch Flow
- Updated:
  - `MFCMouseEffect/WebUI/app.js`
- Added i18n status strings for bridge mode switching.
- Added `switchBridgeMode(mode)`:
  - calls `POST /api/gpu/bridge_mode`
  - merges latest runtime patch
  - refreshes GPU banner
- Added selector change handler:
  - `#gpu_bridge_mode_request` now applies immediately on change.

## Result
- Bridge mode switching is now interactive and immediate.
- Users can quickly iterate between `host_compat` and `compositor` requests during migration/testing.
