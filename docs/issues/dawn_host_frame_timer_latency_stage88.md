# Dawn host frame timer latency baseline (Stage 88)

## Problem
User reported severe cursor-to-effect lag in `hold_neon3d`, even when Dawn backend was active.

Local diagnostic snapshot showed:
- `gpu_in_use = true`
- `queue_ready = true`
- command submit timeline still frequently spaced at ~16ms / 31ms

This indicated the bottleneck was host frame-loop cadence, not GPU readiness.

## Root Cause
`OverlayHostWindow` used a fixed `SetTimer(..., 16ms)` loop.

That hard-capped command collection/submission cadence to around 60Hz (or worse under timer coalescing), causing visible lag against mouse movement.

## Changes
- File: `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.cpp`
1. Raised frame-loop baseline interval:
   - from fixed 16ms -> dynamic 8ms baseline
2. Added hold-sensitive fast path:
   - if hold-continuous ripple command exists in current frame, next interval becomes 4ms
3. Added loop-scoped high-resolution timer request:
   - `timeBeginPeriod(1)` on frame-loop start
   - `timeEndPeriod(1)` on frame-loop stop
4. Timer interval updates are applied only when value changes to avoid unnecessary timer churn.

- File: `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.h`
1. Added timer interval helper declarations.
2. Added state fields for current interval, high-resolution state, and hold command counter.

## Why this design
- Directly targets the measured latency root cause (host tick cadence).
- Keeps changes local to host scheduling layer; does not alter rendering architecture or CPU fallback contracts.
- Enables high responsiveness during hold while preserving a controlled baseline outside hold.

## Validation
1. Build with VS2026 Professional MSBuild succeeded.
2. GPU submit path and CPU fallback path interfaces were unchanged.
3. Timer precision is only raised while the overlay frame loop is active.
