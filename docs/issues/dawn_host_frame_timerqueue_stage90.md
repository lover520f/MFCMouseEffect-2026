# Dawn host frame pump via timer queue (Stage 90)

## Problem
Even after interval tuning, diagnostics still showed frame submit cadence dominated by ~16ms.

`WM_TIMER`-based scheduling remained a practical limiter for low-latency cursor-follow behavior.

## Changes
- File: `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.h`
1. Added `kMsgFrameTick` message and timer queue callback declaration.
2. Added frame timer helpers:
   - `StartFrameTimer`
   - `StopFrameTimer`
3. Added `frameTimer_` handle state.

- File: `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.cpp`
1. Frame loop now prefers `CreateTimerQueueTimer` to drive periodic ticks.
2. Timer callback posts `kMsgFrameTick` to host window, then `OnTick` runs on window thread.
3. Dynamic interval updates now recreate timer queue period instead of relying on `WM_TIMER`.
4. Kept `WM_TIMER` as fallback only when timer queue creation fails.

## Why this design
- Improves tick scheduling stability compared to plain `WM_TIMER`.
- Keeps rendering and state updates on the existing window thread (no rendering thread split yet).
- Maintains fallback behavior and compatibility.

## Validation
1. Build with VS2026 Professional MSBuild succeeded.
2. Existing render path contracts and CPU fallback behavior were not changed.
