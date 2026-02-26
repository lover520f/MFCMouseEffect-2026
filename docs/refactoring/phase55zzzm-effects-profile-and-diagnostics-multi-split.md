# phase55zzzm: effects profile and diagnostics multi split

## Scope
- Capability bucket: `effects` + `state diagnostics` (large-scale maintainability split).
- Goal: perform one larger responsibility split pass across macOS effect profile resolution and state diagnostics assembly.

## Change Summary
1. Split macOS effect profile resolution by category:
   - Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosEffectRenderProfile.Shared.h`
   - Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosEffectRenderProfile.ClickTrail.cpp`
   - Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosEffectRenderProfile.ScrollHoldHover.cpp`
   - Kept `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosEffectRenderProfile.cpp` as thin anchor translation unit.
2. Split state diagnostics by concern:
   - Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.WasmDiagnostics.cpp`
   - Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.InputCaptureDiagnostics.cpp`
   - Reduced `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.Diagnostics.cpp` to GPU-route diagnostics only.
3. Build wiring:
   - Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`
   - Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`

## Contract Invariants
- `macos_effect_profile::*` public API signatures unchanged.
- `/api/state.wasm` and `/api/state.input_capture` payload contracts unchanged.
- `/api/state.effects_profile` payload contracts unchanged (builder split from previous slice preserved).

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`
