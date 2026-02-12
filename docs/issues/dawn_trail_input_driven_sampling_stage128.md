# Dawn Trail Input-Driven Sampling (Stage 128)

## Summary
Reduced redundant trail workload by changing fallback cursor sampling from per-frame polling to low-frequency recovery polling.

## Why
`TrailOverlayLayer` previously called `GetCursorPos` every update when no externally pushed move point was available.  
Under hold-heavy scenes this kept trail generation active at frame cadence and competed with hold_neon3d latency.

## What Changed
1. Added fallback sampling interval control in `TrailOverlayLayer`:
- new state: `lastCursorFallbackSampleMs_`
- fallback polling interval: `24 ms`

2. Sampling behavior now:
- Prefer externally pushed points (`AddPoint`) first.
- Only if no pushed point exists, perform fallback `GetCursorPos` sampling at the capped interval.

## Safety
- Trail still recovers if upstream move events are sparse.
- Normal trail rendering path is unchanged when move points are flowing.

## Validation
- `Release|x64` build passed with VS 2026 MSBuild (amd64 path).

## Next
- Continue reducing layered CPU final-present pressure while keeping fallback robustness.
