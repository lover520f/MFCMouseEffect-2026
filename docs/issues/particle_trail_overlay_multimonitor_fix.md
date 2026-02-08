# Particle Trail Multi-Monitor Coordinate Fix

## Symptom
- `trail = particle` looked like `none` on some setups after OverlayHost moved to per-monitor layered windows.
- Other trail renderers (line/streamer/electric/meteor/tubes) still worked.

## Root Cause
- `ParticleTrailOverlayLayer` converted screen points to overlay-local points during `Update/Emit`.
- In per-monitor mode, overlay origin changes per surface during `Render`.
- This mismatch made particle points bind to a stale/wrong monitor origin, so they were drawn outside the visible area.

## Fix
- Store particle positions in screen space only.
- Convert to local space in `Render` using `ScreenToOverlayPoint` for the current surface.

## Impact
- Restores visible rainbow particle trail across monitor layouts.
- No behavior change for non-particle trail types.
