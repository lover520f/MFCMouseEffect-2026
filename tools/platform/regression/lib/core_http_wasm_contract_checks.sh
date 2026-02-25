#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_wasm_contract_checks_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$_mfx_core_http_wasm_contract_checks_dir/wasm_fixture_helpers.sh"
source "$_mfx_core_http_wasm_contract_checks_dir/core_http_wasm_contract_catalog_checks.sh"
source "$_mfx_core_http_wasm_contract_checks_dir/core_http_wasm_contract_path_checks.sh"
source "$_mfx_core_http_wasm_contract_checks_dir/core_http_wasm_contract_runtime_checks.sh"
source "$_mfx_core_http_wasm_contract_checks_dir/core_http_wasm_contract_transfer_checks.sh"
source "$_mfx_core_http_wasm_contract_checks_dir/core_http_wasm_contract_fixture_checks.sh"
source "$_mfx_core_http_wasm_contract_checks_dir/core_http_wasm_contract_dispatch_checks.sh"
source "$_mfx_core_http_wasm_contract_checks_dir/core_http_wasm_contract_platform_checks.sh"

_mfx_core_http_detect_wasm_manifest_path() {
    local repo_root="$1"
    local catalog_output_file="$2"
    local manifest_path="$repo_root/examples/wasm-plugin-template/dist/plugin.json"
    if [[ ! -f "$manifest_path" ]]; then
        manifest_path="$(sed -n 's/.*"manifest_path":"\([^"]*\)".*/\1/p' "$catalog_output_file" | head -n 1)"
    fi
    printf '%s' "$manifest_path"
}

_mfx_core_http_run_wasm_contract_checks() {
    local platform="$1"
    local tmp_dir="$2"
    local base_url="$3"
    local token="$4"
    local repo_root="$5"
    local wasm_check_profile="${MFX_CORE_HTTP_WASM_CHECK_PROFILE:-all}"

    if [[ "$wasm_check_profile" != "all" && "$wasm_check_profile" != "path" ]]; then
        mfx_fail "core wasm check profile must be one of: all, path (got: $wasm_check_profile)"
    fi

    _mfx_core_http_wasm_contract_catalog_checks \
        "$tmp_dir" \
        "$base_url" \
        "$token" \
        "$repo_root"

    local wasm_manifest_path
    wasm_manifest_path="$(_mfx_core_http_detect_wasm_manifest_path "$repo_root" "$tmp_dir/wasm-catalog.out")"
    if [[ -n "$wasm_manifest_path" ]]; then
        _mfx_core_http_wasm_contract_path_checks \
            "$tmp_dir" \
            "$base_url" \
            "$token" \
            "$wasm_manifest_path"

        if [[ "$wasm_check_profile" == "path" ]]; then
            return 0
        fi

        _mfx_core_http_wasm_contract_transfer_checks \
            "$tmp_dir" \
            "$base_url" \
            "$token" \
            "$wasm_manifest_path"

        _mfx_core_http_wasm_contract_runtime_checks \
            "$tmp_dir" \
            "$base_url" \
            "$token" \
            "$wasm_manifest_path"

        _mfx_core_http_wasm_contract_fixture_checks \
            "$tmp_dir" \
            "$base_url" \
            "$token" \
            "$wasm_manifest_path"

        _mfx_core_http_wasm_contract_dispatch_checks \
            "$platform" \
            "$tmp_dir" \
            "$base_url" \
            "$token"

        local invalid_manifest_path="${wasm_manifest_path}.missing"
        _mfx_core_http_assert_wasm_load_manifest_failure \
            "$tmp_dir/wasm-load-invalid.out" \
            "$base_url" \
            "$token" \
            "$invalid_manifest_path" \
            "manifest_load" \
            "manifest_io_error" \
            "manifest_io_error" \
            "core wasm load-manifest invalid path"

        _mfx_core_http_assert_wasm_load_manifest_ok \
            "$tmp_dir/wasm-load-manifest-reload.out" \
            "$base_url" \
            "$token" \
            "$wasm_manifest_path" \
            "core wasm load-manifest reload"
    else
        mfx_info "skip wasm dispatch test: no plugin manifest found"
    fi

    _mfx_core_http_wasm_contract_platform_checks "$platform" "$tmp_dir"
}
