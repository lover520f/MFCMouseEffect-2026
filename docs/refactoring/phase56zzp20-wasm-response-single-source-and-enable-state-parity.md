# phase56zzp20: wasm response single-source + enable/state parity gates

## Context
- Verdict: `Regression-gap risk`.
- `WebSettingsServer.WasmRouteResponseUtils` assembled a separate wasm response field set, partially overlapping with `/api/state.wasm` (`BuildWasmState`) but not guaranteed to stay synchronized.
- This created long-term drift risk between:
  - `/api/state` `wasm` diagnostics
  - `/api/wasm/enable|disable|policy|reload` responses

## Changes
1. Unified response assembly:
- `MouseFx/Server/WebSettingsServer.WasmRouteResponseUtils.cpp`
- `BuildWasmResponse(...)` now merges shared `BuildWasmState(cfg, controller)` output into route response payload, replacing duplicated per-field wiring.

2. Added parity assertions in regression and manual selfcheck:
- `tools/platform/regression/lib/core_http_wasm_contract_dispatch_checks.sh`
- `tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- New checks compare `wasm-enable` response vs `/api/state` snapshot for:
  - `invoke_supported`
  - `render_supported`
  - (macOS) `overlay_max_inflight`
  - (macOS) `overlay_min_image_interval_ms`
  - (macOS) `overlay_min_text_interval_ms`

3. Added bool parse helper support:
- `tools/platform/regression/lib/core_http_wasm_parse_helpers.sh`
- `tools/platform/manual/lib/wasm_selfcheck_parse_helpers.sh`

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
- WASM route responses and `/api/state.wasm` now share one diagnostics source.
- Enable-route payload parity is now explicitly gated in both automated and manual selfcheck flows.
