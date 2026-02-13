# Dawn Native Stage16: Hold Runtime Diag Write-Throttle Fix

## Background
After adding `hold_runtime_auto.json` diagnostics, hold update path wrote JSON on every update tick (8ms in smooth mode / 20ms in efficient mode).

This introduced avoidable IO on the hot path and caused visible long-press stutter and higher CPU.

## Root Cause
`HoldEffect::OnHoldUpdate` performed synchronous file write in the per-frame command block.

## Fix
- Keep runtime snapshots for lifecycle boundaries only:
  - `hold_start`
  - `hold_end`
- Remove hot-path snapshot write from `hold_update`.

## Files Changed
- `MFCMouseEffect/MouseFx/Effects/HoldEffect.cpp`

## Validation
- Rebuilt `Release|x64` with VS2026 MSBuild (0 errors, 0 warnings).
- Expected behavior:
  - Long-press interaction no longer incurs per-update disk IO.
  - CPU overhead from diagnostics is reduced.

## Notes
This change is orthogonal to D2D COM first-chance exceptions and focuses strictly on hold-path responsiveness.
