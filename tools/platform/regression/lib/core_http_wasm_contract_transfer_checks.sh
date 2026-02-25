#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_wasm_contract_transfer_checks() {
    local tmp_dir="$1"
    local base_url="$2"
    local token="$3"
    local wasm_manifest_path="$4"

    _mfx_core_http_assert_wasm_import_selected_ok \
        "$tmp_dir/wasm-import-selected.out" \
        "$base_url" \
        "$token" \
        "$wasm_manifest_path" \
        "core wasm import-selected"

    _mfx_core_http_assert_wasm_export_all_ok \
        "$tmp_dir/wasm-export-all.out" \
        "$base_url" \
        "$token" \
        "1" \
        "core wasm export-all"

    local invalid_manifest_path="${wasm_manifest_path}.missing"
    _mfx_core_http_assert_wasm_import_selected_failure \
        "$tmp_dir/wasm-import-selected-invalid.out" \
        "$base_url" \
        "$token" \
        "$invalid_manifest_path" \
        "manifest_path_not_found" \
        "manifest_path does not exist" \
        "core wasm import-selected invalid path"

    local invalid_manifest_not_file_path="$tmp_dir/wasm-import-not-file"
    mkdir -p "$invalid_manifest_not_file_path"
    _mfx_core_http_assert_wasm_import_selected_failure \
        "$tmp_dir/wasm-import-selected-not-file.out" \
        "$base_url" \
        "$token" \
        "$invalid_manifest_not_file_path" \
        "manifest_path_not_file" \
        "manifest_path is not a file" \
        "core wasm import-selected not file path"

    local invalid_manifest_json_path="$tmp_dir/wasm-import-invalid-json.json"
    printf '{invalid-json' >"$invalid_manifest_json_path"
    _mfx_core_http_assert_wasm_import_selected_failure \
        "$tmp_dir/wasm-import-selected-invalid-json.out" \
        "$base_url" \
        "$token" \
        "$invalid_manifest_json_path" \
        "manifest_load_failed" \
        "" \
        "core wasm import-selected invalid json"

    _mfx_core_http_assert_wasm_import_selected_failure \
        "$tmp_dir/wasm-import-selected-required.out" \
        "$base_url" \
        "$token" \
        "   " \
        "manifest_path_required" \
        "manifest_path is required" \
        "core wasm import-selected required path"
}
