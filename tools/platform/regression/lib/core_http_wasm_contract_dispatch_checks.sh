#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_wasm_contract_dispatch_checks() {
    local platform="$1"
    local tmp_dir="$2"
    local base_url="$3"
    local token="$4"

    local code_wasm_enable
    code_wasm_enable="$(mfx_http_code "$tmp_dir/wasm-enable.out" "$base_url/api/wasm/enable" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{}')"
    mfx_assert_eq "$code_wasm_enable" "200" "core wasm enable status"
    mfx_assert_file_contains "$tmp_dir/wasm-enable.out" "\"ok\":true" "core wasm enable ok"

    local require_rendered_any="false"
    if [[ "$platform" == "macos" ]]; then
        require_rendered_any="true"
    fi
    _mfx_core_http_assert_wasm_test_dispatch_ok \
        "$tmp_dir/wasm-test-dispatch.out" \
        "$base_url" \
        "$token" \
        "core wasm test-dispatch" \
        "$require_rendered_any"

    local code_state_after_dispatch
    code_state_after_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_state_after_dispatch" "200" "core wasm state after dispatch status"
    _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
        "$tmp_dir/wasm-test-dispatch.out" \
        "$tmp_dir/state-after-wasm-dispatch.out" \
        "core wasm dispatch diagnostics consistency"
}
