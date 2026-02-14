# Dawn Native Stage 12: Hold State Bridge for GPU-v2 Routes

Date: 2026-02-13

## Goal
Prepare a stable input/state bridge for future GPU hold renderers without touching the current visual stability path.

## What Changed
1. `HoldEffect` now emits a normalized hold-state command for `_gpu_v2` hold routes:
   - command: `hold_state`
   - payload format: `hold_ms,x,y`
   - file: `MFCMouseEffect/MouseFx/Effects/HoldEffect.cpp`
2. FluxField CPU renderer now understands hold timing commands directly:
   - `hold_ms`
   - `threshold_ms`
   - `hold_state`
   - progress uses hold-time bias alignment instead of only frame `t`
   - file: `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudCpuRenderer.h`
3. FluxField GPU-v2 placeholder renderer now consumes and stores `hold_state` as route-local runtime state (for next real GPU backend step) while keeping stable CPU draw fallback.
   - file: `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`

## Why
- Keep current "no regression" rendering behavior.
- Remove future coupling between GPU backend implementation and raw UI event timing details.
- Provide one consistent hold-state protocol so backend replacement is localized.

## Validation
- Build passes with VS 2026 `Release|x64`.
- Existing hold visual route remains stable (renderer output still based on proven CPU path).
