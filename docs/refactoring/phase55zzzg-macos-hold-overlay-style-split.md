# phase55zzzg: macOS hold overlay style split

## Scope
- Capability bucket: `effects` (macOS hold effect renderer maintainability).
- Goal: split hold-style parsing/color/path construction from hold overlay lifecycle logic.

## Change Summary
1. Added style module:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayStyle.h`
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayStyle.mm`
2. Kept `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayRenderer.mm` focused on:
   - overlay window lifecycle
   - state storage/update
   - render animation wiring
3. Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt` to include the new style translation unit.

## Contract Invariants
- Hold overlay public APIs remain unchanged:
  - `StartHoldPulseOverlay`
  - `UpdateHoldPulseOverlay`
  - `StopHoldPulseOverlay`
  - `GetActiveHoldPulseWindowCount`
- Hold style mapping semantics and visual parameter usage remain unchanged.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
