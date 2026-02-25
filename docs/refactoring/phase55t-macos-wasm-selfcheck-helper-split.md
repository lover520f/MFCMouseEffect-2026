# Phase 55t: macOS WASM Selfcheck Helper Split

## Issue Classification
- Verdict: `Process debt`.
- Problem: `run-macos-wasm-runtime-selfcheck.sh` had repeated `load-manifest` HTTP/assert blocks, increasing maintenance cost and copy-paste drift risk.

## Goal
1. Reduce selfcheck script coupling and duplication.
2. Keep assertions readable while preserving existing behavior.
3. Maintain deterministic regression semantics across all load-failure branches.

## Implementation
- Added shared helper module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_common.sh`
  - Functions:
    - JSON escaping for manifest path payload.
    - shared `load-manifest` HTTP call wrapper.
    - success/failure assertion helpers with stage/code contract checks.
- Refactored:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
  - Replaced repeated inline request/assert blocks with helper calls.
  - Preserved existing functional checks (success path, failure-stage coverage, failure reset after valid reload).

## Validation
- `bash -n tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh tools/platform/manual/lib/wasm_selfcheck_common.sh`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Closure
- macOS WASM selfcheck remains behavior-equivalent but now has cleaner responsibility boundaries and lower change-risk for future failure-contract extensions.
