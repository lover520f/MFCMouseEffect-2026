#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_wasm_json_escape() {
    local value="$1"
    printf '%s' "$value" | sed 's/\\/\\\\/g; s/"/\\"/g'
}

_mfx_core_http_wasm_load_manifest_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"

    local manifest_path_escaped
    manifest_path_escaped="$(_mfx_core_http_wasm_json_escape "$manifest_path")"
    mfx_http_code "$output_file" "$base_url/api/wasm/load-manifest" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "{\"manifest_path\":\"$manifest_path_escaped\"}"
}

_mfx_core_http_wasm_import_selected_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"

    local manifest_path_escaped
    manifest_path_escaped="$(_mfx_core_http_wasm_json_escape "$manifest_path")"
    mfx_http_code "$output_file" "$base_url/api/wasm/import-selected" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "{\"manifest_path\":\"$manifest_path_escaped\"}"
}

_mfx_core_http_wasm_export_all_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    mfx_http_code "$output_file" "$base_url/api/wasm/export-all" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{}'
}

_mfx_core_http_assert_wasm_load_manifest_ok() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"
    local context="$5"

    local code
    code="$(_mfx_core_http_wasm_load_manifest_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_stage\":\"\"" "$context failure stage cleared"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_code\":\"\"" "$context failure code cleared"
}

_mfx_core_http_assert_wasm_load_manifest_failure() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"
    local expected_stage="$5"
    local expected_code="$6"
    local context="$7"

    local code
    code="$(_mfx_core_http_wasm_load_manifest_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":false" "$context should fail"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_stage\":\"$expected_stage\"" "$context failure stage"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_code\":\"$expected_code\"" "$context failure code"
}

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
