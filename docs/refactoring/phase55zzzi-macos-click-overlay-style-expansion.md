# phase55zzzi: macOS click overlay style expansion

## Scope
- Capability bucket: `effects` (macOS click effect renderer maintainability).
- Goal: move click-type normalization and star-path construction into click style module.

## Change Summary
1. Expanded click style module contracts:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayStyle.h`
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayStyle.mm`
2. `MacosClickPulseOverlayRenderer.mm` now delegates:
   - `NormalizeClickType`
   - `CreateClickPulseStarPath`
3. Renderer keeps lifecycle/animation behavior only.

## Contract Invariants
- Click overlay public APIs remain unchanged:
  - `CloseAllClickPulseWindows`
  - `ShowClickPulseOverlay`
- `ripple/star/text` type behavior and visual output semantics remain unchanged.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
