# Dawn Backend + CPU Fallback (Stage 23 Bridge Mode Request + Auto Fallback)

## Goal
- Allow explicit bridge mode requests (`host_compat` / `compositor`) without code changes.
- Keep runtime stable by auto-falling back when compositor prerequisites are not ready.

## Changes

### 1) Bridge Request Mode Input
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnOverlayBridge.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnOverlayBridge.cpp`
- Added field:
  - `requestedMode`
- Runtime input:
  - environment variable `MFX_DAWN_BRIDGE_MODE`
  - values:
    - `host_compat` (default)
    - `compositor`

### 2) Auto Fallback Logic
- When `MFX_DAWN_BRIDGE_MODE=compositor`:
  - if compositor APIs are ready: `mode=compositor`, `detail=bridge_enabled_compositor`
  - else: fallback to `mode=host_compat`, `detail=bridge_fallback_host_compat_compositor_not_ready`
- This preserves stability and avoids hard failure paths.

### 3) API / Schema Visibility
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `dawn_overlay_bridge` now includes:
  - `requested_mode`
  - `requested_mode_label_en` / `requested_mode_label_zh`
- Added schema bridge codes:
  - `bridge_enabled_compositor`
  - `bridge_fallback_host_compat_compositor_not_ready`

### 4) UI Banner Explanation
- Updated:
  - `MFCMouseEffect/WebUI/app.js`
- Banner now shows downgrade text when requested mode differs from active mode.

## Result
- Bridge mode can be switched externally (env var) for staged rollout.
- Unsupported compositor requests downgrade safely with explicit user-visible reason.
