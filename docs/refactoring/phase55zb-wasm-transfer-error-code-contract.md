# Phase 55zb: WASM Transfer Error-Code Contract

## Issue Classification
- Verdict: `Process debt`.
- Problem: WASM transfer APIs (`import-selected`, `export-all`) previously exposed only free-form `error` text, making regression assertions brittle and less stable across message wording changes.

## Goal
1. Add stable machine-readable `error_code` fields for transfer responses.
2. Keep existing `error` text for backward-compatible human diagnostics.
3. Upgrade regression checks to assert `error_code` semantics.

## Implementation
- Updated transfer result models:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.h`
  - `PluginImportResult` and `PluginExportResult` now include `errorCode`.
- Updated transfer service mapping:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.cpp`
  - import path now maps stable codes (for example: `manifest_path_not_found`, `manifest_load_failed`, `copy_failed`).
  - export path now maps stable codes (for example: `no_plugins_discovered`, `create_export_directory_failed`, `copy_failed`).
- Updated HTTP route responses:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmImportSelectedRoute.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmExportRoutes.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.WasmImportFolderDialogRoute.cpp`
  - transfer and folder-dialog responses now include `error_code` for deterministic consumer handling.
- Updated regression contracts:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_wasm_helpers.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http.sh`
  - import failure contract now asserts `error_code=manifest_path_not_found` (with legacy text kept as secondary assertion).

## Validation
- `bash -n tools/platform/regression/lib/core_http_wasm_helpers.sh`
- `bash -n tools/platform/regression/lib/core_http.sh`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Closure
- WASM transfer error handling now has stable API-level error-code semantics, reducing flaky string-coupled checks while keeping backward-compatible error text.
