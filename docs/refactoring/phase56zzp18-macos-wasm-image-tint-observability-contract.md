# phase56zzp18: macOS wasm image tint observability + contract gate

## Context
- Verdict: `Regression-gap risk`.
- After enabling macOS plugin-image tint bridge behavior, there was still no direct runtime observability proving image overlay requests (and tint-intended requests) were actually flowing through runtime.
- Existing contracts validated command counters and text fallback counters, but not image-overlay tint path counters.

## Changes
1. Added runtime counters in macOS overlay runtime:
- `Platform/macos/Wasm/MacosWasmOverlayRuntime.h/.cpp`
- New counters:
  - `requests`
  - `requestsWithAsset`
  - `applyTintRequests`
  - `applyTintRequestsWithAsset`

2. Wired counter recording at image render window path:
- `Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Window.cpp`
- Records per-request `(hasAsset, applyTint)` before Swift window creation.

3. Exposed counters in wasm diagnostics + schema:
- `MouseFx/Server/SettingsStateMapper.WasmDiagnostics.cpp`
- `MouseFx/Server/SettingsSchemaBuilder.CapabilitiesSections.cpp`
- Added diagnostic keys:
  - `mac_image_overlay_requests`
  - `mac_image_overlay_requests_with_asset`
  - `mac_image_overlay_apply_tint_requests`
  - `mac_image_overlay_apply_tint_requests_with_asset`

4. Extended contract and manual selfcheck assertions:
- `tools/platform/regression/lib/core_http_wasm_dispatch_assert_helpers.sh`
- `tools/platform/manual/lib/wasm_selfcheck_dispatch_assert_helpers.sh`
- macOS assertions now check:
  - image overlay request counter increases when `executed_image_commands > 0`
  - tint counter never exceeds total request counter

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
- macOS image tint path now has state-level counters and automated monotonic contracts.
- Future regressions where tint/image overlay path silently stops executing can be detected by scripts, not only visual inspection.
