# Phase 55zd: WASM Transfer Error-Code Regression Matrix and i18n

## Issue Classification
- Verdict: `Process debt`.
- Problem:
  - transfer failure regression still covered only a subset (`manifest_path_not_found`),
  - WebUI error display preferred backend free-form `error` text over `error_code` mapping, reducing localization quality.

## Goal
1. Expand deterministic transfer failure-code matrix in core HTTP regression.
2. Ensure WebUI error display prefers `error_code` mapping for stable and localizable feedback.
3. Keep compatibility by preserving free-form `error` fallback.

## Implementation
- Updated regression matrix:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http.sh`
  - added import failure assertions for:
    - `manifest_path_required` (blank path),
    - `manifest_path_not_file` (directory path),
    - `manifest_load_failed` (invalid JSON manifest file),
    - existing `manifest_path_not_found`.
  - import-folder-dialog probe now asserts `error_code` field presence.
- Updated WebUI error preference:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
  - action error message now prefers mapped `error_code` text, then falls back to backend `error`.
- Added EN/ZH transfer error-code translations:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUI/i18n.js`

## Validation
- `bash -n tools/platform/regression/lib/core_http.sh`
- `bash -n tools/platform/regression/lib/core_http_wasm_helpers.sh`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `pnpm --dir MFCMouseEffect/WebUIWorkspace run build`
- `pnpm --dir MFCMouseEffect/WebUIWorkspace run test:automation-platform`

## Closure
- WASM transfer path now has broader failure-code regression coverage and localized WebUI-first error-code presentation, reducing both contract blind spots and diagnostics drift.
