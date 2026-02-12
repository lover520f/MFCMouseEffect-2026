# Dawn Async Probe Auto-Activation Fix (Stage 109)

## Summary
Fixed a backend-state dead zone where `render_backend` was set to Dawn, runtime queue was already ready, but `activeBackend` remained CPU until the user toggled backend again.

## Root Cause
Async runtime probe completed in background, but backend activation state (`activeBackend_`, `pipelineMode_`, host submit context) was not synchronized from probe result.

## Fix
- Added `OverlayHostService::SyncBackendActivationFromRuntime`.
- On async probe completion, automatically promote to Dawn when queue becomes ready (`requested=dawn|auto`).
- On probe-refresh path, also synchronize backend activation immediately.

## Files
- `MFCMouseEffect/MouseFx/Core/OverlayHostService.h`
- `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`

## Validation
- Build passes (`Release|x64`).
- Expected runtime behavior:
  - `render_backend: dawn`
  - `render_backend_active: dawn` (after async probe readiness)
  - `dawn_command_consumer.detail` no longer stuck at `backend_not_dawn` for Dawn request path.

## Impact
This is a root-state fix for backend activation convergence. It does not bypass existing CPU safety fallback for final present.
