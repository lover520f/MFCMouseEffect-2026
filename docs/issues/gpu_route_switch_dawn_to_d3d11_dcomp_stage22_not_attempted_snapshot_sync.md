# GPU Route Switch Stage 22: Not-Attempted Snapshot Sync

Date: 2026-02-13

## Goal
Keep takeover trial snapshot file in sync when takeover is skipped by fast guard.

## Changes
- Added `D3D11DCompPresenter::RecordTakeoverNotAttempted(const char* reason)`.
- `OverlayHostWindow::StartFrameLoop()` now records a `not_attempted` snapshot when `ShouldAttemptTakeover()` is false.

## Why
After Stage21, default-off runs no longer called takeover trial, which was correct, but `gpu_takeover_trial_result_auto.json` could remain stale from earlier sessions. This stage writes a fresh skip snapshot for deterministic diagnostics.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - default off: trial snapshot updates to `last_trial_result=not_attempted` with detail `takeover_not_attempted_by_fast_guard`.

## Risk
- Low. Diagnostics write only; no takeover policy or rendering behavior change.
