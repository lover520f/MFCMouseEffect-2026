# Phase 55zf: WASM Focused Contract Gate and Selfcheck Expansion

## Background
- Phase 55 is still in-progress (M2 hardening), and daily validation often needs "WASM-only confidence" without running every automation/input contract.
- Existing `core_http` contract checks were all-or-nothing, which increased iteration cost for WASM-specific closure work.

## Scope
- Keep existing `all` contract behavior unchanged.
- Add a WASM-focused contract mode and dedicated entry script.
- Expand macOS WASM selfcheck to cover transfer/error-code semantics in the same one-command flow.

## Code Changes

### 1) Core HTTP contract scope split (`all` + `wasm`)
- File: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http.sh`
- `mfx_run_core_http_contract_checks` now accepts optional third argument `check_scope`:
  - `all` (default): preserves existing full behavior.
  - `wasm`: runs state/schema + WASM catalog/import/export/load/enable/dispatch/failure checks, and skips non-WASM automation/input-capture assertions.
- Startup permission simulation is scope-aware:
  - `all` keeps previous startup-denied -> recover -> revoke -> regrant flow.
  - `wasm` starts trusted to avoid non-WASM permission transition dependency.

### 2) New WASM-only regression entry script
- File: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-wasm-contract-regression.sh`
- Behavior:
  - macOS-only in current roadmap (Linux host call exits with skip message).
  - Builds `mfx_entry_posix_host` with core runtime lane ON.
  - Runs `mfx_run_core_http_contract_checks ... "wasm"`.
- Purpose: provide a faster focused gate for WASM M2 closure without replacing full-suite gates.

### 3) macOS WASM selfcheck transfer coverage expansion
- File: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_common.sh`
  - Added helperized transfer/probe assertions:
    - import-selected (ok + failure code)
    - export-all (count/path/manifest consistency)
    - import-from-folder-dialog probe
- File: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
  - One-command selfcheck now additionally validates:
    - `/api/wasm/catalog` base contract
    - folder-dialog `probe_only=true` support contract
    - import-selected success
    - import-selected failure `error_code` semantics (`manifest_path_not_found`, `manifest_path_not_file`, `manifest_path_required`)
    - export-all output contract before load/enable/dispatch checks

## Validation
- `bash -n tools/platform/regression/lib/core_http.sh`
- `bash -n tools/platform/regression/run-posix-core-wasm-contract-regression.sh`
- `bash -n tools/platform/manual/lib/wasm_selfcheck_common.sh`
- `bash -n tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto`
- `./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Result
- WASM closure work now has a dedicated focused regression gate while full `all` contracts remain unchanged.
- macOS WASM manual selfcheck is no longer load/invoke-only; transfer/error-code contracts are covered in the same command.
