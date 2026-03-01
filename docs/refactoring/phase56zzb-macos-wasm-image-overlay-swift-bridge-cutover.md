# Phase 56zzb - macOS WASM Image Overlay Swift Bridge Cutover

## Scope
- Capability: `wasm` (image overlay render path).
- Goal: migrate macOS WASM image overlay window/layer rendering leaf to Swift bridge and shrink ObjC++ allowlist.

## Decision
- Add dedicated Swift bridge for WASM image overlay window creation + composition + animation.
- Keep `MacosWasmImageOverlayRendererCore.Window.cpp` as pure C++ orchestration:
  - plan unpack
  - motion delta compute
  - bridge invocation
  - overlay slot lifecycle and delayed close scheduling
- Remove `MacosWasmImageOverlayRendererCore.Window.cpp` from ObjC++ allowlist.

## Code Changes
1. Added bridge files:
- `MFCMouseEffect/Platform/macos/Wasm/MacosWasmImageOverlayBridge.swift`
- `MFCMouseEffect/Platform/macos/Wasm/MacosWasmImageOverlaySwiftBridge.h`

2. Refactored runtime leaf:
- `MFCMouseEffect/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Window.cpp`
  - removed AppKit/QuartzCore ObjC++ rendering code.
  - delegates window creation/render animation to Swift bridge.
  - retains C++ delayed close callback (`dispatch_after_f`) + shared release path.

3. Build wiring updates:
- `MFCMouseEffect/Platform/macos/CMakeLists.txt`
  - added Swift compile step/object for `MacosWasmImageOverlayBridge.swift`.
  - linked Swift object into `mfx_shell_macos`.
  - removed `MacosWasmImageOverlayRendererCore.Window.cpp` from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`.

## Verification
- `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
- `./tools/platform/regression/run-posix-wasm-regression-suite.sh --platform auto`

All commands passed on macOS.

## Result
- WASM image overlay render leaf is Swift-owned.
- ObjC++ allowlist reduced from `7` to `6`.
