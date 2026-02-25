#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_wasm_contract_runtime_checks() {
    local tmp_dir="$1"
    local base_url="$2"
    local token="$3"
    local wasm_manifest_path="$4"

    _mfx_core_http_assert_wasm_reload_ok \
        "$tmp_dir/wasm-reload.out" \
        "$base_url" \
        "$token" \
        "core wasm reload"

    _mfx_core_http_assert_wasm_test_reset_runtime_ok \
        "$tmp_dir/wasm-reset-runtime.out" \
        "$base_url" \
        "$token" \
        "core wasm reset runtime"

    _mfx_core_http_assert_wasm_reload_failure \
        "$tmp_dir/wasm-reload-without-target.out" \
        "$base_url" \
        "$token" \
        "reload_target_missing" \
        "" \
        "" \
        "core wasm reload without target"

    _mfx_core_http_assert_wasm_load_manifest_ok \
        "$tmp_dir/wasm-load-manifest-after-reset.out" \
        "$base_url" \
        "$token" \
        "$wasm_manifest_path" \
        "core wasm load-manifest after reset"
}
