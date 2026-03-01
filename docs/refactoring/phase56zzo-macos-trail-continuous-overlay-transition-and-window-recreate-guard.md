# Phase 56zzo: macOS Trail Continuous Overlay Transition Guard

## Scope
- Capability: `effects` (macOS trail path only)
- Files:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseEffect.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseEffect.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosLineTrailOverlayBridge.swift`

## Problem Classification
- Classification: `Bug/regression`
- Evidence:
  - `line/streamer` continuous path kept line overlay state across type transitions (`line/streamer -> none/electric/...`), causing stale residual line behavior.
  - Line-trail Swift overlay kept historical points when overlay window was recreated (screen-frame switch), which can produce cross-screen connector artifacts.

## Changes
1. Continuous trail lifecycle guard in `MacosTrailPulseEffect`:
   - Added `continuousTrailActive_` runtime flag.
   - On `trail=none`, immediately resets line-trail overlay if active.
   - On transition from continuous types (`line/streamer`) to non-continuous types, immediately resets line-trail overlay.
   - On teleport-drop in continuous path, also resets line-trail overlay to avoid stale connectors.
2. Window recreate stale-point guard in `MacosLineTrailOverlayBridge.swift`:
   - When line-trail overlay window is recreated (screen/frame changed), historical points are cleared before new window use.
   - This prevents old-screen points from being connected in the new-screen coordinate space.
3. Runtime observability for continuous line-trail:
   - Added line-trail runtime snapshot bridge (`active`, `point_count`) from Swift bridge to C++ wrapper.
   - `/api/state.effects_runtime` now includes:
     - `line_trail_active`
     - `line_trail_point_count`
   - `/api/effects/test-overlay-windows` now also returns line-trail before/after snapshots for scripted diagnostics.

## Validation
Executed on macOS host:
```bash
cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8
./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto
./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto
./tools/platform/regression/run-macos-objcxx-surface-regression.sh
```

Result: all passed.

## Risk Notes
- Behavior change is scoped to macOS trail runtime only.
- Windows path is untouched.
- Continuous trail reset now happens deterministically on type transition / teleport-drop, which may shorten residual fade tail in those transitions (intentional for correctness).
