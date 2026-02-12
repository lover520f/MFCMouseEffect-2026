# Dawn Hold Input Immediate Frame Kick (Stage 162)

## Background
- Hold updates can arrive between host timer ticks.
- Without an immediate frame kick, visual follow may wait for the next timer cadence, adding visible lag.

## Change
- In `OverlayHostService`, request immediate frame after hold-related ripple updates:
  - `UpdateRipplePosition`
  - `UpdateRippleHoldElapsed`
  - `UpdateRippleHoldThreshold`

## Why
- Convert input updates into near-immediate render opportunities.
- Reduce hold_neon3d cursor/effect separation on the current layered final-present path.

## Validation
- Build `Release|x64`.
- Hold + fast drag; verify better follow responsiveness and no black-screen regression.
