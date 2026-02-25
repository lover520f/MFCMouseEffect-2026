# Phase 55zg: Core HTTP WASM Contract Module Split

## Why
- `core_http.sh` accumulated orchestration + non-WASM contracts + WASM contracts in one file.
- Phase 55 M2 iteration focuses heavily on WASM paths, so keeping WASM contract logic isolated reduces change risk and review cost.

## Scope
- No behavior contract change.
- Split WASM contract execution block out of `core_http.sh` into dedicated module.
- Keep existing `all` and `wasm` scope semantics unchanged.

## Code Changes

### 1) New WASM contract module
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_checks.sh`.
- Introduced:
  - `_mfx_core_http_detect_wasm_manifest_path`
  - `_mfx_core_http_run_wasm_contract_checks`
- Module now owns:
  - WASM catalog/import dialog probe contracts
  - import-selected / export-all / load-manifest / enable / dispatch contracts
  - transfer/load failure semantics assertions
  - macOS WASM-specific invariants (`runtime_backend`, `render_supported`, render-visible assertions)

### 2) `core_http.sh` reduced to orchestration
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http.sh`:
  - source new module file
  - replace inlined WASM contract block with module call
  - keep non-WASM automation/input-capture and platform checks in the main script
- Result:
  - clearer boundary between WASM and non-WASM contract evolution
  - lower risk when changing WASM-only checks

## Validation
- `bash -n tools/platform/regression/lib/core_http.sh`
- `bash -n tools/platform/regression/lib/core_http_wasm_contract_checks.sh`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- API behavior: unchanged.
- Regression entrypoints: unchanged (plus existing WASM-focused gate introduced in previous slice remains valid).
