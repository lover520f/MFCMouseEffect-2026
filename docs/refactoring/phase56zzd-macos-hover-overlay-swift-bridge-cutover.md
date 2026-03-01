# Phase 56zzd - macOS Hover Overlay Swift Bridge Cutover

## Scope
- Capability: `effects` (hover category).
- Goal: migrate hover overlay rendering leaf from ObjC++ to Swift bridge and continue shrinking ObjC++ allowlist.

## Decision
- Add a dedicated Swift bridge to create/configure hover overlay window + ring/tubes layers + animations.
- Keep `MacosHoverPulseOverlayRendererCore.Layers.cpp` as pure C++ orchestration:
  - active window handle lifecycle
  - plan->bridge call
  - show/close/count wrappers
- Remove hover layers file from ObjC++ allowlist.

## Code Changes
1. Added bridge files:
- `MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayBridge.swift`
- `MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlaySwiftBridge.h`

2. Refactored hover layers entry:
- `MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Layers.cpp`
  - removed AppKit/CALayer ObjC path.
  - delegates overlay rendering to Swift bridge.
  - keeps C++ active-window lifecycle with shared `ReleaseOverlayWindow`.

3. Internal contract cleanup:
- `MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Internal.h`
  - removed obsolete ObjC-only declarations (`ConfigureHoverRingLayer`, `AddHoverExtraLayersAndAnimations`).
  - kept plan contract as C++ compatible `CGRect`.
- `MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Plan.cpp`
  - switched `rawFrame` to `CGRect`.

4. Build wiring:
- `MFCMouseEffect/Platform/macos/CMakeLists.txt`
  - added Swift compile/object wiring for hover bridge.
  - removed `MacosHoverPulseOverlayRendererCore.Layers.cpp` from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`.

## Verification
- `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
- `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-macos-objcxx-surface-regression.sh`

All commands passed on macOS.

## Result
- Hover overlay rendering leaf is Swift-owned.
- ObjC++ allowlist reduced from `5` to `4`.
