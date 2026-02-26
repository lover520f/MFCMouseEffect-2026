# phase55zzzl: effects profile state builder split

## Scope
- Capability bucket: `effects` (state diagnostics maintainability).
- Goal: split `/api/state.effects_profile` payload assembly into dedicated builder unit.

## Change Summary
1. Added:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.EffectsProfileStateBuilder.h`
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.EffectsProfileStateBuilder.cpp`
2. `SettingsStateMapper.EffectsDiagnostics.cpp` now delegates `BuildEffectsProfileState` to `BuildEffectsProfileStateJson`.
3. Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt` to include the new builder source.

## Contract Invariants
- `/api/state.effects_profile` schema and field semantics unchanged.
- Existing effects profile contracts in regression remain unchanged.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
