# Dawn Backend + CPU Fallback (Stage 29 Bridge Fallback Guided Action)

## Goal
- Make bridge fallback diagnostics directly actionable.
- Avoid misleading "Enable Dawn" suggestion when GPU backend is already active.

## Changes

### 1) Bridge Fallback Specific Advice
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- In `BuildGpuBannerJson(...)`, when:
  - active backend is `dawn`
  - bridge detail is `bridge_fallback_host_compat_compositor_not_ready`
- Banner action is overridden to:
  - `action_code: switch_bridge_host_compat`
  - advice text suggests switching to stable host-compatible bridge mode.

### 2) Remove Redundant Enable-Dawn Advice in GPU-Active State
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- If GPU is already active and generic advice would be `enable_dawn_backend`, it is replaced with `trigger_probe_now` guidance.

### 3) WebUI One-Click Stable Bridge Switch
- Updated:
  - `MFCMouseEffect/WebUI/app.js`
- Added support for `switch_bridge_host_compat` action code:
  - triggers `POST /api/gpu/bridge_mode` with `host_compat`
  - updates local state/banner/select immediately.
- Added i18n button label:
  - EN: `Use Stable Bridge`
  - ZH: `切换稳定桥接`

### 4) Status Schema Sync
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Added `switch_bridge_host_compat` into `gpu_status_schema.action_codes`.

## Result
- When compositor path is unavailable, users get a direct, correct remediation button.
- GPU-active banner actions are now semantically accurate and less confusing.
