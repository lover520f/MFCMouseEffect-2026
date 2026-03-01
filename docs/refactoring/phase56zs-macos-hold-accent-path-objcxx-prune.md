# Phase 56zs - macOS Hold Accent Path ObjC++ Prune

## Scope
- Capability: `effects` (hold accent rendering path).
- Goal: remove another non-essential ObjC++ compile unit while keeping hold visual behavior unchanged.

## Decision
- `MacosHoldPulseOverlayStyle.Accent.cpp` should only compute geometry/style hints (path, stroke width, fill mode).
- AppKit/CALayer property application remains in the ObjC++ leaf (`MacosHoldPulseOverlayRendererCore.Start.cpp`).
- Remove accent module from ObjC++ allowlist after conversion.

## Code Changes
1. `MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayStyle.Internal.h`
- Replaced layer-oriented helper contract with compute contract:
  - `BuildSpecialHoldAccentPath(...)`

2. `MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayStyle.Accent.cpp`
- Removed `NSColor`/`CAShapeLayer` coupling.
- Added pure CoreGraphics plan builder returning:
  - `CGPathRef`
  - line width
  - whether to fill with base color

3. `MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Start.cpp`
- Consumes `BuildSpecialHoldAccentPath(...)`.
- Applies fill/stroke colors and layer fields locally in ObjC++ leaf code.

4. `MFCMouseEffect/Platform/macos/CMakeLists.txt`
- Removed `MacosHoldPulseOverlayStyle.Accent.cpp` from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`.

## Verification
- `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
- `cmake --build /tmp/mfx-platform-macos-core-automation-build --target mfx_entry_posix_host -j8`
- `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-macos-objcxx-surface-regression.sh`

All commands passed on macOS.

## Result
- ObjC++ allowlist reduced from `16` to `15`.
- Hold accent compute path is now C++-only; ObjC++ remains only at rendering leafs.
