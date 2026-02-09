# Dawn Backend + CPU Fallback (Stage 3 Probe)

## Goal
- Improve backend diagnostics before full Dawn device wiring.
- Expose whether GPU/display hardware is present to settings and runtime state.

## Changes

### 1) GPU Hardware Probe Module
- Added:
  - `MFCMouseEffect/MouseFx/Gpu/GpuHardwareProbe.h`
- Provides:
  - `gpu::HasDesktopDisplayAdapter()`
- Implementation uses `EnumDisplayDevicesW` and checks:
  - `DISPLAY_DEVICE_ATTACHED_TO_DESKTOP`
  - not `DISPLAY_DEVICE_MIRRORING_DRIVER`

### 2) Dawn Runtime Probe Improvement
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
- If no desktop adapter is found:
  - returns detail `no_display_adapter`
- Keeps previous detail outputs:
  - `dawn_disabled_at_build`
  - `dawn_not_wired`

### 3) OverlayHostService Diagnostic API
- Added:
  - `OverlayHostService::HasGpuHardware()`
- File updates:
  - `MFCMouseEffect/MouseFx/Core/OverlayHostService.h`
  - `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`

### 4) Web State Exposure
- `/api/state` now includes:
  - `gpu_hardware_available`
- Updated file:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`

### 5) Project File Registration
- Added GPU headers to project metadata:
  - `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

## Result
- Backend fallback is still deterministic (CPU safe path).
- Runtime now reports more actionable reason/details for Dawn fallback.
- Architecture remains ready for Stage 4 (real Dawn adapter/device/surface initialization).
