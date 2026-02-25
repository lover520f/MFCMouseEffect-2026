# Phase 55x: Core HTTP WASM Helper Module Split

## Issue Classification
- Verdict: `Process debt`.
- Problem: `core_http.sh` kept growing and embedded WASM load-manifest request/assert helpers directly, increasing script coupling and reducing maintainability.

## Goal
1. Move WASM-specific helper routines into a dedicated regression-lib module.
2. Keep existing core HTTP contract behavior unchanged.
3. Preserve script readability and future extension space.

## Implementation
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_helpers.sh`
  - contains:
    - `_mfx_core_http_wasm_load_manifest_http_code`
    - `_mfx_core_http_assert_wasm_load_manifest_ok`
    - `_mfx_core_http_assert_wasm_load_manifest_failure`
    - local JSON escape helper for manifest payloads.
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http.sh`
  - sources new helper module and removes duplicated inline WASM helper implementations.

## Validation
- `bash -n tools/platform/regression/lib/core_http.sh tools/platform/regression/lib/core_http_wasm_helpers.sh`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Closure
- WASM load-manifest helper logic is now isolated in a dedicated regression module, reducing coupling in `core_http.sh` while preserving all existing contract checks.
