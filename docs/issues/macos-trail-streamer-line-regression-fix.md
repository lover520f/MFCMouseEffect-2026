# macOS trail: streamer matchsticks and line invisible regression

## Symptom
- `streamer` (绚丽流光) looked like disconnected matchsticks under fast movement.
- `line` (普通线条) sometimes appeared to have no visible output.

## Classification
- Bug/regression.

## Root Cause
1. `line` path used a separate coordinate conversion path from the main overlay pipeline, which could diverge from the coordinates used by other effects.
2. `line` renderer cleared path when only one sampled point existed, so short/slow movement could look like "no effect".
3. `line` branch consumed coalesced move events without interpolation, so sparse samples could appear as tiny dots.
4. `streamer` path style enforced large minimum segment length and high max segment length, producing stick-like segments.

## Fix
- Unified line trail overlay coordinates to the shared `ScreenToOverlayPoint(...)` path.
- Single-point line trail now renders a small dot (same color) instead of clearing immediately.
- `line` branch now interpolates move segments before feeding line overlay so coalesced events still produce continuous visible trails.
- `streamer` segment style tuned:
  - Lower min segment length.
  - Lower max segment length cap.
  - Higher interpolation density in trail emission.

## Files
- `MFCMouseEffect/Platform/macos/Effects/MacosLineTrailOverlay.mm`
- `MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayStyle.mm`
- `MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseEffect.mm`

## Verification
- Build:
  - `cmake --build /tmp/mfx-platform-macos-core-build --target mfx_entry_posix_host -j8`
- Gate:
  - `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
- Manual:
  - Switch trail to `line`, move slowly and quickly: trail should remain visible and continuous.
  - Switch trail to `streamer`, move quickly: significantly reduced matchstick-style long sticks.
