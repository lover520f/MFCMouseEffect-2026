# Dawn Backend + CPU Fallback (Stage 4 Runtime Entry)

## Goal
- Move Dawn probing from header-inline placeholder to a real translation unit.
- Add dynamic runtime probing (DLL + symbol checks) without hard-linking Dawn SDK yet.
- Keep CPU fallback deterministic.

## Changes

### 1) DawnRuntime Refactor to .cpp
- Updated:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
- Added:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.cpp`
- `DawnRuntime.h` now only declares APIs:
  - `bool IsDawnCompiled();`
  - `DawnRuntimeInitResult TryInitializeDawnRuntime();`

### 2) Dynamic Loader Probe
- `DawnRuntime.cpp` attempts to load one of:
  - `webgpu_dawn.dll`
  - `dawn_native.dll`
  - `dawn.dll`
- If loaded, checks entry symbols:
  - `wgpuGetProcAddress` or `dawnProcSetProcs`

### 3) Diagnostic Detail Codes (Expanded)
- Existing + new detail values now include:
  - `no_display_adapter`
  - `dawn_disabled_at_build`
  - `dawn_loader_missing`
  - `dawn_symbols_missing`
  - `dawn_runtime_loaded_no_device`

### 4) Project Registration
- Added `DawnRuntime.cpp` to project:
  - `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

## Runtime Behavior
- Even when Dawn DLL/symbols are discoverable, Stage 4 still returns CPU backend
  until adapter/device/surface initialization is wired.
- Fallback remains explicit and traceable through `render_backend_detail`.

## Build Note
- `OverlayHostService.cpp` now explicitly includes:
  - `MouseFx/Gpu/GpuHardwareProbe.h`
- This avoids symbol visibility regression for `gpu::HasDesktopDisplayAdapter()`
  after `DawnRuntime` was split into `.h + .cpp`.

## Next Stage (Stage 5)
- Implement actual Dawn adapter/device/queue creation.
- Add overlay surface/swapchain hookup.
- Flip success path to `ok=true`, `backend=dawn` when full chain is ready.
