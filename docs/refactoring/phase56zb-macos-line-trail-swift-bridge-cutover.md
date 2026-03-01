# Phase56zb: macOS Line Trail Swift Bridge Cutover

## Background
- `MacosLineTrailOverlay.cpp` previously carried full AppKit/Quartz runtime logic under ObjC++ mode.
- Current migration target is to keep C++ on capability orchestration and move macOS UI/runtime ownership into Swift bridges.

## What Changed
1. Line trail Swift bridge added
- Added:
  - `MFCMouseEffect/Platform/macos/Effects/MacosLineTrailOverlayBridge.swift`
  - `MFCMouseEffect/Platform/macos/Effects/MacosLineTrailOverlaySwiftBridge.h`
- New C ABI surface:
  - `mfx_macos_line_trail_create_v1`
  - `mfx_macos_line_trail_update_v1`
  - `mfx_macos_line_trail_reset_v1`
  - `mfx_macos_line_trail_release_v1`

2. Line trail C++ entry converted to thin bridge wrapper
- Updated:
  - `MFCMouseEffect/Platform/macos/Effects/MacosLineTrailOverlay.cpp`
- Behavior:
  - C++ keeps public API unchanged (`UpdateLineTrail` / `ResetLineTrail`).
  - C++ only performs coordinate conversion (`ScreenToOverlayPoint`) and forwards normalized args to Swift bridge.
  - Window lifecycle, path rendering, timer/fade loop live in Swift.

3. Build wiring and ObjC++ surface pruning
- Updated:
  - `MFCMouseEffect/Platform/macos/CMakeLists.txt`
- Changes:
  - Added explicit Swift object build for `MacosLineTrailOverlayBridge.swift`.
  - Linked Swift object into `mfx_shell_macos`.
  - Removed `MacosLineTrailOverlay.cpp` from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`.

## Validation
```bash
cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8
cmake --build /tmp/mfx-platform-macos-core-automation-build --target mfx_entry_posix_host -j8
./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto
./tools/platform/regression/run-macos-objcxx-surface-regression.sh
```

## Notes
- No public API/schema change.
- This phase is structural migration only; runtime behavior should remain equivalent.
