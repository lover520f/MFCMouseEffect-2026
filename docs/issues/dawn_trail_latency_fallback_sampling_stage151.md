# Dawn Trail Latency Fallback Sampling Fix (Stage 151)

## Background
- In `latencyPriorityMode`, trail fallback cursor sampling was slower than normal mode.
- This could amplify perceived hold/trail lag when move events are sparse or coalesced.

## Change
- In `TrailOverlayLayer`, change latency fallback sampling interval:
  - from `32ms` to `8ms`.

## Why
- Latency-priority mode should prioritize responsiveness, not reduce sampling cadence.
- This is a root-cause timing fix in trail point acquisition, not a temporary patch.

## Validation
- Build `Release|x64`.
- Repro hold fast-move and compare:
  - cursor-follow feel should be tighter;
  - no new black-screen behavior;
  - diagnostics should still show Dawn command path active.
