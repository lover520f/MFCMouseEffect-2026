# Dawn GPU Final Present Policy Module (Stage 114)

## Summary
Extracted GPU final-present eligibility logic into a dedicated policy module under `MouseFx/Gpu`, and added startup/interaction guards to reduce regression risk while continuing architecture landing.

## Why
`OverlayHostWindow` previously mixed window lifecycle and many GPU-present gating conditions. That made it harder to reason about behavior and risky to evolve toward a dedicated DirectComposition host chain.

## What changed
1. Added policy module:
- `MouseFx/Gpu/GpuFinalPresentPolicy.h`
- `MouseFx/Gpu/GpuFinalPresentPolicy.cpp`

2. Policy input includes:
- opt-in flag
- forced layered rollback state
- backend + pipeline mode
- active layer count + gpu-exclusive compatibility
- runtime capability probe result
- process uptime
- recent non-empty GPU command activity

3. Policy outputs:
- `useLayeredSurfaces`
- `eligibleForGpuFinalPresent`
- `detail`

4. `OverlayHostWindow` now consumes policy:
- `ShouldUseLayeredSurfaces()` delegates to policy
- layered-present diagnostic detail uses policy detail
- tracks `lastNonEmptyGpuCommandTickMs` for interaction-window gating

## Safety
- Default stable behavior unchanged: still layered CPU final present without opt-in.
- Even with opt-in, startup guard and interaction guard prevent aggressive startup-time attempts.

## Validation
- `Release|x64` build passed.

## Next
Use the same policy output to switch between layered host and a future dedicated DirectComposition GPU final-present host chain.
