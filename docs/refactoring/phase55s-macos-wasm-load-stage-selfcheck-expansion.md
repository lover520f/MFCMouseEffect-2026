# Phase 55s: macOS WASM Load-Stage Selfcheck Expansion

## Issue Classification
- Verdict: `Process debt`.
- Problem: existing selfcheck covered `manifest_load` classification only; stage-level failures like `manifest_api_version` and `load_module` were not asserted.

## Goal
1. Expand selfcheck to validate additional load-failure stages.
2. Ensure stage/code diagnostics are reset after a subsequent successful load.
3. Keep the flow deterministic and non-interactive.

## Implementation
- Extended `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh` with new fixtures and assertions:
  - `manifest_api_version` + `manifest_api_unsupported`:
    - temp manifest with `api_version=2`.
  - `load_module` + `module_load_failed`:
    - temp manifest with missing `entry` wasm file.
  - load-failure reset contract:
    - reload valid manifest and assert:
      - `last_load_failure_stage=""`
      - `last_load_failure_code=""`

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Closure
- macOS WASM selfcheck now covers both classification and stage-level contracts (`manifest_load`, `manifest_api_version`, `load_module`) and verifies failure-state cleanup after successful reload.
