# Dawn GPU Host Chain Hidden HWND + DComp Target (Stage 119)

## Summary
Extended host-chain runtime activation from device-only setup to a minimal DirectComposition host chain with hidden probe HWND, target, and root visual.

## Why
Device-only activation was necessary but insufficient. Real host-chain readiness also requires target/root wiring on an actual window handle.

## What changed
1. `GpuFinalPresentHostChain` activation now creates:
- hidden probe HWND
- `IDCompositionTarget` via `CreateTargetForHwnd`
- `IDCompositionVisual` root
- `SetRoot` + `Commit`

2. Status observability expanded:
- `probe_hwnd_created`
- `dcomp_target_created`
- `dcomp_root_visual_created`

3. Existing counters remain:
- `activation_attempts`
- `activation_success`
- `activation_failure`

## Safety
- Still diagnostics/bring-up stage.
- No final-present takeover yet.
- Default layered CPU final-present path unchanged.

## Validation
- `Release|x64` build passed.

## Next
Bind this host-chain path to a dedicated non-layered present host and start controlled handoff from layered CPU final present.
