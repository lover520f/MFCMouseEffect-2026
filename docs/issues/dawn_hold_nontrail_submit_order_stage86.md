# Dawn hold non-trail submit order (Stage 86)

## Problem
Under `hold_neon3d` pressure, frames that contain both trail and non-trail geometry could still feel delayed.

Even with hold-priority non-trail throttle tuning, command submission order was still trail-first in the mixed frame path.

## Root Cause
In `SubmitOverlayGpuCommands`, the branch for `hasTrailGeometry && hasNonTrailGeometry` submitted trail packet before non-trail packet.

For hold interaction, non-trail geometry is the latency-sensitive HUD path, so trail-first ordering can delay visible hold response.

## Changes
- File: `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
1. In mixed trail+non-trail frames:
   - if `holdActive`, submit non-trail packet first, then submit trail packet
   - otherwise, keep original trail-first behavior
2. Kept existing trail throttling and counters intact.
3. Kept success semantics intact (`cmdSubmitOk = trailOk && nonTrailOk`) to avoid behavior drift.

## Why this design
- Targets hold latency directly without changing pipeline contracts.
- Limits behavior change to hold+mixed path only.
- Preserves existing diagnostics and fallback-safe logic.

## Validation
1. Build with VS2026 Professional MSBuild succeeded.
2. CPU fallback path not modified.
3. Mixed path now supports hold-aware submission ordering.
