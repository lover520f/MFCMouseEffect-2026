# Phase 56zzc - macOS Overlay Render Support ObjC++ Prune

## Scope
- Capability: `effects` (shared macOS overlay support boundary).
- Goal: remove ObjC++ compile dependency from `MacosOverlayRenderSupport.cpp` while preserving render behavior.

## Decision
- Keep `MacosOverlayRenderSupport` as pure C++ bridge/orchestration utility.
- Move shared scale/fade animation-group construction into effect leaf renderers (`click/trail/scroll` layers).
- Keep ObjC++ only in true AppKit/CALayer leaf files.

## Code Changes
1. `MFCMouseEffect/Platform/macos/Effects/MacosOverlayRenderSupport.cpp`
- Removed Objective-C block path (`dispatch_block_t` sync/async overloads).
- Main-thread callbacks now use pure C++ `dispatch_sync_f/dispatch_async_f`.
- Removed `CreateScaleFadeAnimationGroup` implementation from shared support.
- `ApplyOverlayContentScale` now uses stable handle signature (`void*`) to avoid C++/ObjC++ symbol mismatch.

2. `MFCMouseEffect/Platform/macos/Effects/MacosOverlayRenderSupport.h`
- Removed ObjC-only main-thread block overload declarations.
- Removed `CreateScaleFadeAnimationGroup` declaration.
- Updated `ApplyOverlayContentScale` signature to `void* contentHandle`.

3. Effect leaf renderers:
- `MFCMouseEffect/Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Layers.cpp`
- `MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Layers.cpp`
- `MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Layers.cpp`
- Added local `CreateScaleFadeAnimationGroup(...)` in each leaf and switched callsites to local helper.

4. Build wiring:
- `MFCMouseEffect/Platform/macos/CMakeLists.txt`
- Removed `MacosOverlayRenderSupport.cpp` from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`.

## Verification
- `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
- `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-macos-objcxx-surface-regression.sh`

All commands passed on macOS.

## Result
- `MacosOverlayRenderSupport.cpp` no longer requires ObjC++ compile mode.
- ObjC++ allowlist reduced from `6` to `5`.
