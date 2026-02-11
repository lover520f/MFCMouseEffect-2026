# Stage 82 - Dawn continuous ripple command throttling

## Background
- In Dawn backend mode, `hover_continuous` and `hold_continuous` can generate very high non-trail command traffic.
- Even with queue ready (`Q1:E1:M1:N1`), command storm increases CPU overhead in preprocess + submit path.

## Changes
- `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.h`
  - Added `ShouldEmitGpuContinuous(...)`.
  - Added per-instance last emit tick cache `lastGpuEmitTickById_` (mutable, command-path only).
- `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.cpp`
  - Added continuous command min-interval policy:
    - hover continuous: `14ms`
    - hold continuous: `10ms`
  - Applied throttle in `AppendGpuCommands(...)` for continuous ripple only.
  - Cleared emit cache entry when ripple instance is removed.
## Why this helps
- Reduces redundant non-trail packet pressure while keeping interaction continuity.
- Keeps CPU fallback architecture intact.

## Notes
- This is a traffic-control optimization on current partial-GPU stage.
- Full visual render migration to real Dawn draw pipeline is still required for complete GPU implementation.
