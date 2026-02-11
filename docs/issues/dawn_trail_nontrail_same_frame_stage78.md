# Dawn Stage 78: Submit Trail + Non-Trail In Same Frame

## Problem
When trail geometry existed in a frame, the consumer only submitted trail packet path and skipped ripple/hold/hover packet submission in the same frame. This made non-trail effects lag behind under heavy trail activity.

## Change
- Refactored Dawn command submit branch in `DawnCommandConsumer`.
- Added `submitNonTrailPacket` lambda for ripple/particle/mixed packet routing.
- New behavior:
  - If frame has trail + non-trail geometry: submit trail packet and then non-trail packet in the same frame.
  - If frame has only non-trail geometry: keep non-trail submit path as before.
  - Fallback remains empty-command submit when needed.

## Diagnostics
- Added new detail code families:
  - `accepted_trail_and_nontrail_geometry_prepared_and_cmd_submit`
  - `accepted_trail_and_nontrail_geometry_prepared_cmd_submit_pending`
- Existing packet counters continue to work (`trail/ripple_click/ripple_hover/ripple_hold/particle/mixed`).

## CPU Fallback
- No change to CPU fallback semantics.
- Dawn submit failures still do not break CPU-compatible behavior.

## Expected Effect
- Better hold/hover/click ripple responsiveness while trail effect is active.
- Reduced perception of non-trail delay under high trail command rate.
