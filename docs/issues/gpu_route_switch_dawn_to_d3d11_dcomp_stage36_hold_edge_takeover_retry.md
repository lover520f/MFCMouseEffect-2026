# GPU Route Switch Stage 36: Retry Takeover Guard on Hold Activation Edge

Date: 2026-02-13

## Goal
Avoid missing the visible-trial takeover attempt window when frame-loop starts before `hold_neon3d` becomes active.

## Changes
- Added `OverlayHostWindow::TryActivateTakeoverIfReady()` as the single takeover-attempt entry.
- Reused this entry in two paths:
  - frame-loop startup (`StartFrameLoop`)
  - `SetHoldNeon3dGpuTrialActive(true)` activation edge
- Guard behavior remains unchanged:
  - only attempts when fast-guard is open
  - otherwise records exact fast-guard reason snapshot.

## Why
- Prior behavior only checked takeover at frame-loop start.
- In real flow, hold activation may occur after frame-loop is already ticking, which could skip trial attempts entirely.
- This update aligns takeover attempt timing with actual `hold_neon3d` lifecycle.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - if startup skipped due to `takeover_wait_hold_neon3d_active`, entering hold can trigger a new guard check/attempt.

## Risk
- Low.
- No change to layered authoritative present path or fallback policy.
