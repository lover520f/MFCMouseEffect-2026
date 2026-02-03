# Trail Effects: Differentiation + Shared History Fix

## Summary
The trail effects **Meteor Shower**, **Neon Streamer**, and **Cyber Electric** used to feel too similar because they were all constrained by the same `TrailWindow` history window (fixed `durationMs_` and `maxPoints_`). Even if a renderer wanted a longer/shorter tail, the window would prune points early, producing nearly the same trail length and density.

This change:
- Adds per-type trail history profiles (duration + max points).
- Splits the old monolithic trail renderer header into focused renderer headers.
- Removes global `rand()/srand()` usage in trail renderers.
- Tunes each renderer so the three effects have clearly different “identity”.

## Root Cause
`TrailWindow` prunes old points every tick using its internal `durationMs_` and `maxPoints_`.
- Previously these were effectively fixed for all trail types.
- Some renderers also had their own internal `durationMs`, but it could not take effect because points were already removed.

Result: tails were similarly short and similarly sampled, so visuals converged.

## Implementation

### 1) Per-type history profile
Profiles are applied in:
- `MFCMouseEffect/MouseFx/Effects/TrailEffect.cpp`

Current values:
- `electric`: `durationMs=280`, `maxPoints=24`
- `streamer`: `durationMs=420`, `maxPoints=46`
- `meteor`: `durationMs=520`, `maxPoints=60`
- `tubes/scifi`: `durationMs=350`, `maxPoints=40`
- default `line`: `durationMs=300`, `maxPoints=32`

### 2) TrailWindow configurability
`TrailWindow` now exposes:
- `SetDurationMs(int)`
- `SetMaxPoints(int)`

So renderers can actually “see” enough history to look different.

### 3) Renderer split (single responsibility)
Old file: `MouseFx/Interfaces/TrailRenderStrategies.h`

Now it is an include-only aggregator for:
- `MouseFx/Renderers/Trail/LineTrailRenderer.h`
- `MouseFx/Renderers/Trail/StreamerTrailRenderer.h`
- `MouseFx/Renderers/Trail/ElectricTrailRenderer.h`

### 4) Visual identity tuning (high level)
- **Streamer**: two-pass neon stroke (outer glow + inner core), head-weighted thickness.
- **Electric**: stabilized arc jitter (per-frame bucket seed) + white core + occasional fork.
- **Meteor**: warmer tail palette (non-chromatic), stronger sparks, directional head flare.

### 5) Shared utilities (reuse)
- `MouseFx/Utils/TrailColor.h` (HSL → `Gdiplus::Color`)
- `MouseFx/Utils/TrailMath.h` (clamp, idle fade factor)
- `MouseFx/Utils/XorShift.h` (lightweight PRNG + seed mix)

## Manual Verification
1. Run `x64\\Debug\\MFCMouseEffect.exe`.
2. Settings → Trail:
   - Switch between `meteor / streamer / electric`.
3. Move the mouse in circles and then stop:
   - Electric should snap/fade quickly with arc forks.
   - Streamer should look like a smooth neon ribbon with glow.
   - Meteor should look warmer with spark particles and a head flare.

