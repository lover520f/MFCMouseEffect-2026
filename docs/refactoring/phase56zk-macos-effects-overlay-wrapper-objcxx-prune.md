# Phase 56zk - macOS Effects Overlay Wrapper ObjC++ Surface Prune

## What Changed
- Added C++ callback-based main-thread bridge entrypoints in `MacosOverlayRenderSupport`:
  - `RunOnMainThreadSync(MainThreadCallback, void*)`
  - `RunOnMainThreadAsync(MainThreadCallback, void*)`
- Refactored the 5 effect overlay wrapper files to use callback context trampolines instead of Objective-C block literals:
  - `MacosClickPulseOverlayRenderer.cpp`
  - `MacosTrailPulseOverlayRenderer.cpp`
  - `MacosScrollPulseOverlayRenderer.cpp`
  - `MacosHoverPulseOverlayRenderer.cpp`
  - `MacosHoldPulseOverlayRenderer.cpp`
- Removed those 5 wrapper files from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST` in `Platform/macos/CMakeLists.txt`.

## Why
- These wrappers are orchestration-only entry files and do not need Objective-C syntax after callback bridge support exists.
- Keeping them in ObjC++ compile mode inflated residual ObjC++ surface and obscured true migration progress.

## Validation
- Build:
  - `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
  - `cmake --build /tmp/mfx-platform-macos-core-automation-build --target mfx_entry_posix_host -j8`
- Regression:
  - `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
  - `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
  - `./tools/platform/regression/run-macos-objcxx-surface-regression.sh`

## Result
- All build and regression gates passed.
- ObjC++ allowlist shrank without user-visible behavior changes.
