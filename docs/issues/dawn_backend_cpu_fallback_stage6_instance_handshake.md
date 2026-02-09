# Dawn Backend + CPU Fallback (Stage 6 Instance Handshake)

## Goal
- Move from symbol-only probe to minimal Dawn runtime handshake.
- Verify that `wgpuCreateInstance` can actually be called at runtime.
- Keep CPU fallback behavior unchanged.

## Changes

### 1) Probe Structure Extension
- Updated `DawnRuntimeProbeInfo`:
  - added `canCreateInstance`
- File:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`

### 2) Runtime Handshake in DawnRuntime
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- Added dynamic proc resolution for:
  - `wgpuCreateInstance` / `webgpuCreateInstance`
  - `wgpuInstanceRelease` / `webgpuInstanceRelease`
- `TryInitializeDawnRuntime()` now attempts:
  1. resolve create-instance symbol
  2. call create-instance with `nullptr` descriptor
  3. release instance when release symbol exists

### 3) New Diagnostic Details
- Possible detail states now include:
  - `dawn_create_instance_proc_missing`
  - `dawn_create_instance_failed`
  - `dawn_instance_ok_no_device`

## Web State Exposure
- `/api/state.dawn_probe` adds:
  - `can_create_instance`
- File:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`

## Result
- Runtime can now distinguish:
  - "DLL exists but not callable"
  - "instance call works"
  - "still CPU because adapter/device/surface stage is not wired"
- Fallback remains deterministic to CPU.

## Next Stage
- Stage 7: implement adapter/device creation and queue readiness checks.
