# Dawn Backend + CPU Fallback (Stage 2 Runtime Split)

## Goal
- Keep `OverlayHostService` focused on orchestration.
- Move Dawn initialization probing into a dedicated GPU runtime module.
- Expose fallback reason to UI/state for easier diagnosis.

## Main Changes

### 1) New GPU Runtime Entry (Dawn)
- Added:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRuntime.h`
- Provides:
  - `gpu::IsDawnCompiled()`
  - `gpu::TryInitializeDawnRuntime()`
- Current status:
  - placeholder implementation only
  - if `MOUSEFX_ENABLE_DAWN` is not defined -> `dawn_disabled_at_build`
  - if defined but not wired -> `dawn_not_wired`

### 2) OverlayHostService Refactor
- `OverlayHostService` now queries GPU runtime module instead of owning Dawn probing details.
- Added backend detail output:
  - `GetRenderBackendDetail()`
  - member `backendDetail_`
- Detail values currently include:
  - `cpu_default`
  - `cpu_forced`
  - `dawn_disabled_at_build`
  - `dawn_not_wired`

Files:
- `MFCMouseEffect/MouseFx/Core/OverlayHostService.h`
- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`

### 3) Web State Diagnostic Field
- Added `/api/state` field:
  - `render_backend_detail`
- This makes fallback reason visible to settings UI and debugging tools.

File:
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`

## Why This Matters
- Architecture remains clean while Dawn is integrated incrementally.
- CPU fallback remains deterministic and explicit.
- Next stage can implement actual Dawn device/surface setup inside `MouseFx/Gpu` without inflating service-layer complexity.

## Next Step
- Implement real Dawn runtime initialization in `gpu::TryInitializeDawnRuntime()`:
  - adapter/device/queue
  - overlay surface/swapchain binding
  - present loop integration
