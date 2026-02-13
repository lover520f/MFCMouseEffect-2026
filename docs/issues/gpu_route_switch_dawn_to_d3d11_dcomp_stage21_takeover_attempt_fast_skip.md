# GPU Route Switch Stage 21: Takeover Attempt Fast Skip

Date: 2026-02-13

## Goal
Avoid unnecessary takeover trial calls when takeover is deterministically not runnable.

## Changes
- Added `D3D11DCompPresenter::ShouldAttemptTakeover()`.
- `OverlayHostWindow::StartFrameLoop()` now calls `TryActivateTakeoverPath()` only when `ShouldAttemptTakeover()` returns true.

## Why
Default runtime usually has takeover disabled. Calling takeover trial unconditionally caused extra lock work and repeated skip-result updates with no value.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - default off: no takeover trial attempt on frame-loop start.
  - enabled/eligible path: behavior unchanged, trial still executes.

## Risk
- Low. Pure fast-path guard; no change to takeover decision logic itself.
