# phase56zzp17: macOS wasm image applyTint bridge parity

## Context
- Verdict: `Bug/regression`.
- Windows `spawn_image` plugin renderer already applies tint when `applyTint=true`.
- macOS bridge exposed `applyTint` in request construction but did not consume it in Swift image rendering; only ring tint changed, plugin image pixels stayed untinted.

## Changes
1. Extended Swift bridge ABI with tint flag:
- `Platform/macos/Wasm/MacosWasmImageOverlaySwiftBridge.h`
- `mfx_macos_wasm_image_overlay_create_v1(..., tintArgb, applyTint, alphaScale, ...)`

2. Wired C++ call-site to pass request tint intent:
- `Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Window.cpp`
- passes `req.applyTint ? 1 : 0` to bridge.

3. Implemented Swift image tinting path:
- `Platform/macos/Wasm/MacosWasmImageOverlayBridge.swift`
- Added helper `mfxCreateTintedImage(...)` and apply it only when:
  - image asset exists
  - `applyTint == true`
- Non-tint path remains unchanged.

## Validation
1. Core WASM contract:
```bash
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto --check-scope wasm --build-dir /tmp/mfx-platform-macos-core-automation-build
```

2. macOS wasm selfcheck:
```bash
./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build --build-dir /tmp/mfx-platform-macos-core-automation-build
```

## Result
- macOS plugin-image tint behavior is now consistent with Windows semantic intent (`applyTint` is effective on image content, not only overlay ring cues).
