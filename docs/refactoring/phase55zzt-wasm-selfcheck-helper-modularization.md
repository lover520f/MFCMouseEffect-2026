# Phase 55zzt: WASM Selfcheck Helper Modularization

## Capability
- WASM

## Why
- `tools/platform/manual/lib/wasm_selfcheck_common.sh` had grown into a large mixed-responsibility file (HTTP wrappers + parsing + runtime/transfer/dispatch assertions).
- The single file made review and extension harder, and increased drift risk when adding new assertions.

## Scope
- Keep all existing selfcheck function names and behavior unchanged.
- Split helper responsibilities into dedicated modules:
  - parse helpers
  - HTTP request helpers
  - assertion helpers (runtime/transfer/dispatch)
- Keep one compatibility entry (`wasm_selfcheck_common.sh`) for existing callers.

## Code Changes
1. Added parse helper module:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_parse_helpers.sh`
2. Added HTTP helper module:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_http_helpers.sh`
3. Added assertion helper modules:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_runtime_assert_helpers.sh`
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_transfer_assert_helpers.sh`
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_dispatch_assert_helpers.sh`
4. Converted the following files into thin compatibility loaders:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_common.sh`
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_assert_helpers.sh`

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_common.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_parse_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_http_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_assert_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_runtime_assert_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_transfer_assert_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/wasm_selfcheck_dispatch_assert_helpers.sh`
- `./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No API/schema/runtime behavior change.
- Script internal architecture only; existing selfcheck call sites remain valid.
