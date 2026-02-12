# Dawn Hold State Signal for Consumer (Stage 149)

## Background
- Stage148 introduced hold trail geometry cap in Dawn consumer.
- Consumer used `rippleHoldCommandCount` as hold-state signal, but `hold_neon3d` path may not always emit ripple-hold commands.
- Result: cap was not consistently applied in real hold sessions.

## Change
- Add `kTrailLatencyPriority` GPU command flag.
- In `TrailOverlayLayer::AppendGpuCommands`, set this flag when trail latency-priority mode is active.
- In `DawnCommandConsumer`, treat hold as active when either condition is true:
  - ripple hold command exists
  - trail latency-priority flag is present

## Why
- Use the same runtime signal that actually drives hold latency-priority behavior.
- Make hold-related geometry cap deterministic for `hold_neon3d`, not dependent on ripple command shape.

## Validation
- Build `Release|x64`.
- Repro long-press fast-move scenario and inspect diagnostics:
  - hold trail geometry cap should apply consistently
  - peak `prepared_trail_triangles` should drop further versus stage148.
