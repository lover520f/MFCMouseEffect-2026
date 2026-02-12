# Dawn Ripple GPU Present (Stage 100)

## Summary
- Implemented the first real visual Dawn path for overlay present: `RipplePulse` commands are now encoded and drawn inside the Dawn surface render pass.
- Kept `CPU` fallback intact for unsupported command types (`trail` / `particle`) and for non-GPU-exclusive layers.

## Root Cause
- Previous Dawn present path only did a clear pass and always fell back to CPU rendering, so switching CPU/GPU mode showed little to no visible difference.

## What Changed
- Added dedicated ripple surface renderer module:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRippleSurfaceRenderer.h`
  - `MFCMouseEffect/MouseFx/Gpu/DawnRippleSurfaceRenderer.cpp`
- Extended Dawn runtime present context with required procs for:
  - shader module creation/release
  - render pipeline creation/release
  - vertex buffer creation/release
  - queue write buffer
  - render pass draw calls (`set pipeline`, `set vertex buffer`, `draw`)
- Updated interop:
  - `DawnSurfaceInterop` now executes ripple draw in the same render pass after clear.
  - returns `hadGpuVisualContent` signal to presenter.
- Updated presenter strategy:
  - GPU path returns success only when frame is `ripple-only` and GPU actually produced visual content.
  - otherwise keeps deterministic CPU fallback.
- Added layer-level GPU exclusive capability guard:
  - `IOverlayLayer::SupportsGpuExclusivePresent()` (default `false`)
  - `RippleOverlayLayer` overrides to `true`
  - host refuses GPU-exclusive skip when non-exclusive visible layers exist.

## Architectural Effect
- Moved from “GPU diagnostics/interop only” to “partial real rendering on GPU”.
- Maintains the intended policy: GPU first where implemented, CPU fallback where not implemented.
- Keeps blast radius bounded by per-layer capability gating.

## Verification
- Build passed with VS 2026 MSBuild (`Release|x64`).
- Existing warning retained (pre-existing/unrelated): `EffectConfig.cpp` unused local variable.

## Next Steps
- Implement `trail` GPU draw pass and mark trail layer as GPU-exclusive-capable after parity check.
- Implement `particle` GPU draw pass, then enable full frame GPU-exclusive path when all visible layers are supported.
