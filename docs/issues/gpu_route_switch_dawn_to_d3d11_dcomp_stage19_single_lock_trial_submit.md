# GPU Route Switch Stage 19: Single-Lock Trial Submit

Date: 2026-02-13

## Goal
Reduce per-frame lock overhead in render loop for DComp trial upload path.

## Changes
- Added `D3D11DCompPresenter::SubmitTrialFrameBGRAIfEnabled(...)`.
- `OverlayHostWindow::RenderSurface()` now uses the new single-call API.
- Refactored submit implementation into `SubmitTrialFrameBGRAUnlocked(...)` so enabled check and submit run under one lock.

## Why
Stage18 removed disabled-path submit calls, but enabled-path still took two presenter locks per frame (`IsTrialFrameUploadEnabled` + `SubmitTrialFrameBGRA`). This stage collapses that into one lock without changing behavior.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - Disabled default: no submit attempts, no skip growth.
  - Enabled visible trial: submit path behavior unchanged.

## Risk
- Low. Refactor-only for call path; counters and fallback logic remain unchanged.
