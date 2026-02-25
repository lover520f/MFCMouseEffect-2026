# Phase 55zzp: WASM Fixture Helper Consolidation

## Capability
- WASM

## Why
- Regression and manual selfcheck scripts duplicated fixture workflow logic:
  - clone plugin directory,
  - parse manifest entry,
  - remove entry wasm,
  - mutate manifest api version.
- Duplication increases drift risk between regression gate and manual validation.

## Scope
- Extract shared fixture utilities into one helper module.
- Rewire both regression and manual wasm flows to consume the same helper API.
- Keep runtime behavior and assertions unchanged.

## Code Changes

### 1) Shared fixture helper module
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/wasm_fixture_helpers.sh`
- Provided reusable primitives:
  - fixture manifest copy
  - manifest entry parse/path resolve
  - entry-file existence check/removal
  - manifest rewrite with target api version

### 2) Regression flow rewired
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_checks.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_helpers.sh`
- Change:
  - removed local fixture plumbing and switched to shared helper calls.
  - removed now-unused local manifest-entry parser.

### 3) Manual selfcheck flow rewired
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_common.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- Change:
  - manual selfcheck now uses the same fixture helper functions as regression.
  - removed local duplicate manifest-entry parser and inline fixture orchestration.

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/wasm_fixture_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_common.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- `./tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No runtime/API contract changes.
- Reduced regression/manual drift by converging fixture operations onto one shared implementation.
