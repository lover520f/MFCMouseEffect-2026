# Phase 56f - Effects Alias Matrix Contract Coverage

## Goal
Lock effect-type alias normalization behavior with explicit contract probes, so future refactors do not silently change type semantics.

## Change
- Extended `/api/effects/test-render-profiles` `command_samples` payload with:
  - `alias_matrix` (input type -> normalized type) for all effect categories.
- Covered representative aliases:
  - click: `TEXT -> text`
  - trail: `stream -> streamer`
  - scroll: `stardust -> twinkle`
  - hover: `suspension -> tubes`
  - hold: `hold_neon3d_gpu_v2 -> hold_quantum_halo_gpu_v2`
- Updated effects regression assertions to validate these mappings.

## Files
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.EffectsProfileStateBuilder.Macos.cpp`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_effect_overlay_checks.sh`

## Validation
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
  - passed

## Risk
- Low: test-route payload extension only; production runtime behavior unchanged.
