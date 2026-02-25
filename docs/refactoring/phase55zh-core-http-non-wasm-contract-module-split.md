# Phase 55zh: Core HTTP Non-WASM Contract Module Split

## Why
- After `55zg`, `core_http.sh` still carried non-WASM contract details (input-permission and automation contracts).
- Further splitting keeps script responsibilities clear and reduces regression-edit risk when evolving specific contract domains.

## Scope
- No behavior change.
- Keep all existing contract assertions and gates.
- Move non-WASM contract details into dedicated modules and keep `core_http.sh` as orchestration entry.

## Code Changes

### 1) New input-permission contract module
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_contract_checks.sh`.
- Responsibilities:
  - permission simulation file writes
  - startup denied/recover + runtime revoke/regrant assertions
  - launch probe and launcher capture assertions
  - degraded notification dedup assertion

### 2) New automation contract module
- Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_checks.sh`.
- Responsibilities:
  - active-process / app-catalog contracts
  - selected-scope persistence contract
  - app-scope matching and binding-priority contract checks
  - matcher+inject and shortcut mapping probes
  - input-indicator probe checks
  - macOS-specific automation invariants (`.app` suffix, non-empty process, injector capability)

### 3) `core_http.sh` now orchestration-focused
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http.sh`:
  - source the two new non-WASM modules
  - replace inline non-WASM blocks with module calls
  - preserve existing `all` / `wasm` scope behavior
- Result:
  - `core_http.sh` line count reduced significantly (now focused on process lifecycle and top-level flow).

## Validation
- `bash -n tools/platform/regression/lib/core_http.sh`
- `bash -n tools/platform/regression/lib/core_http_input_contract_checks.sh`
- `bash -n tools/platform/regression/lib/core_http_automation_contract_checks.sh`
- `bash -n tools/platform/regression/lib/core_http_wasm_contract_checks.sh`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- API and regression behavior: unchanged.
- Change is structural only, to improve maintainability and reduce coupling.
