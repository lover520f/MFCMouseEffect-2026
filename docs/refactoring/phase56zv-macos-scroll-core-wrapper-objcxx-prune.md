# Phase 56zv - macOS Scroll Core Wrapper ObjC++ Prune

## Scope
- Capability: `effects` (scroll category).
- Goal: shrink ObjC++ surface by moving AppKit rendering leaf code from `Core.cpp` into `Core.Layers.cpp`.

## Decision
- Keep `MacosScrollPulseOverlayRendererCore.cpp` as pure wrapper/orchestration.
- Move command-path rendering and layer creation (ObjC/CALayer dependent) into `MacosScrollPulseOverlayRendererCore.Layers.cpp`.
- Remove `MacosScrollPulseOverlayRendererCore.cpp` from ObjC++ allowlist.

## Code Changes
1. `MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.cpp`
- Removed AppKit/CALayer and dispatch code.
- Kept only profile-to-command wrapper overload:
  - `ShowScrollPulseOverlayOnMain(overlayPt, horizontal, delta, effectType, themeName, profile)`

2. `MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Layers.cpp`
- Added command-path renderer:
  - `ShowScrollPulseOverlayOnMain(const ScrollEffectRenderCommand&, const std::string&)`
- Added local ObjC++ layer helpers:
  - `CreateBodyLayer(...)`
  - `CreateArrowLayer(...)`
- Added required includes for window registry and dispatch timing close path.

3. `MFCMouseEffect/Platform/macos/CMakeLists.txt`
- Removed `MacosScrollPulseOverlayRendererCore.cpp` from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`.

## Verification
- `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
- `cmake --build /tmp/mfx-platform-macos-core-automation-build --target mfx_entry_posix_host -j8`
- `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-macos-objcxx-surface-regression.sh`

All commands passed on macOS.

## Result
- ObjC++ allowlist reduced from `13` to `12`.
- Scroll `Core.cpp` is now C++-only; ObjC++ remains in rendering leaf (`Core.Layers.cpp`).
