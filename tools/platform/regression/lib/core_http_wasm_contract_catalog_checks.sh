#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_wasm_contract_catalog_checks() {
    local tmp_dir="$1"
    local base_url="$2"
    local token="$3"
    local repo_root="$4"

    local code_wasm_catalog
    code_wasm_catalog="$(mfx_http_code "$tmp_dir/wasm-catalog.out" "$base_url/api/wasm/catalog" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{}')"
    mfx_assert_eq "$code_wasm_catalog" "200" "core wasm catalog status"
    mfx_assert_file_contains "$tmp_dir/wasm-catalog.out" "\"ok\":true" "core wasm catalog ok"
    mfx_assert_file_contains "$tmp_dir/wasm-catalog.out" "\"plugins\":" "core wasm catalog plugins field"
    mfx_assert_file_contains "$tmp_dir/wasm-catalog.out" "\"search_roots\":" "core wasm catalog search_roots field"

    _mfx_core_http_assert_wasm_import_dialog_probe_supported \
        "$tmp_dir/wasm-import-dialog-probe.out" \
        "$base_url" \
        "$token" \
        "core wasm import dialog probe"

    _mfx_core_http_assert_wasm_import_dialog_probe_trimmed_initial_path \
        "$tmp_dir/wasm-import-dialog-probe-trimmed-initial-path.out" \
        "$base_url" \
        "$token" \
        "$repo_root/examples" \
        "core wasm import dialog probe trimmed initial path"
}
