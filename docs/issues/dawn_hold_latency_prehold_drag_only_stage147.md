# Dawn Hold Pre-Hold Latency Priority Drag-Only (Stage 147)

## Background
- Stage146 moved hold latency-priority activation into pre-hold candidate window.
- This reduced drag-like pressure, but changed normal long-press feel before hold promotion.

## Change
- Keep pre-hold latency-priority activation only when movement is drag-like.
- `AppController::ShouldPrioritizeHoldLatency` now accepts `dragLikeMove`:
  - hold running: unchanged behavior
  - pre-hold candidate: enable latency-priority only if `dragLikeMove == true`

## Why
- Preserve previous normal long-press hand feel.
- Retain pre-hold latency protection for heavy drag-like workload.

## Validation
- Build `Release|x64`.
- Verify two cases:
  - normal long-press move: feel should match earlier baseline
  - drag-like long-press move: hold follow latency improvement should remain.
