# Dawn Particle GPU Surface Pass (Stage 105)

## Summary
- Added particle sprite visual encoding on Dawn surface pass.
- Dawn surface draw capability now covers trail + ripple + particle command types.

## Changes
- New files:
  - `MFCMouseEffect/MouseFx/Gpu/DawnParticleSurfaceRenderer.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnParticleSurfaceRenderer.cpp`
- Interop update:
  - `DawnSurfaceInterop` now encodes particle draw between trail and ripple draw.
  - Combined detail now includes particle draw counters (`particle_s...`).

## Notes
- Layered HWND safety guard remains active: CPU is still authoritative final present path.
- This stage completes GPU draw capability coverage while preserving stable display behavior.
