# Scroll wheel input shaper (unified)

## Symptom

- Wheel beep was easiest to reproduce with scroll renderer `helix`.
- Slow wheel input was usually fine; burst wheel input was more likely to trigger the problem.

## Root cause

- `ScrollEffect` originally created one new ripple instance per wheel tick with no input shaping.
- This allowed burst wheel packets to create short-lived visual instance storms.
- `helix` exposed the issue first because its per-instance cost is higher.

## Fix

### 1) Introduce unified scroll input shaping

- File: `MFCMouseEffect/MouseFx/Effects/ScrollEffect.h`
- File: `MFCMouseEffect/MouseFx/Effects/ScrollEffect.cpp`
- Add a common shaper profile (`emit interval`, `max active`, `max duration`).
- Coalesce burst wheel ticks by accumulating delta inside one emission interval.
- Apply shaping to all scroll renderers.

### 2) Keep renderer-specific profile tuning

- File: `MFCMouseEffect/MouseFx/Effects/ScrollEffect.h`
- File: `MFCMouseEffect/MouseFx/Effects/ScrollEffect.cpp`
- Default profile is moderate for simple renderers.
- `helix` profile remains stricter (`14ms`, max `8` active, max duration `240ms`).
- Track active ripple ids and stop oldest instance when cap is exceeded.

### 3) Reduce helix per-frame complexity

- File: `MFCMouseEffect/MouseFx/Renderers/Scroll/HelixRenderer.h`
- Reduce segment count (`64 -> 44`) and tone down aura width/alpha.
- Keep double-helix structure while reducing burst render pressure.

## Verification checklist

1. Set scroll effect type to `helix` and burst-scroll on desktop/app windows.
2. Confirm no wheel beep and no severe frame hitch.
3. Switch to non-helix renderer (`arrow`/`chevron`) and repeat burst-scroll.
4. Confirm all scroll renderers keep smooth feedback under fast wheel input.

## Notes

- This fix keeps root-cause handling in the app input/render chain.
- No system-level sound toggles are required.
