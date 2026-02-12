# Dawn Hold Neon3D Latency Priority (Stage 127)

## Summary
Reduced long-hold Neon HUD latency in the current layered final-present architecture by prioritizing hold tracking and adaptively reducing trail follow pressure.

## Why
When backend is `dawn` but final present is still layered CPU fallback, hold interactions can feel delayed because:
- heavy hold rendering and trail updates compete on the same CPU present path
- smooth follow filtering lags during fast cursor movement

## What Changed
1. `AppController` hold-priority trail dispatch:
- New hold-latency priority path for `hold_neon3d` while button is down.
- Active only when backend is `dawn` and GPU final presenter is not active.
- Trail move dispatch is throttled to ~14 ms during that phase to protect hold follow responsiveness.

2. `HoldEffect` smooth follow adaptive alpha:
- For Neon3D in `smooth` mode, smoothing alpha now adapts by cursor speed.
- Under layered CPU final-present, fast moves use higher alpha (stronger follow), reducing visible cursor/effect lag.
- Slow moves keep moderate smoothing to avoid jitter.

## Safety
- No black-screen risk path is introduced.
- Behavior is scoped to hold + Neon3D + dawn backend + layered fallback.
- Other hold types and modes keep existing behavior.

## Validation
- `Release|x64` build passed with VS 2026 MSBuild (amd64 path).

## Next
- Continue final-present takeover integration in an architecture-complete path.
- Keep CPU fallback as hard safety net while staged GPU final-present is verified.
