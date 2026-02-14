# Stage 32 - Neon GPU-v2 White Sector Artifact Fix (Angle Wrap)

## Symptom
- `hold_neon3d_gpu_v2` sometimes showed a bright white fan/sector that did not belong to the intended HUD3D look.
- Artifact became obvious when progress/head angle moved across angle wrap boundaries.

## Root cause
- Pixel shader helper `AngDist(a, b)` used:
  - `abs(a - b)` and `min(d, TAU - d)`
- When one angle exceeded the principal range and the other stayed in `[-PI, PI]`, `d` could be greater than `TAU`.
- In that case, `TAU - d` became negative, so distance became negative.
- Downstream expressions like `exp(-AngDist(...) * k)` then turned into exponential blow-up (because `-negative * k` is positive), producing white over-bright sectors.

## Fix
- Replaced `AngDist` with wrapped-angle delta based on `atan2(sin, cos)`:

```hlsl
float AngDist(float a, float b) {
    float wrapped = atan2(sin(a - b), cos(a - b));
    return abs(wrapped);
}
```

- This guarantees angular distance is always in `[0, PI]`, eliminating negative distance and preventing shader energy explosion.

## Files changed
- `MFCMouseEffect/MouseFx/Renderers/Hold/NeonHudGpuV2ShaderPipeline.h`

## Validation
1. Build `Release|x64` successfully.
2. Long-press with `hold_neon3d_gpu_v2`.
3. Confirm the previous white sector artifact is gone and the HUD ring remains stable during progress sweep.

## Compatibility impact
- No route change.
- No CPU fallback behavior change.
- Pure shader math correction only, low risk.
