# Stage 96 - Dawn GPU Presenter Surface Lifecycle Probe

## Goal
- Continue GPU landing without breaking current CPU fallback behavior.
- Replace pure placeholder presenter behavior with real Dawn surface lifecycle probing.

## Changes
- Added runtime-present context export:
  - `DawnRuntimePresentContext` in `MouseFx/Gpu/DawnRuntime.h`.
  - `GetDawnRuntimePresentContext()` in `MouseFx/Gpu/DawnRuntime.cpp`.
- Runtime now keeps a live `WGPUInstance` together with live device/queue when queue priming succeeds.
  - Properly released in `ReleaseLiveQueueContextLocked()`.
- `DawnGpuPresenter` now performs real compositor-surface steps:
  - create surface from HWND (`wgpuInstanceCreateSurface`)
  - configure surface (`wgpuSurfaceConfigure`)
  - acquire frame (`wgpuSurfaceGetCurrentTexture`)
  - present (`wgpuSurfacePresent`)
  - release/unconfigure surface in presenter destructor
- Presenter still returns fallback to CPU render path intentionally:
  - detail: `gpu_present_surface_ready_fallback_cpu_render_pending`
  - this avoids visual regression before Dawn render pass content is fully wired.

## Why
- This stage removes fake "not implemented" path and validates the real GPU presentation ABI chain.
- Keeps architecture stable while preserving user-visible correctness through existing CPU fallback.

## Validation
- Build: `Release|x64` compiles successfully.
- Runtime diagnostics should move from:
  - `gpu_present_not_implemented_fallback_cpu`
  to:
  - `gpu_present_surface_ready_fallback_cpu_render_pending`
  when surface lifecycle is healthy.

