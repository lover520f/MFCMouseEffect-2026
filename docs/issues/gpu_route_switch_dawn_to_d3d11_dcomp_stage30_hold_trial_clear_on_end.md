# GPU Route Switch Stage 30: Clear Trial Frame After Hold Ends

Date: 2026-02-13

## Goal
Remove stale hold-neon3d image that could remain on secondary monitor after hold ends.

## Symptom
After releasing long-press, effect image could stay on secondary monitor until next hold action.

## Root Cause
Stage29 gated trial uploads strictly to hold-active period. When hold ended, no more uploads were sent, so visible-trial swapchain could keep the last frame.

## Changes
- Added `holdNeon3dGpuTrialNeedsClear_` in `OverlayHostWindow`.
- On hold gate transition `true -> false`, schedule one extra trial upload.
- `RenderSurface()` now allows trial upload when:
  - hold is active, or
  - clear-on-end is pending.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - no stale hold image remains on secondary monitor after releasing long-press.

## Risk
- Low. One extra trial upload only on hold-end transition; layered authoritative rendering unchanged.
