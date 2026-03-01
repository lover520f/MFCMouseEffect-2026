# Phase 56zl - macOS Text Fallback ObjC++ Surface Prune

## What Changed
- Refactored `Platform/macos/Effects/MacosTextEffectFallback.cpp` from Objective-C block scheduling to pure C++ callback scheduling:
  - `RunOnMainThreadSync/Async` now use callback-style invocations.
  - Animation tick loop now uses `dispatch_after_f` with explicit `TextAnimationState` context (no block literal capture path).
- Removed `MacosTextEffectFallback.cpp` from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST` in `Platform/macos/CMakeLists.txt`.

## Why
- `MacosTextEffectFallback.cpp` is now a pure C++ bridge/orchestration unit over Swift bridge C ABI and does not require Objective-C syntax.
- Keeping it in ObjC++ mode would over-report remaining Objective-C surface and slow down planned Swift-first migration closure.

## Validation
- Build:
  - `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
  - `cmake --build /tmp/mfx-platform-macos-core-automation-build --target mfx_entry_posix_host -j8`
- Regression:
  - `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
  - `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
  - `./tools/platform/regression/run-macos-objcxx-surface-regression.sh`

## Result
- All gates passed.
- macOS ObjC++ allowlist shrank by one more file without behavior regression.
