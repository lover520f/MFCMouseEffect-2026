# Phase56zc: macOS Effects Swift Bridge Path Dedup

## Background
- After LineTrail Swift cutover, parts of the effects stack still kept duplicated helper paths:
  - LineTrail Swift bridge locally reimplemented overlay window/frame/scale operations.
  - Text fallback kept private main-thread dispatch helpers equivalent to shared overlay support.
- This phase removes those duplicates so bridged capabilities stay on one execution path.

## What Changed
1. LineTrail bridge now reuses shared overlay bridge symbols
- Updated:
  - `MFCMouseEffect/Platform/macos/Effects/MacosLineTrailOverlayBridge.swift`
- Changes:
  - Added `_silgen_name` bindings to shared overlay bridge C ABI:
    - create/release/show window
    - resolve screen frame
    - apply content scale
  - Removed local `NSScreen` frame/scale resolution and local window lifecycle implementation.
  - LineTrail window lifecycle is now owned by shared overlay bridge functions.

2. Text fallback removed duplicate main-thread helper path
- Updated:
  - `MFCMouseEffect/Platform/macos/Effects/MacosTextEffectFallback.cpp`
- Changes:
  - Removed local `RunOnMainThreadSync/Async` helper implementations.
  - Reused `macos_overlay_support::RunOnMainThreadSync/Async`.
  - Removed no-longer-needed local `pthread` include.

## Validation
```bash
cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8
cmake --build /tmp/mfx-platform-macos-core-automation-build --target mfx_entry_posix_host -j8
./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto
./tools/platform/regression/run-macos-objcxx-surface-regression.sh
```

## Notes
- No schema/API/user-facing behavior change in this phase.
- Goal is path convergence and removal of duplicate runtime helper ownership.
