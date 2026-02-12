# Dawn GPU presenter observability (Stage 94)

## Goal
Provide objective telemetry for GPU presenter landing progress, so we can clearly distinguish:
- GPU command submission active
- GPU presenter active/full
- CPU fallback still in effect

## Changes
- `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.h`
- `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.cpp`
  - Added GPU presenter counters and last-detail tracking:
    - attempts
    - success
    - fallback
    - active flag
    - last detail

- `MFCMouseEffect/MouseFx/Core/OverlayHostService.h`
- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
  - Exposed presenter metrics to higher layers.

- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
  - Added `gpu_presenter` block in state JSON.
  - Updated `gpu_acceleration` semantics:
    - `full` only when GPU presenter is active
    - otherwise remains `partial` even if Dawn backend/queue is ready

## Why this is root-direction
- Removes ambiguity where user sees `gpu_in_use=true` but output is still CPU-presented.
- Gives concrete completion signal for full GPU takeover.
- Enables stage-by-stage GPU landing with measurable acceptance criteria.

## Validation
1. Release build succeeded with VS2026 Professional MSBuild.
2. State JSON now reports presenter-level activity and fallback reason.
