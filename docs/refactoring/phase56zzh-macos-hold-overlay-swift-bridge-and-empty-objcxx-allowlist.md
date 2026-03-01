# Phase 56zzh - macOS Hold Overlay Swift Bridge and Empty ObjC++ Allowlist

## Scope
- Capability: `effects` (hold category) + ObjC++ surface gate compatibility.
- Goal:
  - migrate hold overlay leaf from ObjC++ to Swift bridge.
  - shrink macOS ObjC++ allowlist from `1` to `0`.
  - keep surface gate valid when allowlist is intentionally empty.

## Decision
- Add dedicated Swift bridge for hold overlay:
  - ring/accent layer creation
  - hold-style accent path variants (charge/lightning/hex/tech/hologram/neon/quantum/flux)
  - breathe/spin animation setup
  - runtime update (`position/scale/lineWidth/opacity`)
- Keep C++ side as orchestration only:
  - command->profile normalization
  - style/base-color selection
  - frame clamp lifecycle and bridge dispatch
- Treat empty ObjC++ allowlist as valid end-state (all macOS sources in Swift/C++).

## Code Changes
1. Added hold bridge files:
- `MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayBridge.swift`
- `MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlaySwiftBridge.h`

2. Refactored hold runtime entry:
- `MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Start.cpp`
  - removed direct AppKit/CALayer logic.
  - now uses bridge create/update APIs.
  - state now stores opaque handles instead of ObjC object pointers.
- `MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.cpp`
- `MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Internal.h`
  - switched to handle-based state fields.

3. Build wiring:
- `MFCMouseEffect/Platform/macos/CMakeLists.txt`
  - added Swift compile/object wiring for hold bridge.
  - ObjC++ allowlist is now empty (`set(MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST ... )` with no entries).

4. Surface gate compatibility:
- `tools/platform/regression/run-macos-objcxx-surface-regression.sh`
  - empty allowlist is now accepted and reported as info, while retaining wildcard/shape/path checks.

## Verification
- `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
- `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-macos-objcxx-surface-regression.sh`

All commands passed on macOS.

## Result
- Hold overlay rendering leaf is Swift-owned.
- macOS ObjC++ allowlist reduced from `1` to `0`.
- `.mm` remains `0`, and surface gate now supports the zero-allowlist end-state.

