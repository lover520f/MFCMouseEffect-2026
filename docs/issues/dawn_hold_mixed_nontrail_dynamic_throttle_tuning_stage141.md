# Dawn Hold Mixed Non-Trail Dynamic Throttle Tuning (Stage 141)

## Background
- Stage140 introduced heavy-trail dynamic throttling for hold mixed frames.
- Real diagnostics showed hold trail geometry now typically in a much smaller range (often `4~6` triangles), so previous heavy threshold (`24`) rarely triggered.

## Tuning
- Recalibrated heavy-trail gating to actual runtime scale:
  - heavy threshold: `24 -> 6` trail triangles
  - heavy interval: `20ms -> 16ms`
- Added explicit skip detail marker for observability:
  - `nontrail_submit_skipped_hold_priority_heavy_trail`

## Why
- Ensure dynamic branch actually engages under realistic hold movement.
- Keep trail continuity priority without over-throttling non-trail packets.

## Validation
- Build `Release|x64`.
- Under hold movement, `nontrail_submit_throttled` and skip detail should become observable when trail triangles stay in the 6+ range.

