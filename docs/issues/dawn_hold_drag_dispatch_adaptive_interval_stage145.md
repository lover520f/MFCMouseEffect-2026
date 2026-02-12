# Dawn Hold Drag Dispatch Adaptive Interval (Stage 145)

## Background
- During hold + system window drag, move dispatch is throttled to reduce workload.
- A fixed 20ms throttle helps stability, but under `hold_neon3d` latency-priority mode it can still add visible follow lag.

## Change
- Make drag-like move dispatch interval adaptive in `AppController`:
  - default path keeps `20ms`
  - hold latency-priority path uses `8ms`
- Apply the interval decision after hold latency-priority state is resolved in `WM_MFX_MOVE`.

## Why
- Keep normal drag throttling behavior unchanged.
- Reduce hold follow delay specifically in the latency-priority path without introducing a separate API or extra runtime mode.

## Validation
- Build `Release|x64`.
- Repro: open settings page, hold `hold_neon3d`, move quickly for 8-10 seconds.
- Check diagnostics:
  - `hold_with_trail` in tail window should stay high
  - hold/trail cursor follow should improve under drag-like movement.
