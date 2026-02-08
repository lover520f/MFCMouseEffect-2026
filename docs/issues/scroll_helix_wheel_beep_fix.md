# Scroll Helix wheel beep fix

## Symptom

- Wheel beep can appear only when scroll renderer is `helix`.
- Slow wheel input is usually fine; burst wheel input is more likely to trigger the problem.

## Root cause

- `ScrollEffect` created one new ripple instance per wheel tick with no emission pacing.
- `helix` has significantly higher per-instance draw cost than simple scroll renderers.
- Under burst input, many helix instances overlapped in a short window, causing frame spikes and visible event jitter.

## Fix

### 1) Add helix-specific emission pacing

- File: `MFCMouseEffect/MouseFx/Effects/ScrollEffect.h`
- File: `MFCMouseEffect/MouseFx/Effects/ScrollEffect.cpp`
- Add `kHelixEmitIntervalMs = 14` to coalesce burst wheel ticks.
- Accumulate wheel delta during the interval and emit one combined visual pulse.

### 2) Add active helix instance cap

- File: `MFCMouseEffect/MouseFx/Effects/ScrollEffect.h`
- File: `MFCMouseEffect/MouseFx/Effects/ScrollEffect.cpp`
- Track active ripple ids for helix and cap concurrent instances (`kHelixMaxActiveRipples = 8`).
- Stop oldest active ripple when cap is exceeded.

### 3) Reduce helix per-frame complexity

- File: `MFCMouseEffect/MouseFx/Renderers/Scroll/HelixRenderer.h`
- Reduce segment count (`64 -> 44`) and tone down aura width/alpha.
- Keep double-helix structure while reducing burst render pressure.

## Verification checklist

1. Set scroll effect type to `helix`.
2. Burst-scroll on desktop and in regular app windows.
3. Confirm visual response remains smooth and no wheel beep is reproduced.
4. Switch to non-helix renderer and confirm behavior is unchanged.

## Notes

- This fix targets the real overload path in `helix` burst scenarios instead of global system sound workarounds.
