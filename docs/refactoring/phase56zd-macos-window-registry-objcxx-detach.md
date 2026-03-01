# Phase56zd: macOS Window Registry ObjC++ Detach

## Background
- `MacosClick/Trail/ScrollPulseWindowRegistry.cpp` modules are ownership registries for overlay window handles.
- They had no ObjC behavior logic but still pulled ObjC++ compile mode via `MacosOverlayRenderSupport.h`.

## What Changed
1. Window registry modules now use C ABI bridge directly
- Updated:
  - `MFCMouseEffect/Platform/macos/Effects/MacosClickPulseWindowRegistry.cpp`
  - `MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseWindowRegistry.cpp`
  - `MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseWindowRegistry.cpp`
- Changes:
  - Include switched from `MacosOverlayRenderSupport.h` to `MacosOverlayRenderSupportSwiftBridge.h`.
  - Close-path switched to direct C ABI call:
    - `mfx_macos_overlay_release_window_v1(handle)`

2. ObjC++ allowlist reduced
- Updated:
  - `MFCMouseEffect/Platform/macos/CMakeLists.txt`
- Removed these files from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`:
  - `MacosClickPulseWindowRegistry.cpp`
  - `MacosTrailPulseWindowRegistry.cpp`
  - `MacosScrollPulseWindowRegistry.cpp`

## Validation
```bash
cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8
cmake --build /tmp/mfx-platform-macos-core-automation-build --target mfx_entry_posix_host -j8
./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto
./tools/platform/regression/run-macos-objcxx-surface-regression.sh
```

## Notes
- No runtime behavior or API contract change.
- This is a structural shrink of ObjC++ compile surface.
