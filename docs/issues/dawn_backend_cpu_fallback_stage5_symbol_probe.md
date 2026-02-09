# Dawn Backend + CPU Fallback (Stage 5 Symbol Probe)

## Goal
- Upgrade Dawn runtime probing from binary-present check to symbol-level readiness checks.
- Keep zero hard dependency on Dawn headers/libs at this stage.

## Changes

### 1) DawnRuntime Probe Model
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- Added `DawnRuntimeProbeInfo`:
  - `compiled`
  - `hasDisplayAdapter`
  - `moduleLoaded`
  - `hasCoreProc`
  - `hasCreateInstance`
  - `hasRequestAdapter`
  - `moduleName`
  - `detail`
- New API:
  - `const DawnRuntimeProbeInfo& GetDawnRuntimeProbeInfo()`

### 2) Dynamic Module + Symbol Checks
- Runtime now probes:
  - module candidates: `webgpu_dawn.dll`, `dawn_native.dll`, `dawn.dll`
  - core proc symbols: `wgpuGetProcAddress` or `dawnProcSetProcs`
  - instance symbols: `wgpuCreateInstance` / `webgpuCreateInstance`
  - adapter request symbols: `wgpuInstanceRequestAdapter` / `webgpuInstanceRequestAdapter`

### 3) Detail Codes Expanded
- Added detail states:
  - `dawn_symbols_partial`
  - `dawn_runtime_ready_for_device_stage` (probe-level)
  - `dawn_runtime_ready_no_device` (init path still not creating device)

### 4) Web State Visibility
- `/api/state` now includes:
  - `dawn_probe` object with all probe flags and detail
- File:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`

## Result
- You can now see exactly why Dawn cannot be activated in a given environment.
- CPU fallback remains unchanged and deterministic.

## Next Step
- Stage 6: replace probe-only readiness with real `Instance -> Adapter -> Device` creation,
  then switch `TryInitializeDawnRuntime()` success path to `ok=true`, `backend=dawn`.
