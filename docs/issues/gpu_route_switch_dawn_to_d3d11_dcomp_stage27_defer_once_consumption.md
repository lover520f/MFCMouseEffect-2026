# GPU Route Switch Stage 27: Defer One-Shot Consumption to Attempt Time

Date: 2026-02-13

## Goal
Avoid losing one-shot takeover triggers before real takeover attempt starts.

## Changes
- Changed `*.once` handling from "consume during control-decision init" to "consume when `TryActivateTakeoverPath()` actually starts attempt".
- Added `ConsumeOneShotControlFileForSource(...)` helper in `GpuTakeoverControl`.
- `file_once` and `file_visible_trial_once` now mark `*_file_consumed` only when attempt path is entered.

## Why
If app initialized but never reached real takeover attempt, old behavior could consume one-shot files prematurely, causing confusing "didn't trigger" runs.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - after creating one-shot file, it remains present until attempt starts.
  - on actual attempt start, file is consumed and `control_*_file_consumed=true`.

## Risk
- Low. Control-path timing fix only; takeover fallback safety unchanged.
