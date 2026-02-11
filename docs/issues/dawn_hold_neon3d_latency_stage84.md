# Dawn hold_neon3d latency reduction (Stage 84)

## Problem
- User feedback: `hold_neon3d` still feels laggy under Dawn path.
- Existing continuous command throttle and smoothing were tuned more for stability than input immediacy.

## Changes
1. `MFCMouseEffect/MouseFx/Layers/RippleOverlayLayer.cpp`
- Continuous GPU emit policy now distinguishes hover vs hold:
  - hover continuous: `14ms` (unchanged)
  - hold continuous: `6ms` (was shared `10ms`)
- Purpose: reduce hold-path update spacing without reopening full command storms.

2. `MFCMouseEffect/MouseFx/Effects/HoldEffect.cpp`
- For `hold_neon3d` / `neon3d` in `smooth` follow mode:
  - smoothing alpha raised to `0.58` (from `0.35`)
  - hold elapsed command interval reduced to `4ms` (from `8ms`)
- Purpose: reduce perceived pointer-follow and progress-update lag in the most latency-sensitive hold renderer.

## Scope and safety
- No architecture change, no API change.
- CPU fallback behavior unchanged.
- Efficient mode remains conservative (`20ms`) for lower CPU budgets.

