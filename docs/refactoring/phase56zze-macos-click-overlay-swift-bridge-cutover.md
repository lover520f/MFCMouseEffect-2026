# Phase 56zze - macOS Click Overlay Swift Bridge Cutover

## Scope
- Capability: `effects` (click category).
- Goal: migrate click overlay rendering leaf from ObjC++ to Swift bridge and continue shrinking ObjC++ allowlist.

## Decision
- Add dedicated Swift bridge for click overlay window/layer creation:
  - base ring
  - star variant
  - text variant
  - scale/fade animation
- Keep `MacosClickPulseOverlayRendererCore.Layers.cpp` as pure C++ orchestration:
  - plan->bridge call
  - registry/show lifecycle
  - delayed close scheduling
- Remove click layers file from ObjC++ allowlist.

## Code Changes
1. Added bridge files:
- `MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayBridge.swift`
- `MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlaySwiftBridge.h`

2. Refactored click layers entry:
- `MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Layers.cpp`
  - removed direct AppKit/CALayer/ObjC block path.
  - delegates click/star/text overlay drawing to Swift bridge.
  - uses `dispatch_after_f` close callback with context object.

3. Internal contract cleanup:
- `MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Internal.h`
  - removed obsolete ObjC-only helper declarations.

4. Build wiring:
- `MFCMouseEffect/Platform/macos/CMakeLists.txt`
  - added Swift compile/object wiring for click bridge.
  - removed `MacosClickPulseOverlayRendererCore.Layers.cpp` from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`.

## Verification
- `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
- `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-macos-objcxx-surface-regression.sh`

All commands passed on macOS.

## Result
- Click overlay rendering leaf is Swift-owned.
- ObjC++ allowlist reduced from `4` to `3`.

