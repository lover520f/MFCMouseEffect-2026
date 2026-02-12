# Dawn Trail GPU Surface Pass (Stage 103)

## Summary
- Added real Dawn trail draw encoding on surface render pass.
- Surface pass now attempts `trail` and `ripple` visual draw in the same frame.

## Changes
- New module:
  - `MFCMouseEffect/MouseFx/Gpu/DawnTrailSurfaceRenderer.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnTrailSurfaceRenderer.cpp`
- `DawnSurfaceInterop` now:
  - executes trail draw before ripple draw in render pass
  - marks GPU visual content when either trail or ripple was drawn
  - reports combined detail suffix for trail/ripple stats
- `DawnSurfaceInteropState` extended with dedicated trail renderer state.

## Notes
- Current layered HWND safety guard still enforces CPU present fallback to avoid black-screen regression.
- This stage focuses on GPU rendering capability landing, preparing for non-layered/compositor-safe present path.
