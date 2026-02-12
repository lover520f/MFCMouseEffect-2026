# Dawn presenter abstraction baseline (Stage 92)

## Goal
Move toward root-cause GPU landing by separating output presentation responsibilities from `OverlayHostWindow`.

This establishes a clean seam where GPU-present implementation can replace CPU-present implementation without rewriting host orchestration.

## Changes
- Added `MFCMouseEffect/MouseFx/Windows/OverlayLayeredCpuPresenter.h`
- Added `MFCMouseEffect/MouseFx/Windows/OverlayLayeredCpuPresenter.cpp`
  - Contains CPU layered-window present logic (`GDI+` draw + `UpdateLayeredWindow`)
  - Handles visibility culling and empty-surface skip behavior

- Updated `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.h`
  - Added `OverlayLayeredCpuPresenter` member

- Updated `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.cpp`
  - `RenderSurface` now delegates CPU presenting to `OverlayLayeredCpuPresenter`
  - Host window keeps orchestration/timing/collection duties only

- Updated project wiring
  - `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - `MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

## Why this is root-direction
- Previous bottleneck analysis shows current output is still CPU layered composition.
- Pure timing tweaks do not solve full GPU takeover.
- Presenter abstraction is required to introduce a true GPU presenter path cleanly.

## Validation
1. Release build succeeded with VS2026 Professional MSBuild.
2. Existing runtime behavior stays compatible while architecture is prepared for GPU presenter insertion.
