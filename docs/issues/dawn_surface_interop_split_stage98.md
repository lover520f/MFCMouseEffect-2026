# Stage 98 - Split Dawn Surface Interop from Presenter

## Goal
- Reduce `DawnGpuPresenter` complexity and coupling.
- Keep presenter focused on orchestration while moving low-level WebGPU ABI details to a dedicated module.

## Changes
- Added new module:
  - `MouseFx/Gpu/DawnSurfaceInterop.h`
  - `MouseFx/Gpu/DawnSurfaceInterop.cpp`
- New dedicated responsibilities:
  - surface lifecycle management (`create/configure/release`)
  - per-frame clear-pass submission (`acquire/view/renderpass/submit/present`)
  - raw WebGPU ABI structs and proc signatures are isolated in this module.
- `DawnGpuPresenter` now:
  - validates runtime/bridge state
  - delegates frame present attempt to `PresentDawnSurfaceClearPass(...)`
  - preserves current CPU fallback strategy.
- Project files updated:
  - `MFCMouseEffect.vcxproj`
  - `MFCMouseEffect.vcxproj.filters`

## Why This Matters
- Improves single responsibility and encapsulation.
- Makes next GPU stages easier:
  - replacing clear-pass with actual effect draw pass can now evolve inside one focused module.
  - presenter stays stable and small.

## Validation
- `Release|x64` build succeeds after split.

