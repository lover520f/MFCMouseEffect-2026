# phase56zzp14: wasm text-config contract route and gate hardening

## Context
- Verdict: `Bug/regression risk`.
- `spawn_text` compute semantics are shared in Core (`BuildSpawnTextConfig`), but contract coverage previously focused on dispatch counters and visual fallback probes.
- Missing gate: no HTTP-contract assertion for text command parameter resolution (`life -> duration`, kinematics -> float distance, scale -> font size, color override).

## Changes
1. Added test-only route:
- `POST /api/wasm/test-resolve-text-config`
- File: `MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestWasmInputApiRoutes.cpp`
- Output includes:
  - `resolved_duration_ms`
  - `resolved_float_distance_px`
  - `resolved_font_size_px_milli`
  - `resolved_color_rgba_hex`
  - plus base/input diagnostics for deterministic assertions.

2. Route implementation uses shared Core resolvers:
- `BuildSpawnTextConfig(...)` from `MouseFx/Core/Wasm/WasmTextCommandConfig.h`
- text/color resolution from `WasmRenderResourceResolver`
- Added test-only base-config override inputs (`base_duration_ms`, `base_float_distance_px`, `base_font_size_px`) so scripts can avoid dependence on user local config.

3. Regression + selfcheck gate expansion:
- Added HTTP helper entry in:
  - `tools/platform/regression/lib/core_http_wasm_http_helpers.sh`
  - `tools/platform/manual/lib/wasm_selfcheck_http_helpers.sh`
- Added assertions in:
  - `tools/platform/regression/lib/core_http_wasm_contract_dispatch_checks.sh`
  - `tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- Three asserted scenarios:
  - motion-driven distance (`life=1200`, `vy=-300`, `ay=100`, `scale=1.0`)
  - clamp path (`life=1`, `scale=100`)
  - negative-scale no-op on font size (`scale=-1`)

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
- `spawn_text` parameter resolution now has route-level, deterministic, script-gated contracts.
- macOS and Windows command semantics drift risk is reduced because regression now locks the shared compute behavior directly.
