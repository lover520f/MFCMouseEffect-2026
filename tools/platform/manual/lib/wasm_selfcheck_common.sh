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
