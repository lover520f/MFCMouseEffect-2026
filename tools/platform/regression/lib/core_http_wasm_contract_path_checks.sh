#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_wasm_contract_path_checks() {
    local tmp_dir="$1"
    local base_url="$2"
    local token="$3"
    local wasm_manifest_path="$4"

    _mfx_core_http_assert_wasm_load_manifest_ok \
        "$tmp_dir/wasm-load-manifest.out" \
        "$base_url" \
        "$token" \
        "$wasm_manifest_path" \
        "core wasm load-manifest"

    _mfx_core_http_assert_wasm_load_manifest_trimmed_path_ok \
        "$tmp_dir/wasm-load-manifest-trimmed.out" \
        "$base_url" \
        "$token" \
        "$wasm_manifest_path" \
        "core wasm load-manifest trimmed path"

    _mfx_core_http_assert_wasm_load_manifest_required_failure \
        "$tmp_dir/wasm-load-required.out" \
        "$base_url" \
        "$token" \
        "core wasm load-manifest required path"

    _mfx_core_http_assert_wasm_load_manifest_blank_path_required_failure \
        "$tmp_dir/wasm-load-required-blank.out" \
        "$base_url" \
        "$token" \
        "core wasm load-manifest blank required path"
}
