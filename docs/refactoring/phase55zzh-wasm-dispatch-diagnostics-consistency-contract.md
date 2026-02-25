# Phase 55zzh: WASM Dispatch Diagnostics Consistency Contract

## Capability
- WASM

## Why
- Existing checks verified `test-dispatch` success, but did not assert whether dispatch response counters and `/api/state` diagnostics stayed consistent.
- Throttle fields are easy to drift during future refactors (`throttled total` vs `capacity/interval breakdown`), and this drift is hard to notice manually.

## Scope
- Keep runtime behavior unchanged.
- Add script-level consistency assertions for dispatch diagnostics.
- Cover both regression workflow and macOS manual selfcheck entry.

## Code Changes

### 1) Regression helper contract expansion
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_helpers.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_contract_checks.sh`
- Added checks:
  - parse dispatch counters (`throttled`, `capacity`, `interval`, `dropped`, `render_error`)
  - parse `/api/state` counters (`last_throttled_*`, `last_dropped_render_commands`, `last_render_error`)
  - assert arithmetic invariant:
    - `throttled == throttled_by_capacity + throttled_by_interval`
  - assert dispatch snapshot and state snapshot values match.

### 2) Manual selfcheck contract expansion
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_common.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- Added checks:
  - fetch `/api/state` immediately after `test-dispatch`
  - reuse same counter consistency assertions in manual selfcheck flow.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Contract Impact
- No API schema or runtime behavior change.
- Only test/verification contracts are strengthened:
  - dispatch response diagnostics and `/api/state` diagnostics must stay consistent.
