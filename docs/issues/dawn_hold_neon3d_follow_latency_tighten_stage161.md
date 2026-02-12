# Dawn Hold Neon3D Follow Latency Tighten (Stage 161)

## Background
- Under current stable path (`GPU command + layered CPU final present`), hold_neon3d still showed visible cursor-follow lag.
- The lag mainly came from two throttles:
  - hold continuous command emission interval in ripple GPU command stream.
  - smooth follow policy in `HoldEffect` under layered final-present path.

## Change
- Tighten hold continuous GPU command interval:
  - `RippleOverlayLayer`: hold continuous min interval `6ms -> 2ms`.
- Improve layered-path hold follow responsiveness:
  - `HoldEffect` smooth mode now uses stronger baseline alpha in low-speed layered case.
  - Add direct-follow snap in layered neon3d when:
    - movement speed is high, or
    - cursor-smooth-point gap is large.

## Why
- Reduce perceived hand lag during long-press drag for hold_neon3d without changing final-present safety policy.
- Keep architecture stable while improving the current practical path.

## Validation
- Build `Release|x64`.
- Manual: long-press `hold_neon3d` and move fast with mixed direction changes.
- Expect reduced cursor/effect separation and quicker catch-up under layered fallback.
