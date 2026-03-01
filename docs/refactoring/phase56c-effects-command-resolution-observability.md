# phase56c: effects command resolution observability

## Why
- We already expose raw active type and normalized aliases in macOS effect profile probes.
- To close “settings value -> actual render command” verification, probe output should include resolved command types directly.

## Changes
- Extended macOS effect command samples:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.EffectsProfileStateBuilder.Macos.cpp`
  - new fields:
    - `sample_input.hold_follow_mode_normalized`
    - `active_resolved` (`click/trail/scroll/hold/hover` from computed render commands)
- Extended effects contract assertions:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_effect_overlay_checks.sh`
  - asserts `active_resolved` and normalized hold-follow mode presence.
- Extended manual effects parity selfcheck:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh`
  - asserts the same two fields in `/api/effects/test-render-profiles`.

## Validation
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto --build-dir /tmp/mfx-platform-macos-build`
- Result: passed.
