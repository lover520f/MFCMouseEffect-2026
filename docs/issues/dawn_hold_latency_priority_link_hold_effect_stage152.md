# Dawn Hold Latency Priority Link (Stage 152)

## Background
- Trail effect already supported `latency_priority`.
- Hold effect did not consume this signal, so hold follow still used legacy smoothing/command cadence.
- This created a split behavior: trail faster, hold still lag-like in some layered final-present scenarios.

## Change
- `AppController::SetTrailLatencyPriorityMode` now broadcasts `latency_priority` to both:
  - trail effect
  - hold effect
- `HoldEffect` now handles `latency_priority` command and applies:
  - stronger smooth follow alpha for neon3d hold in latency mode
  - tighter hold elapsed command interval (`2ms`) in latency mode

## Why
- Keep hold/trail latency strategy aligned under one control signal.
- Reduce perceived cursor desync without changing non-latency default behavior.

## Validation
- Build `Release|x64`.
- Repro hold_neon3d long-press fast movement:
  - hold and trail should feel more synchronized;
  - no black-screen regression;
  - Dawn path still active and CPU final-present fallback unchanged.
