#!/usr/bin/env bash

set -euo pipefail

mfx_wasm_selfcheck_json_escape() {
    local value="$1"
    printf '%s' "$value" | sed 's/\\/\\\\/g; s/"/\\"/g'
}

mfx_wasm_selfcheck_load_manifest_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"

    local manifest_path_escaped
    manifest_path_escaped="$(mfx_wasm_selfcheck_json_escape "$manifest_path")"
    mfx_http_code "$output_file" "$base_url/api/wasm/load-manifest" \
        -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" \
        -d "{\"manifest_path\":\"$manifest_path_escaped\"}"
}

mfx_wasm_selfcheck_assert_load_manifest_ok() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"
    local manifest_path="$5"

    local code
    code="$(mfx_wasm_selfcheck_load_manifest_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "selfcheck $label ok"
}

mfx_wasm_selfcheck_assert_load_manifest_failure() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"
    local manifest_path="$5"
    local expected_stage="$6"
    local expected_code="$7"

    local code
    code="$(mfx_wasm_selfcheck_load_manifest_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":false" "selfcheck $label should fail"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_stage\":\"$expected_stage\"" \
        "selfcheck $label stage"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_code\":\"$expected_code\"" \
        "selfcheck $label code"
}

mfx_wasm_selfcheck_import_selected_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"

    local manifest_path_escaped
    manifest_path_escaped="$(mfx_wasm_selfcheck_json_escape "$manifest_path")"
    mfx_http_code "$output_file" "$base_url/api/wasm/import-selected" \
        -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" \
        -d "{\"manifest_path\":\"$manifest_path_escaped\"}"
}

mfx_wasm_selfcheck_export_all_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"

    mfx_http_code "$output_file" "$base_url/api/wasm/export-all" \
        -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" \
        -d '{}'
}

mfx_wasm_selfcheck_import_dialog_probe_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"

    mfx_http_code "$output_file" "$base_url/api/wasm/import-from-folder-dialog" \
        -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" \
        -d '{"probe_only":true}'
}

mfx_wasm_selfcheck_assert_import_selected_ok() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"
    local manifest_path="$5"

    local code
    code="$(mfx_wasm_selfcheck_import_selected_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "selfcheck $label ok"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"\"" "selfcheck $label error code clear"
}

mfx_wasm_selfcheck_assert_import_selected_failure() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"
    local manifest_path="$5"
    local expected_error_code="$6"

    local code
    code="$(mfx_wasm_selfcheck_import_selected_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":false" "selfcheck $label should fail"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"$expected_error_code\"" \
        "selfcheck $label error code"
}

mfx_wasm_selfcheck_assert_export_all_ok() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"
    local minimum_count="$5"

    local code
    code="$(mfx_wasm_selfcheck_export_all_http_code "$output_file" "$base_url" "$token")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "selfcheck $label ok"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"\"" "selfcheck $label error code clear"

    local export_path
    export_path="$(sed -n 's/.*"export_path":"\([^"]*\)".*/\1/p' "$output_file" | head -n 1)"
    if [[ -z "$export_path" ]]; then
        mfx_fail "selfcheck $label export path parse failed"
    fi
    if [[ ! -d "$export_path" ]]; then
        mfx_fail "selfcheck $label export path not found: $export_path"
    fi

    local exported_count
    exported_count="$(sed -n 's/.*"count":\([0-9][0-9]*\).*/\1/p' "$output_file" | head -n 1)"
    if [[ -z "$exported_count" ]]; then
        mfx_fail "selfcheck $label exported count parse failed"
    fi
    if (( exported_count < minimum_count )); then
        mfx_fail "selfcheck $label exported count too small: expected >= $minimum_count got $exported_count"
    fi

    local exported_manifest_count
    exported_manifest_count="$(find "$export_path" -mindepth 2 -maxdepth 2 -type f -name 'plugin.json' | wc -l | tr -d ' ')"
    if [[ "$exported_manifest_count" != "$exported_count" ]]; then
        mfx_fail "selfcheck $label exported manifest count mismatch: response=$exported_count manifests=$exported_manifest_count"
    fi
}

mfx_wasm_selfcheck_assert_import_dialog_probe_supported() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"

    local code
    code="$(mfx_wasm_selfcheck_import_dialog_probe_http_code "$output_file" "$base_url" "$token")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "selfcheck $label ok"
    mfx_assert_file_contains "$output_file" "\"probe_only\":true" "selfcheck $label probe flag"
    mfx_assert_file_contains "$output_file" "\"supported\":true" "selfcheck $label supported"
}
