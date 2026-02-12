# Dawn GPU Final Present Auto Rollback (Stage 112)

## Summary
Enabled a guarded non-layered GPU final-present attempt path for Dawn compositor mode, with immediate automatic rollback to layered CPU final present when GPU present fails.

## Why
We were stuck at "GPU command path active but CPU final present forever", and previous non-layered attempts could cause black screen risk. This stage moves forward with strict gates plus deterministic fallback.

## What changed
1. `OverlayHostWindow` now allows non-layered host surfaces only when all gates pass:
- backend is `dawn`
- pipeline mode is `dawn_compositor`
- no `forceLayeredCpuFallback`
- all active layers report `SupportsGpuExclusivePresent() == true`
- `gpu_final_present_capability.likely_available == true`

2. Added runtime rollback guard:
- if non-layered GPU present fails, mark pending rollback immediately
- next tick forces rebuild back to layered surfaces
- lock into layered CPU fallback for stability until backend/pipeline context changes

3. Failure tracking:
- consecutive GPU present failures are tracked and reset on success/context change
- presenter detail appends `_auto_rollback_layered_cpu` when rollback is triggered

## Safety
- CPU fallback remains authoritative.
- Black-screen exposure window is reduced to a bounded single-failure rollback cycle.
- No API contract changes.

## Validation
- `Release|x64` build passed.

## Next
Add a dedicated DirectComposition host chain so non-layered GPU final present no longer depends on legacy overlay host behavior.
