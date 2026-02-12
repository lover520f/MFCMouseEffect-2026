# Dawn Hold Trail History Window Tighten (Stage 139)

## Problem
- In hold latency-priority mode, trail visibility had been restored, but command preprocessing could still carry too much history.
- Excess trail history increases CPU-side preprocessing work and may widen cursor-to-effect delay under fast hold movement.

## Change
- Tightened trail history window only when `latencyPriorityMode` is active:
  - effective trail lifetime cap: `180ms`
  - effective max points cap: `24`
- Normal (non-latency-priority) trail behavior remains unchanged.

## Scope
- `TrailOverlayLayer::Update` now trims by an effective duration in priority mode.
- `TrailOverlayLayer::SampleCursorPoint` now enforces an effective max-point cap in priority mode.

## Expected Impact
- Keep trail visible during hold while reducing preprocessing load.
- Improve hold-path responsiveness and reduce "heavy tail" lag feeling.
- Preserve regular trail quality outside hold latency-priority mode.

## Validation
- Build `Release|x64` and verify no regressions.
- Observe `dawn_command_consumer.prepared_trail_vertices/triangles` under hold should trend lower than before.

