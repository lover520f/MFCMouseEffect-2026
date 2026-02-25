# Phase 55zzu: Core HTTP WASM Helper Modularization and Lock Race Hardening

## Capability
- WASM
- Regression infrastructure

## Why
- `tools/platform/regression/lib/core_http_wasm_helpers.sh` had grown to mixed responsibilities (parse + HTTP + runtime/transfer/dispatch assertions) and was hard to evolve safely.
- Concurrent gate runs could occasionally hit a lock read race (`owner.env` removed between existence check and `sed`), causing unexpected script failure instead of normal wait.

## Scope
- Keep all existing WASM helper function names and call sites unchanged.
- Split core HTTP WASM helper stack by responsibility.
- Harden shared lock owner-pid read path to tolerate transient `owner.env` races.

## Code Changes
1. Modularized core HTTP WASM helper stack:
   - compatibility loader:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_helpers.sh`
   - parse helpers:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_parse_helpers.sh`
   - HTTP helpers:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_http_helpers.sh`
   - assert aggregating loader:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_assert_helpers.sh`
   - runtime assertions:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_runtime_assert_helpers.sh`
   - transfer assertions:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_transfer_assert_helpers.sh`
   - dispatch assertions:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_dispatch_assert_helpers.sh`
2. Hardened lock owner-pid read:
   - updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/common.sh`
   - added `mfx_read_lock_owner_pid` to handle missing/unreadable `owner.env` gracefully and avoid race-time `sed` failure.

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/common.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_parse_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_http_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_assert_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_runtime_assert_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_transfer_assert_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_dispatch_assert_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_checks.sh`
- `./tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-wasm-path-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No API/schema/runtime behavior change.
- Script internal architecture improved and lock-wait robustness hardened for concurrent local runs.
