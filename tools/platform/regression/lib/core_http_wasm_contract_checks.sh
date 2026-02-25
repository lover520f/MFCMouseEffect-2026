#!/usr/bin/env bash

set -euo pipefail

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

    local code_wasm_catalog
    code_wasm_catalog="$(mfx_http_code "$tmp_dir/wasm-catalog.out" "$base_url/api/wasm/catalog" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{}')"
    mfx_assert_eq "$code_wasm_catalog" "200" "core wasm catalog status"
    mfx_assert_file_contains "$tmp_dir/wasm-catalog.out" "\"ok\":true" "core wasm catalog ok"
    mfx_assert_file_contains "$tmp_dir/wasm-catalog.out" "\"plugins\":" "core wasm catalog plugins field"
    mfx_assert_file_contains "$tmp_dir/wasm-catalog.out" "\"search_roots\":" "core wasm catalog search_roots field"

    local code_wasm_import_dialog_probe
    code_wasm_import_dialog_probe="$(mfx_http_code "$tmp_dir/wasm-import-dialog-probe.out" "$base_url/api/wasm/import-from-folder-dialog" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{"probe_only":true}')"
    mfx_assert_eq "$code_wasm_import_dialog_probe" "200" "core wasm import dialog probe status"
    mfx_assert_file_contains "$tmp_dir/wasm-import-dialog-probe.out" "\"ok\":true" "core wasm import dialog probe ok"
    mfx_assert_file_contains "$tmp_dir/wasm-import-dialog-probe.out" "\"probe_only\":true" "core wasm import dialog probe mode"
    mfx_assert_file_contains "$tmp_dir/wasm-import-dialog-probe.out" "\"supported\":" "core wasm import dialog supported field"
    mfx_assert_file_contains "$tmp_dir/wasm-import-dialog-probe.out" "\"error_code\":" "core wasm import dialog probe error_code field"

    local wasm_manifest_path
    wasm_manifest_path="$(_mfx_core_http_detect_wasm_manifest_path "$repo_root" "$tmp_dir/wasm-catalog.out")"
    if [[ -n "$wasm_manifest_path" ]]; then
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

        _mfx_core_http_assert_wasm_load_manifest_ok \
            "$tmp_dir/wasm-load-manifest.out" \
            "$base_url" \
            "$token" \
            "$wasm_manifest_path" \
            "core wasm load-manifest"

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

        _mfx_core_http_assert_wasm_load_manifest_failure \
            "$tmp_dir/wasm-load-invalid.out" \
            "$base_url" \
            "$token" \
            "$invalid_manifest_path" \
            "manifest_load" \
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

    if [[ "$platform" == "macos" ]]; then
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
    fi
}
