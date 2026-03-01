# Phase 56zza - macOS WASM Image Window Lifecycle Unify

## Scope
- Capability: `wasm` (image overlay runtime path).
- Goal: unify window lifecycle calls through shared overlay-support boundary and remove local close-path duplication.

## Decision
- Keep image overlay render/layer composition in current ObjC++ leaf (`MacosWasmImageOverlayRendererCore.Window.cpp`).
- Route window create/show/release through `macos_overlay_support` APIs.
- Replace delayed close Objective-C block with C++ `dispatch_after_f` callback context.

## Code Changes
1. `MFCMouseEffect/Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Window.cpp`
- Added `MacosOverlayRenderSupport.h` dependency.
- Switched window lifecycle:
  - create: `macos_overlay_support::CreateOverlayWindow(...)`
  - show: `macos_overlay_support::ShowOverlayWindow(...)`
  - release: `macos_overlay_support::ReleaseOverlayWindow(...)`
- Added C++ delayed-close callback context:
  - `WasmImageOverlayCloseContext`
  - `CloseWasmImageOverlayAfterDelay(void*)`
- Replaced `dispatch_after` block with `dispatch_after_f`.

## Verification
- `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
- `./tools/platform/regression/run-posix-wasm-regression-suite.sh --platform auto`

All commands passed on macOS.

## Result
- WASM image overlay window lifecycle now shares one runtime boundary with effects/overlay modules.
- Reduced local direct AppKit window management duplication and prepared follow-up Swift migration.
