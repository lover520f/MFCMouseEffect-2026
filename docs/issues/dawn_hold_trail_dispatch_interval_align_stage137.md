# Dawn Hold Trail Dispatch Interval Align (Stage 137)

## Context
- After stage136, trail visibility during hold was restored.
- Diagnostics still showed a small fraction of hold frames without trail commands.

## Change
- Align hold-priority trail dispatch interval with downstream hold submit cadence:
  - `AppController::kHoldNeon3DTrailDispatchIntervalMs`
  - `14ms -> 8ms`

## Why
- `14ms` caps trail move dispatch to ~71Hz, which can still feel behind during rapid hold movement.
- `8ms` aligns with existing Dawn hold-side scheduling and better matches high-refresh cursor motion.

## Expected Effect
- More hold frames should carry both `trail` and `hold` commands.
- Lower perceived cursor-to-trail delay during `hold_neon3d`.

## Validation
- Build should pass on `Release|x64` using VS2026 MSBuild path.

