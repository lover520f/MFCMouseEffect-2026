# Dawn hold bypass warmup skip (Stage 87)

## Problem
Startup warmup path could still increase perceived latency for `hold_neon3d`.

When warmup was active, any frame containing trail commands could be early-returned after noop submit, skipping geometry preprocessing for that frame.

## Root Cause
Warmup gating condition only checked:
- warmup active
- `status.trailCommandCount > 0`

It did not exclude latency-sensitive hold interactions.

## Changes
- File: `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
1. Moved `holdActive` calculation earlier (before warmup gating).
2. Changed warmup skip condition:
   - before: warmup active + trail commands
   - after: warmup active + trail commands + `!holdActive`
3. Result: hold-active frames are no longer dropped into warmup-preprocess-skip early return.

## Why this design
- Keeps existing warmup behavior for non-hold startup frames.
- Removes avoidable latency for real hold interaction path.
- No pipeline contract changes and no CPU fallback logic changes.

## Validation
1. Build with VS2026 Professional MSBuild succeeded.
2. Hold path can proceed through preprocessing during warmup window.
3. Non-hold warmup optimization remains intact.
