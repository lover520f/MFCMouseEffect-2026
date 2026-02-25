#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_assert_wasm_import_selected_ok() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"
    local context="$5"

    local code
    code="$(_mfx_core_http_wasm_import_selected_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"\"" "$context error code clear"
    mfx_assert_file_contains "$output_file" "\"manifest_path\":\"" "$context manifest_path"
    mfx_assert_file_contains "$output_file" "\"primary_root_path\":\"" "$context primary_root_path"
}

_mfx_core_http_assert_wasm_import_selected_failure() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"
    local expected_error_code="$5"
    local expected_error_text="$6"
    local context="$7"

    local code
    code="$(_mfx_core_http_wasm_import_selected_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":false" "$context should fail"
    if [[ -n "$expected_error_code" ]]; then
        mfx_assert_file_contains "$output_file" "\"error_code\":\"$expected_error_code\"" "$context error code"
    fi
    if [[ -n "$expected_error_text" ]]; then
        mfx_assert_file_contains "$output_file" "\"error\":\"$expected_error_text\"" "$context error text"
    fi
}

_mfx_core_http_assert_wasm_export_all_ok() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local minimum_count="$4"
    local context="$5"

    local code
    code="$(_mfx_core_http_wasm_export_all_http_code "$output_file" "$base_url" "$token")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"\"" "$context error code clear"
    mfx_assert_file_contains "$output_file" "\"export_path\":\"" "$context export path"
    mfx_assert_file_contains "$output_file" "\"count\":" "$context count field"

    local export_path
    export_path="$(sed -n 's/.*"export_path":"\([^"]*\)".*/\1/p' "$output_file" | head -n 1)"
    if [[ -z "$export_path" ]]; then
        mfx_fail "$context export path parse failed"
    fi
    if [[ ! -d "$export_path" ]]; then
        mfx_fail "$context export path not found: $export_path"
    fi

    local exported_count
    exported_count="$(sed -n 's/.*"count":\([0-9][0-9]*\).*/\1/p' "$output_file" | head -n 1)"
    if [[ -z "$exported_count" ]]; then
        mfx_fail "$context exported count parse failed"
    fi
    if (( exported_count < minimum_count )); then
        mfx_fail "$context exported count too small: expected >= $minimum_count got $exported_count"
    fi

    local exported_dir_count
    exported_dir_count="$(find "$export_path" -mindepth 1 -maxdepth 1 -type d | wc -l | tr -d ' ')"
    if [[ -z "$exported_dir_count" ]]; then
        mfx_fail "$context exported directory count parse failed"
    fi
    if [[ "$exported_dir_count" != "$exported_count" ]]; then
        mfx_fail "$context exported directory count mismatch: response=$exported_count filesystem=$exported_dir_count"
    fi

    local exported_manifest_count
    exported_manifest_count="$(find "$export_path" -mindepth 2 -maxdepth 2 -type f -name 'plugin.json' | wc -l | tr -d ' ')"
    if [[ -z "$exported_manifest_count" ]]; then
        mfx_fail "$context exported manifest count parse failed"
    fi
    if [[ "$exported_manifest_count" != "$exported_count" ]]; then
        mfx_fail "$context exported manifest count mismatch: response=$exported_count manifests=$exported_manifest_count"
    fi

    while IFS= read -r exported_manifest; do
        if [[ ! -s "$exported_manifest" ]]; then
            mfx_fail "$context exported manifest is empty: $exported_manifest"
        fi
    done < <(find "$export_path" -mindepth 2 -maxdepth 2 -type f -name 'plugin.json' | sort)
}

_mfx_core_http_assert_wasm_import_dialog_probe_supported() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local context="$4"

    local code
    code="$(_mfx_core_http_wasm_import_dialog_probe_http_code "$output_file" "$base_url" "$token")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"probe_only\":true" "$context probe flag"
    mfx_assert_file_contains "$output_file" "\"supported\":true" "$context supported"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"\"" "$context error code clear"
}

_mfx_core_http_assert_wasm_import_dialog_probe_trimmed_initial_path() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local initial_path="$4"
    local context="$5"

    local code
    code="$(_mfx_core_http_wasm_import_dialog_probe_http_code "$output_file" "$base_url" "$token" "$initial_path")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"probe_only\":true" "$context probe flag"
    mfx_assert_file_contains "$output_file" "\"supported\":true" "$context supported"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"\"" "$context error code clear"

    local selected_folder_path
    selected_folder_path="$(_mfx_core_http_wasm_parse_string_field "$output_file" "selected_folder_path")"
    mfx_assert_eq "$selected_folder_path" "$initial_path" "$context selected folder path"
}
