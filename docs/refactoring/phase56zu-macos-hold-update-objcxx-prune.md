# Phase 56zu - macOS Hold Update ObjC++ Prune

## Scope
- Capability: `effects` (hold category).
- Goal: reduce ObjC++ surface by moving hold update leaf logic into an existing ObjC++ file.

## Decision
- Keep `MacosHoldPulseOverlayRendererCore.Update.cpp` as command-wrapper only.
- Move AppKit-dependent update and active-window-count logic to `MacosHoldPulseOverlayRendererCore.Start.cpp` (already ObjC++).
- Remove `MacosHoldPulseOverlayRendererCore.Update.cpp` from ObjC++ allowlist.

## Code Changes
1. `MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Start.cpp`
- Added:
  - `UpdateHoldPulseOverlayOnMain(const ScreenPoint&, uint32_t)`
  - `GetActiveHoldPulseWindowCountOnMain()`
- The moved code keeps original frame update, scale, line-width, and opacity progression behavior.

2. `MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Update.cpp`
- Reduced to command-wrapper only:
  - `UpdateHoldPulseOverlayOnMain(const HoldEffectUpdateCommand&)`
- Removed direct AppKit/CALayer manipulation from this file.

3. `MFCMouseEffect/Platform/macos/CMakeLists.txt`
- Removed `MacosHoldPulseOverlayRendererCore.Update.cpp` from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`.

## Verification
- `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
- `cmake --build /tmp/mfx-platform-macos-core-automation-build --target mfx_entry_posix_host -j8`
- `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-macos-objcxx-surface-regression.sh`

All commands passed on macOS.

## Result
- ObjC++ allowlist reduced from `14` to `13`.
- Hold update path keeps behavior while shrinking ObjC++ compile area.
