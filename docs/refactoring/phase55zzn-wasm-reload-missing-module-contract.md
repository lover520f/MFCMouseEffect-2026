# Phase 55zzn: WASM Reload Missing-Module Contract

## Capability
- WASM

## Why
- `reload_target_missing` path is now deterministic, but reload failure with an active target still lacked deterministic contract guard.
- We need a reproducible path to validate `reload` failure semantics when target metadata exists but underlying module becomes unavailable.

## Scope
- Add deterministic fixture flow:
  - load manifest from temporary fixture,
  - remove fixture entry wasm file,
  - call reload and assert failure contract.
- Keep production behavior unchanged; test-only routes remain env-gated.

## Code Changes

### 1) Regression helper contract enrichment
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_helpers.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_checks.sh`
- Added:
  - manifest `entry` parser helper for fixture mutation.
  - extended reload-failure assertion with optional `last_load_failure_stage/code` checks.
  - deterministic fixture scenario asserting:
    - `error_code="module_load_failed"`
    - `last_load_failure_stage="load_module"`
    - `last_load_failure_code="module_load_failed"`
  - restore step to load original manifest after failure.

### 2) Manual selfcheck parity
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_common.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- Added same fixture-based reload failure contract in one-command macOS selfcheck, keeping regression/manual semantics aligned.

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_common.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- `./tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- Reload now has deterministic, script-guarded failure semantics in both major negative paths:
  - no target (`reload_target_missing`)
  - target exists but module unavailable (`module_load_failed` + stage/code)
- This reduces silent reload drift risk across future runtime and route refactors.
