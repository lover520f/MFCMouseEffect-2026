# GPU Route Switch Stage 16: Trial Frame Upload to DComp Swapchain

Date: 2026-02-13

## Goal
Validate real frame-data path for takeover by uploading per-frame BGRA buffers into the DComp trial swapchain, while keeping layered output authoritative.

## Changes
- Added `D3D11DCompPresenter::SubmitTrialFrameBGRA(...)`:
  - validates args and readiness
  - resizes composition swapchain to frame size when needed
  - uploads CPU BGRA frame into swapchain backbuffer (`UpdateSubresource`)
  - performs `Present(0, 0)` on trial chain
- `OverlayHostWindow::RenderSurface()` now calls trial submit after `UpdateLayeredWindow`.
- Added status counters:
  - `trialFrameSubmitAttempts`
  - `trialFrameSubmitSuccess`
  - `trialFrameSubmitFailure`
- Exposed counters in `/api/state -> gpu_present_host`.

## Why
Previous stages validated device/target/swapchain construction. This stage validates real dynamic frame transport into takeover chain, which is a critical step before any visible final-present migration.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - with visible trial enabled and ready, submit success counter should increase.
  - layered path remains visible output regardless of submit outcome.

## Risk
- Medium (gated trial path active in render loop), but visible behavior remains protected by layered fallback authority.
