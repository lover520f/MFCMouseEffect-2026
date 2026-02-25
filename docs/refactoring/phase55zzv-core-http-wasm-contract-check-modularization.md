# Phase 55zzv: Core HTTP WASM Contract Check Modularization

## Capability
- WASM
- Regression infrastructure

## Why
- `core_http_wasm_contract_checks.sh` had grown into a long monolithic workflow combining catalog, path, transfer, runtime, fixture, dispatch, and platform checks.
- Splitting by scenario keeps each contract slice focused and reduces change risk when adding new checks.

## Scope
- Keep existing entrypoints and function names unchanged.
- Split contract checks into scenario-specific modules and keep `core_http_wasm_contract_checks.sh` as the orchestrator.

## Code Changes
1. Added scenario modules:
   - catalog + import-dialog probe:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_catalog_checks.sh`
   - path semantics:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_path_checks.sh`
   - runtime reload/reset:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_runtime_checks.sh`
   - transfer (import/export) + invalid cases:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_transfer_checks.sh`
   - fixture-based reload failures:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_fixture_checks.sh`
   - dispatch enable/invoke/diagnostics:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_dispatch_checks.sh`
   - platform-specific assertions:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_platform_checks.sh`
2. Orchestrator now delegates to these modules:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_checks.sh`

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_catalog_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_path_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_runtime_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_transfer_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_fixture_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_dispatch_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_platform_checks.sh`
- `./tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No API/schema/runtime behavior change.
- Script internal architecture only.
