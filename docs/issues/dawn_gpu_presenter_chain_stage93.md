# Dawn GPU presenter chain wiring (Stage 93)

## Goal
Continue root-cause GPU landing by introducing a dedicated presenter chain:
- GPU presenter (Dawn path)
- CPU presenter (layered window fallback)

## Changes
- Added `MFCMouseEffect/MouseFx/Windows/OverlayPresentFrame.h`
  - Shared frame payload for presenter interfaces.

- Added `MFCMouseEffect/MouseFx/Interfaces/IOverlayPresenter.h`
  - Presenter abstraction for output stage.

- Added `MFCMouseEffect/MouseFx/Windows/DawnGpuPresenter.h`
- Added `MFCMouseEffect/MouseFx/Windows/DawnGpuPresenter.cpp`
  - Wired runtime/bridge readiness checks.
  - Current stage returns fallback detail because true GPU target-surface present is not implemented yet.

- Updated `MFCMouseEffect/MouseFx/Windows/OverlayLayeredCpuPresenter.*`
  - Converted to implement `IOverlayPresenter`.

- Updated `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.*`
  - `RenderSurface` now builds a shared frame payload.
  - If backend is `dawn_compositor`, tries `DawnGpuPresenter` first.
  - Automatically falls back to CPU presenter when GPU presenter is not ready/implemented.

- Updated project files:
  - `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

## Why this is root-direction
- Final output stage is now explicitly pluggable.
- Next stage can implement actual Dawn present target in `DawnGpuPresenter` without rewriting host scheduling and layer orchestration.

## Validation
1. Release build succeeded with VS2026 Professional MSBuild.
2. Runtime behavior remains stable due to deterministic CPU fallback in presenter chain.
