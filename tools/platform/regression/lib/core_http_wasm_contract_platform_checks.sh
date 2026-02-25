#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_wasm_contract_platform_checks() {
    local platform="$1"
    local tmp_dir="$2"

    if [[ "$platform" != "macos" ]]; then
        return 0
    fi

    if ! mfx_file_contains_fixed "$tmp_dir/state.out" "\"runtime_backend\":\"wasm3_static\""; then
        mfx_fail "core wasm runtime backend on macos: expected wasm3_static"
    fi
    if ! mfx_file_contains_fixed "$tmp_dir/state.out" "\"render_supported\":true"; then
        mfx_fail "core wasm render capability on macos: expected render_supported=true"
    fi
    if mfx_file_contains_fixed "$tmp_dir/wasm-catalog.out" "\"wasm_catalog_not_supported_on_this_platform\""; then
        mfx_fail "core wasm catalog support on macos: unexpected unsupported marker"
    fi
    if ! mfx_file_contains_fixed "$tmp_dir/wasm-import-dialog-probe.out" "\"supported\":true"; then
        mfx_fail "core wasm import dialog support on macos: expected supported=true"
    fi
}
