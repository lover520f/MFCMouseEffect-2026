# Dawn Backend + CPU Fallback (Stage 14 Device Handshake)

## Goal
- Extend Dawn runtime probing from instance-only to adapter/device handshake.
- Keep current rendering path stable by preserving CPU fallback until OverlayHost GPU bridge exists.

## Changes

### 1) Dawn Runtime Probe/Handshake Expansion
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- Added probe flags:
  - `hasRequestDevice`
  - `canRequestAdapter`
  - `canCreateDevice`
- Added dynamic symbol resolution:
  - `wgpuInstanceRequestAdapter` / `webgpuInstanceRequestAdapter`
  - `wgpuAdapterRequestDevice` / `webgpuAdapterRequestDevice`
  - optional release symbols for adapter/device.
- Added async callback wait flow with timeout:
  - adapter request timeout/failure handling
  - device request timeout/failure handling

### 2) New Runtime Detail Codes
- Added detail codes:
  - `dawn_request_adapter_proc_missing`
  - `dawn_request_adapter_timeout`
  - `dawn_request_adapter_failed`
  - `dawn_request_device_proc_missing`
  - `dawn_request_device_timeout`
  - `dawn_request_device_failed`
  - `dawn_device_ready_cpu_bridge_pending`

### 3) Stage Result Contract
- Current stage outcome after successful adapter/device handshake:
  - still returns CPU backend intentionally:
    - `ok=false`
    - `backend=\"cpu\"`
    - `detail=\"dawn_device_ready_cpu_bridge_pending\"`
- Reason:
  - OverlayHost GPU rendering bridge is not wired yet.
  - This avoids false-positive GPU mode and protects current architecture stability.

### 4) Web Status/Schema Sync
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Synced:
  - `dawn_probe` payload includes new handshake flags.
  - new `state_code` mappings and action guidance.
  - `gpu_status_schema.state_codes` and `action_codes` include stage14 additions.

## Result
- Dawn runtime diagnostics are now significantly deeper and actionable.
- Project remains stable on CPU fallback while preparing for the next stage: OverlayHost Dawn rendering bridge.
