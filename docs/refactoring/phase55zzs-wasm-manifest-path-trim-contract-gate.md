# Phase 55zzs: WASM Path Trim Contract Gate

## Capability
- WASM

## Why
- WASM routes already trim ASCII whitespace from path payload fields (`manifest_path`, `initial_path`).
- Previous regression checks covered required/invalid/success paths, but did not explicitly lock:
  - `load-manifest` padded path (`"  /path/plugin.json  "`) should still load successfully
  - `load-manifest` blank path (`"   "`) should fail as required-path error
  - folder-dialog probe padded `initial_path` should roundtrip as trimmed value

## Scope
- Extend regression and manual selfcheck contracts for:
  - `load-manifest` trimmed-path + blank-path semantics
  - folder-dialog probe `initial_path` trimmed-path semantics
- Keep backend behavior unchanged; this is contract hardening.

## Code Changes
1. Regression helper expansion:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_helpers.sh`
   - Added:
     - `_mfx_core_http_wasm_load_manifest_trimmed_payload_http_code`
     - `_mfx_core_http_assert_wasm_load_manifest_trimmed_path_ok`
     - `_mfx_core_http_assert_wasm_load_manifest_blank_path_required_failure`
     - `_mfx_core_http_wasm_import_dialog_probe_http_code`
     - `_mfx_core_http_assert_wasm_import_dialog_probe_supported`
     - `_mfx_core_http_assert_wasm_import_dialog_probe_trimmed_initial_path`
2. Regression contract wiring:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_checks.sh`
   - Added checks:
     - trimmed-path load success + `active_manifest_path` equality
     - blank-path required failure (`manifest_path_required`)
3. Manual selfcheck helper expansion:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_common.sh`
   - Added:
     - `mfx_wasm_selfcheck_load_manifest_trimmed_payload_http_code`
     - `mfx_wasm_selfcheck_assert_load_manifest_trimmed_path_ok`
     - `mfx_wasm_selfcheck_assert_load_manifest_blank_path_required_failure`
     - `mfx_wasm_selfcheck_import_dialog_probe_http_code` (`initial_path` payload support)
     - `mfx_wasm_selfcheck_assert_import_dialog_probe_trimmed_initial_path`
4. Manual selfcheck wiring:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
   - Added the same trimmed/blank path assertions in one-command selfcheck flow.

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_common.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- `./tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto`
- `./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No runtime API/schema change.
- Added explicit gates for:
  - `manifest_path` whitespace-trim semantics
  - `manifest_path` blank-path required failure semantics
  - `initial_path` whitespace-trim semantics in folder-dialog probe mode
