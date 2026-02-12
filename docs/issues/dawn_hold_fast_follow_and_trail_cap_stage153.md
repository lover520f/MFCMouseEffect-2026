# Dawn Hold Fast Follow And Trail Cap (Stage 153)

## Background
- After stage152, hold/trail latency-priority signal is aligned.
- Under very fast hold movement (especially with layered final-present fallback), smoothing can still feel behind cursor.
- Latency-priority trail history in layer still kept up to 24 points before GPU command preprocessing.

## Change
- In `HoldEffect` smooth follow:
  - when `latency_priority` is on, `hold_neon3d`, layered final-present fallback, and speed is high,
    switch to direct follow for that update (snap smoothed point to cursor).
- In `TrailOverlayLayer`:
  - tighten latency-priority point cap from `24` to `18`.

## Why
- Reduce high-speed hold cursor lag from smoothing accumulation.
- Lower trail geometry/input workload earlier in the pipeline.

## Validation
- Build `Release|x64`.
- Repro long-press fast movement and observe:
  - tighter hold cursor sync at high speed;
  - stable rendering without black-screen regression;
  - Dawn backend still active.
