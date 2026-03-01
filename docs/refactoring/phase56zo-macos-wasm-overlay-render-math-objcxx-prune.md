# Phase 56zo - macOS WASM Overlay Render Math ObjC++ Surface Prune

## What Changed
- `MacosWasmOverlayRenderMath` is now pure math-only:
  - Removed `ColorFromArgb(...)` from `MacosWasmOverlayRenderMath.h/.cpp`.
  - Removed `AppKit` dependency from the module.
- Moved ARGB -> `NSColor` conversion to wasm renderer call-sites where Objective-C runtime is already required:
  - `MacosWasmImageOverlayRendererCore.Window.cpp`
  - `MacosWasmTextOverlay.Style.cpp`
- Removed `MacosWasmOverlayRenderMath.cpp` from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`.

## Why
- `OverlayRenderMath` belongs to shared numeric clamp/scale logic and should not carry Objective-C surface.
- Keeping Objective-C color conversion in renderer leaf nodes keeps boundaries clear and makes ObjC++ shrink measurable.

## Validation
- Syntax probe:
  - `MacosWasmOverlayRenderMath.cpp` passes `-x c++ -fsyntax-only`.
- Build:
  - `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
  - `cmake --build /tmp/mfx-platform-macos-core-automation-build --target mfx_entry_posix_host -j8`
- Regression:
  - `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
  - `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
  - `./tools/platform/regression/run-macos-objcxx-surface-regression.sh`

## Result
- All gates passed.
- ObjC++ allowlist reduced by one additional file with no behavior change.
