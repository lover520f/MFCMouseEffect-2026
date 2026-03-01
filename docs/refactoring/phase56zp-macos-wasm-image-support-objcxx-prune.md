# Phase 56zp - macOS WASM Image Support ObjC++ Surface Prune

## What Changed
- Converted `MacosWasmImageOverlayRendererSupport` path helper from Objective-C type contract to pure C++:
  - Replaced `NsPathFromWide(...) -> NSString*` with `Utf8PathFromWide(...) -> std::string`.
  - Removed `Foundation` dependency from `MacosWasmImageOverlayRendererSupport.cpp`.
- Updated wasm image renderer window path (`MacosWasmImageOverlayRendererCore.Window.cpp`) to create `NSString` locally from UTF-8 where AppKit runtime is already required.
- Removed `MacosWasmImageOverlayRendererSupport.cpp` from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`.

## Why
- Support module is utility math/path logic and should not carry Objective-C object contracts.
- Keeping Objective-C conversions in renderer leaf path preserves behavior while shrinking ObjC++ area.

## Validation
- Syntax probe:
  - `MacosWasmImageOverlayRendererSupport.cpp` passes `-x c++ -fsyntax-only`.
- Build:
  - `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
  - `cmake --build /tmp/mfx-platform-macos-core-automation-build --target mfx_entry_posix_host -j8`
- Regression:
  - `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
  - `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
  - `./tools/platform/regression/run-macos-objcxx-surface-regression.sh`

## Result
- All gates passed.
- ObjC++ allowlist reduced by one additional file with unchanged runtime behavior.
