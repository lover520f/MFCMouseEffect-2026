# Dawn Stage 79: Hold-First Scheduling Under Trail Pressure

## Goal
Reduce perceived lag for `hold_neon3d` when trail rendering is also active.

## Changes
1. `PreprocessTrailGeometry` now accepts policy flag:
- Signature: `PreprocessTrailGeometry(stream, bool skipTrailGeometryBuild = false)`
- When `skipTrailGeometryBuild=true`, non-trail preprocessing (ripple/particle) still runs, but heavy trail geometry baking is skipped for this frame.

2. Earlier parallel activation:
- Trail preprocess parallel threshold lowered from `2048` to `1024` vertices.

3. Hold-first scheduling in consumer:
- In `DawnCommandConsumer`, when hold is active and trail commands are present, trail baking is rate-limited (~20ms window) during hold burst.
- Non-trail packet submission is preserved in those frames so hold/ripple path remains responsive.

## Why This Helps
- Previously, heavy trail baking could delay hold/ripple path in the same frame.
- Now hold-heavy moments prioritize non-trail path continuity and reduce short stall spikes.

## Fallback Safety
- CPU fallback remains unchanged.
- Dawn submission failures still do not break fallback behavior.
