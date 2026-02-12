# Stage 97 - Dawn GPU Minimal Clear Pass Submission

## Goal
- Move from surface-lifecycle probe to real GPU command submission on surface textures.
- Keep CPU fallback behavior unchanged to avoid user-visible regressions.

## Changes
- Expanded `DawnRuntimePresentContext` with render-pass related procs:
  - texture view create/release
  - command encoder create/beginRenderPass/finish/release
  - command buffer release
  - queue submit
  - render pass encoder end/release
- `DawnGpuPresenter` now performs per-frame minimal GPU work:
  - acquire surface texture
  - create texture view
  - record a render pass with transparent clear
  - submit command buffer to queue
  - present surface
- Presenter still returns CPU fallback path for final composition:
  - preserves current effect correctness while GPU draw content migration is not complete.

## Why
- This stage proves command recording + queue submission + present pipeline is executable on runtime ABI.
- It removes another large unknown before replacing CPU effect rasterization with GPU-native rendering.

## Validation
- Build: `Release|x64` succeeded (`0` errors, `0` warnings).
- Runtime detail transitions from pure surface probe into command-submission-capable path (while still fallback rendering for visuals).

