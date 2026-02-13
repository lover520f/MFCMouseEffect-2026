# GPU Route Switch Stage 17: Trial Submit Counter Hygiene

Date: 2026-02-13

## Goal
Improve observability quality by separating trial submit skips from true failures.

## Changes
- Added new status counters:
  - `trialFrameSubmitSkippedDisabled`
  - `trialFrameSubmitSkippedNotReady`
- `SubmitTrialFrameBGRA(...)` behavior update:
  - visible trial disabled -> count as `skipped_disabled` (not failure)
  - trial not ready/swapchain not ready -> count as `skipped_not_ready` (not failure)
  - only real submit-path errors count as `trialFrameSubmitFailure`
- Exposed new counters via `/api/state -> gpu_present_host`.

## Why
Without skip/failure separation, logs can over-report failures and hide real issues. This stage makes trial-path health signals actionable.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - default (visible trial off): `skipped_disabled` increases, failure remains stable.

## Risk
- Low. Metrics-only refinement; rendering behavior unchanged.
