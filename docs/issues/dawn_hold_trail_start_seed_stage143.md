# Dawn Hold Trail Start Seed (Stage 143)

## Background
- Diagnostics still showed a few early hold-phase frames as `hold-only` (no trail command), especially around hold promotion timing.

## Change
- When `kHoldTimerId` promotes a press to real hold:
  - proactively seed trail with hold-down point (`pendingHold_.pt`)
  - then seed once more with current cursor point if it has moved
- This occurs before `HoldEffect::OnHoldStart`.

## Why
- Reduce startup gap where hold effect starts but trail has not yet formed enough points.
- Improve continuity perception at hold-entry boundary.

## Validation
- Build `Release|x64`.
- Compare hold timeline:
  - fewer early `hold-only` points
  - stable `hold_with_trail` ratio in recent hold window.

