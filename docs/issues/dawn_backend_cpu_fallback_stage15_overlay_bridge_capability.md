# Dawn Backend + CPU Fallback (Stage 15 Overlay Bridge Capability)

## Goal
- Introduce an explicit overlay bridge capability module so Dawn runtime handshake and actual GPU composition are decoupled.
- Expose bridge capability status in API payloads to make next-stage rollout observable.

## Changes

### 1) New Bridge Capability Module
- Added:
  - `MFCMouseEffect/MouseFx/Gpu/DawnOverlayBridge.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnOverlayBridge.cpp`
- API:
  - `GetDawnOverlayBridgeStatus()`
  - `IsDawnOverlayBridgeAvailable()`
- Current stage behavior:
  - returns `bridge_not_compiled` by default.
  - if `MOUSEFX_ENABLE_DAWN_OVERLAY_BRIDGE` is defined, returns `bridge_compiled_stub` (still unavailable).

### 2) Build Integration
- Updated:
  - `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`
- New files are now part of project build and VS filters.

### 3) Web API Visibility
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Added response field:
  - `dawn_overlay_bridge` with
    - `compiled`
    - `available`
    - `detail`
- Included in:
  - `/api/state`
  - `/api/gpu/probe_now`
- Extended schema:
  - `gpu_status_schema.bridge_codes`

### 4) UI Banner Hint
- Updated:
  - `MFCMouseEffect/WebUI/app.js`
- For state `device_ready_cpu_bridge_pending`, banner now appends bridge detail (en/zh text wrapper), making fallback reason explicit.

## Result
- Runtime handshake progress and overlay bridge readiness are now separate, explicit dimensions.
- This keeps architecture stable and prepares Stage 16 (real Dawn overlay composition path) with minimal churn.
