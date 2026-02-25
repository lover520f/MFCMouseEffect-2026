#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_wasm_contract_fixture_checks() {
    local tmp_dir="$1"
    local base_url="$2"
    local token="$3"
    local wasm_manifest_path="$4"

    local reload_failure_fixture_dir="$tmp_dir/wasm-reload-failure-fixture"
    local reload_failure_manifest_path
    reload_failure_manifest_path="$(mfx_wasm_fixture_manifest_copy \
        "$wasm_manifest_path" \
        "$reload_failure_fixture_dir" \
        "core wasm reload failure fixture")"
    mfx_wasm_fixture_require_entry_file "$reload_failure_manifest_path" "core wasm reload failure fixture" >/dev/null

    _mfx_core_http_assert_wasm_load_manifest_ok \
        "$tmp_dir/wasm-load-manifest-reload-failure-fixture.out" \
        "$base_url" \
        "$token" \
        "$reload_failure_manifest_path" \
        "core wasm load-manifest reload-failure fixture"

    mfx_wasm_fixture_remove_entry_file "$reload_failure_manifest_path" "core wasm reload failure fixture" >/dev/null

    _mfx_core_http_assert_wasm_reload_failure \
        "$tmp_dir/wasm-reload-module-missing.out" \
        "$base_url" \
        "$token" \
        "module_load_failed" \
        "load_module" \
        "module_load_failed" \
        "core wasm reload with missing module"

    _mfx_core_http_assert_wasm_load_manifest_ok \
        "$tmp_dir/wasm-load-manifest-after-reload-failure.out" \
        "$base_url" \
        "$token" \
        "$wasm_manifest_path" \
        "core wasm load-manifest restore after reload failure"

    local reload_api_fixture_dir="$tmp_dir/wasm-reload-api-fixture"
    local reload_api_manifest_path
    reload_api_manifest_path="$(mfx_wasm_fixture_manifest_copy \
        "$wasm_manifest_path" \
        "$reload_api_fixture_dir" \
        "core wasm reload api fixture")"
    local reload_api_entry_relative
    reload_api_entry_relative="$(mfx_wasm_fixture_manifest_entry_relative \
        "$reload_api_manifest_path" \
        "core wasm reload api fixture")"

    _mfx_core_http_assert_wasm_load_manifest_ok \
        "$tmp_dir/wasm-load-manifest-reload-api-fixture.out" \
        "$base_url" \
        "$token" \
        "$reload_api_manifest_path" \
        "core wasm load-manifest reload-api fixture"

    mfx_wasm_fixture_write_manifest_with_api_version \
        "$reload_api_manifest_path" \
        "reload-api-unsupported-plugin" \
        "2" \
        "$reload_api_entry_relative"

    _mfx_core_http_assert_wasm_reload_failure \
        "$tmp_dir/wasm-reload-api-unsupported.out" \
        "$base_url" \
        "$token" \
        "manifest_api_unsupported" \
        "manifest_api_version" \
        "manifest_api_unsupported" \
        "core wasm reload with unsupported manifest api"

    _mfx_core_http_assert_wasm_load_manifest_ok \
        "$tmp_dir/wasm-load-manifest-after-api-reload-failure.out" \
        "$base_url" \
        "$token" \
        "$wasm_manifest_path" \
        "core wasm load-manifest restore after api reload failure"
}
