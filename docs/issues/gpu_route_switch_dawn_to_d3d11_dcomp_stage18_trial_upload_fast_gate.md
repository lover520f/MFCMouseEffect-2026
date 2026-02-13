# GPU Route Switch Stage 18: Trial Upload Fast Gate

Date: 2026-02-13

## Goal
Reduce render-loop overhead and diagnostics noise when trial upload path is not enabled.

## Changes
- Added `D3D11DCompPresenter::IsTrialFrameUploadEnabled()`.
- `OverlayHostWindow::RenderSurface()` now calls `SubmitTrialFrameBGRA(...)` only when fast gate is open.
- Fast gate condition:
  - `visibleTrialEnabled && takeoverEnabled`

## Why
Stage17 clarified counters, but render loop still invoked trial submit every frame when disabled. This stage removes unnecessary lock/branch work and prevents meaningless skip growth.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - default setup: no per-frame trial submit calls, skip counters stay stable.

## Risk
- Low. Gate is conservative and only suppresses disabled-path calls.
