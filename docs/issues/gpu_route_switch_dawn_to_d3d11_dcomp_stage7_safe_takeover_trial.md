# GPU Route Switch Stage 7: Safe Takeover Trial Skeleton

Date: 2026-02-12

## Goal
Introduce a deterministic takeover trial entrypoint with automatic fallback accounting, while keeping layered final present as the only visible render path.

## Changes
- Added `D3D11DCompPresenter::TryActivateTakeoverPath()`.
  - Runs only when initialized and takeover is enabled+eligible.
  - Attempts at most once per process run.
  - Current stage behavior: records trial and immediately falls back to layered path.
- `OverlayHostWindow::StartFrameLoop()` now triggers one takeover trial attempt.
- Added status fields:
  - `takeoverActive`
  - `takeoverAttempts`
  - `takeoverFallbacks`
- Exposed new fields through `/api/state -> gpu_present_host`.

## Why
This creates a controlled checkpoint before real takeover implementation. We can verify gating/control logic and rollback telemetry without risking black-screen regressions.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - when takeover is enabled and eligible, `takeover_attempts` increments once
  - `takeover_fallbacks` increments correspondingly
  - `takeover_active` remains false in this stage

## Risk
- Low. Visible rendering remains layered CPU present.
