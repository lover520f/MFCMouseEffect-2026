# Dawn timer queue regression rollback (Stage 91)

## Problem
After switching host frame pump to timer queue, user observed behavioral regression:
- click/trail felt desynchronized
- effects appeared inconsistent ("each doing its own thing")

## Action
Applied immediate rollback to restore known-stable scheduling behavior.

## Changes
- File: `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.h`
1. Removed timer-queue specific declarations and state fields.

- File: `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.cpp`
1. Removed timer-queue callback/message path.
2. Restored `WM_TIMER` as the primary frame tick mechanism.
3. Kept previously validated dynamic intervals:
   - default 8ms
   - hold 4ms
4. Kept loop-scoped high-resolution timer request (`timeBeginPeriod/timeEndPeriod`).

## Why
- Priority is user-visible correctness and consistent behavior.
- Reverting risky scheduler change is lower-risk than layering more fixes on top of regression.

## Validation
1. Release build succeeded after rollback.
2. Scheduling path is back to the prior stable model.
