# Dawn Hold Trail Source Gating Removal (Stage 138)

## Background
- Recent diagnostics still showed a minority of hold frames as `hold-only` (no trail command).
- Even after downstream Dawn submit tuning, source-side move dispatch still had an extra hold-priority trail gate in `AppController`.

## Root Change
- Removed source-level hold trail secondary throttling from `AppController`:
  - deleted `ShouldDispatchTrailDuringHoldPriority(...)`
  - removed `kHoldNeon3DTrailDispatchIntervalMs` and related state
  - trail move dispatch now follows `allowMoveDispatch` directly
- Retained only drag-like move gating (`ShouldDispatchDragMove`) as the single upstream gate.

## Why
- Keep one clear responsibility per stage:
  - source layer: capture and dispatch move events
  - consumer/submit layer: prioritize/limit GPU packet submits
- Avoid stacked throttles that can suppress trail commands during hold.

## Expected Result
- Higher probability that hold frames include both trail and hold commands.
- Lower chance of visible trail intermittently dropping during rapid hold movement.

## Validation
- Build should pass in `Release|x64`.
- Compare `dawn_command_consumer_timeline`:
  - `hold_with_trail` ratio should improve or stay stable.

