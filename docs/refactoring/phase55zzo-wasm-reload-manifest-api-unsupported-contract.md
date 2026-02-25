# Phase 55zzo: WASM Reload Manifest-API-Unsupported Contract

## Capability
- WASM

## Why
- Reload contracts already covered:
  - no target (`reload_target_missing`)
  - target exists but module missing (`module_load_failed`)
- Missing piece: target exists but manifest becomes incompatible (`api_version` changed) after load.

## Scope
- Add deterministic fixture mutation path to validate reload failure semantics for manifest API incompatibility.
- Keep runtime/route implementation unchanged; this slice is contract hardening for future regression safety.

## Code Changes

### 1) Regression contract expansion
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_checks.sh`
- Added deterministic flow:
  1. load fixture manifest successfully,
  2. mutate fixture manifest to `api_version=2` (unsupported),
  3. call reload and assert:
     - `error_code="manifest_api_unsupported"`
     - `last_load_failure_stage="manifest_api_version"`
     - `last_load_failure_code="manifest_api_unsupported"`
  4. restore by reloading original manifest.

### 2) Manual selfcheck parity
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- Added the same fixture mutation + reload-failure assertions to the one-command macOS selfcheck flow.

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- `./tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- Reload now has deterministic coverage across three major negative modes:
  - missing target,
  - missing module,
  - unsupported manifest API version.
- This materially reduces WASM reload behavior drift risk during future refactors.
