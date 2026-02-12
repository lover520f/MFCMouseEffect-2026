# Dawn hold-priority non-trail throttle (Stage 85)

## Problem
`hold_neon3d` still felt delayed when the frame only had non-trail geometry (ripple/particle).  
The generic non-trail submit throttle (`8ms`) was still applied, which added avoidable hold latency.

## Root Cause
In `SubmitOverlayGpuCommands`, non-trail-only frames used a fixed throttle window:
- `kNonTrailSubmitIntervalMs = 8`
- No hold-aware branch for latency-sensitive continuous hold updates

So hold frames were treated the same as hover/click non-trail frames.

## Changes
- File: `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
1. Added hold-specific non-trail throttle interval:
   - `kNonTrailSubmitIntervalWhenHoldMs = 2`
2. In non-trail-only branch, throttle interval now selects by runtime hold state:
   - hold active -> 2ms window
   - otherwise -> keep 8ms window
3. Added clearer diagnostic detail for hold throttling:
   - `accepted_nontrail_geometry_submit_throttled_hold_priority*`

## Why this design
- Keeps original protection for normal non-trail traffic (hover/click/mixed).
- Gives hold path much lower scheduling latency without fully removing guardrails.
- Small isolated change, no architecture break, no CPU fallback path impact.

## Validation
1. Build with VS2026 Professional MSBuild succeeded.
2. Existing CPU fallback gating was not modified in this commit.
3. Diagnostic detail string can distinguish hold-priority throttling from normal throttling.
