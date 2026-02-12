# Stage 99 - Feed OverlayGpuCommandStream into Dawn Surface Interop

## Goal
- Establish stable input contract from host frame to Dawn interop layer.
- Prepare for real GPU effect drawing (trail/ripple/particle) without reworking presenter interface again.

## Changes
- `OverlayPresentFrame` now carries GPU command stream pointer:
  - file: `MouseFx/Windows/OverlayPresentFrame.h`
  - field: `gpuCommandStream`
- `OverlayHostWindow` now forwards current frame command stream into present frame:
  - file: `MouseFx/Windows/OverlayHostWindow.cpp`
- Added command stream stats helper module:
  - `MouseFx/Gpu/DawnSurfaceCommandStats.h`
  - `MouseFx/Gpu/DawnSurfaceCommandStats.cpp`
- `DawnSurfaceInterop` interface now accepts frame command stream:
  - file: `MouseFx/Gpu/DawnSurfaceInterop.h`
  - function: `PresentDawnSurfaceClearPass(..., const OverlayGpuCommandStream* commandStream, ...)`
- Interop detail now reports command mix suffix:
  - example: `_cmd12_t3_r8_p1`
  - helps verify GPU present path receives expected effect stream shape.

## Why
- Presenter remains thin and orchestration-focused.
- Interop gets the exact per-frame command stream context required for next phase GPU drawing.
- This avoids repeated signature churn when replacing clear-pass with true effect pipelines.

## Validation
- `Release|x64` build validated after integration.

