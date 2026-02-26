# phase55zzzj: macOS scroll overlay style normalization split

## Scope
- Capability bucket: `effects` (macOS scroll effect renderer maintainability).
- Goal: move scroll-type normalization into the existing scroll style module.

## Change Summary
1. Expanded scroll style module:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayStyle.h`
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayStyle.mm`
2. `MacosScrollPulseOverlayRenderer.mm` now delegates:
   - `NormalizeScrollType`
3. Renderer remains focused on overlay lifecycle + animation wiring.

## Contract Invariants
- Scroll overlay public APIs remain unchanged:
  - `CloseAllScrollPulseWindows`
  - `ShowScrollPulseOverlay`
- `arrow/helix/twinkle` type semantics remain unchanged.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
