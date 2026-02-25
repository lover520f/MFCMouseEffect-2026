#!/usr/bin/env bash

set -euo pipefail

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
    mfx_assert_file_contains "$output_file" "\"error_code\":\"\"" "selfcheck $label error code clear"
}

mfx_wasm_selfcheck_assert_load_manifest_trimmed_path_ok() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"
    local manifest_path="$5"

    local code
    code="$(mfx_wasm_selfcheck_load_manifest_trimmed_payload_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "selfcheck $label ok"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"\"" "selfcheck $label error code clear"

    local active_manifest_path
    active_manifest_path="$(mfx_wasm_selfcheck_parse_string_field "$output_file" "active_manifest_path")"
    mfx_assert_eq "$active_manifest_path" "$manifest_path" "selfcheck $label active manifest path"
}

mfx_wasm_selfcheck_assert_load_manifest_failure() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"
    local manifest_path="$5"
    local expected_stage="$6"
    local expected_code="$7"
    local expected_error_code="${8:-$expected_code}"

    local code
    code="$(mfx_wasm_selfcheck_load_manifest_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":false" "selfcheck $label should fail"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_stage\":\"$expected_stage\"" \
        "selfcheck $label stage"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_code\":\"$expected_code\"" \
        "selfcheck $label code"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"$expected_error_code\"" \
        "selfcheck $label error code"
}

mfx_wasm_selfcheck_assert_load_manifest_required_failure() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"

    local code
    code="$(mfx_wasm_selfcheck_load_manifest_payload_http_code "$output_file" "$base_url" "$token" '{}')"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":false" "selfcheck $label should fail"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"manifest_path_required\"" \
        "selfcheck $label error code"
}

mfx_wasm_selfcheck_assert_load_manifest_blank_path_required_failure() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"

    local code
    code="$(mfx_wasm_selfcheck_load_manifest_payload_http_code "$output_file" "$base_url" "$token" '{"manifest_path":"   "}')"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":false" "selfcheck $label should fail"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"manifest_path_required\"" \
        "selfcheck $label error code"
}

mfx_wasm_selfcheck_reload_failure() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"
    local expected_error_code="$5"
    local expected_stage="${6:-}"
    local expected_code="${7:-}"

    local code
    code="$(mfx_wasm_selfcheck_reload_http_code "$output_file" "$base_url" "$token")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":false" "selfcheck $label should fail"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"$expected_error_code\"" \
        "selfcheck $label error code"
    if [[ -n "$expected_stage" ]]; then
        mfx_assert_file_contains "$output_file" "\"last_load_failure_stage\":\"$expected_stage\"" \
            "selfcheck $label stage"
    fi
    if [[ -n "$expected_code" ]]; then
        mfx_assert_file_contains "$output_file" "\"last_load_failure_code\":\"$expected_code\"" \
            "selfcheck $label code"
    fi
}

mfx_wasm_selfcheck_assert_test_reset_runtime_ok() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"

    local code
    code="$(mfx_wasm_selfcheck_test_reset_runtime_http_code "$output_file" "$base_url" "$token")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "selfcheck $label ok"
    mfx_assert_file_contains "$output_file" "\"plugin_loaded\":false" "selfcheck $label plugin unloaded"
    mfx_assert_file_contains "$output_file" "\"has_active_manifest_path\":false" "selfcheck $label manifest target cleared"
    mfx_assert_file_contains "$output_file" "\"has_active_wasm_path\":false" "selfcheck $label wasm target cleared"
}

mfx_wasm_selfcheck_assert_reload_ok() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"

    local code
    code="$(mfx_wasm_selfcheck_reload_http_code "$output_file" "$base_url" "$token")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "selfcheck $label ok"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"\"" "selfcheck $label error code clear"
}
