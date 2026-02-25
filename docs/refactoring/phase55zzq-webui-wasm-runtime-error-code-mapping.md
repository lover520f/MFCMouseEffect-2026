# Phase 55zzq: WebUI WASM Runtime Error-Code Mapping Expansion

## Capability
- WASM

## Why
- Backend runtime routes now expose structured `error_code` for load/reload flows.
- WebUI error-code model previously focused on transfer/import/export codes; several runtime reload/load codes fell back to raw backend text.

## Scope
- Expand shared WebUI WASM action error-code mapping for runtime codes.
- Extend wasm error-model test script with runtime code cases.
- Backfill legacy WebUI i18n dictionary (EN/ZH) with runtime error-code keys so translated rendering and fallback rendering stay aligned.
- Keep API/runtime behavior unchanged; this is presentation + test hardening.

## Code Changes

### 1) Runtime error-code mapping
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUIWorkspace/src/wasm/action-error-model.js`
- Added mappings for runtime-focused codes including:
  - `reload_target_missing`
  - `module_load_failed`
  - `manifest_api_unsupported`
  - `load_manifest_failed`
  - `reload_failed`
  - plus fallback/system-side codes (`no_controller`, `wasm_host_unavailable`, `runtime_unavailable`, `unknown_error`, etc.)

### 2) Test coverage expansion
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUIWorkspace/scripts/test-wasm-action-error-model.mjs`
- Added assertions for:
  - runtime reload/load fallback messages
  - translation callback behavior on runtime code keys

### 3) Legacy WebUI i18n parity
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUI/i18n.js`
- Added EN/ZH keys for runtime action errors so legacy i18n table matches action error-model coverage:
  - `wasm_error_no_controller`
  - `wasm_error_wasm_host_unavailable`
  - `wasm_error_unknown_error`
  - `wasm_error_load_manifest_failed`
  - `wasm_error_reload_failed`
  - `wasm_error_reload_target_missing`
  - `wasm_error_runtime_unavailable`
  - `wasm_error_module_load_failed`
  - `wasm_error_api_version_call_failed`
  - `wasm_error_api_version_unsupported`
  - `wasm_error_manifest_io_error`
  - `wasm_error_manifest_json_parse_error`
  - `wasm_error_manifest_invalid`
  - `wasm_error_manifest_api_unsupported`
  - `wasm_error_entry_wasm_path_invalid`

## Validation
- `node /Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUIWorkspace/scripts/test-wasm-action-error-model.mjs`
- `pnpm --dir /Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUIWorkspace test:wasm-error-model`
- `pnpm --dir /Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUIWorkspace run build`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No backend contract change.
- WebUI now surfaces stable, user-readable messages for runtime WASM error codes instead of relying on raw backend error text.
