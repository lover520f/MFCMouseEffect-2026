#!/usr/bin/env bash

set -euo pipefail

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
    mfx_assert_file_contains "$output_file" "\"error_code\":\"\"" "$context error code clear"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_stage\":\"\"" "$context failure stage cleared"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_code\":\"\"" "$context failure code cleared"
}

_mfx_core_http_assert_wasm_load_manifest_trimmed_path_ok() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"
    local context="$5"

    local code
    code="$(_mfx_core_http_wasm_load_manifest_trimmed_payload_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"\"" "$context error code clear"

    local active_manifest_path
    active_manifest_path="$(_mfx_core_http_wasm_parse_string_field "$output_file" "active_manifest_path")"
    mfx_assert_eq "$active_manifest_path" "$manifest_path" "$context active manifest path"
}

_mfx_core_http_assert_wasm_load_manifest_failure() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"
    local expected_stage="$5"
    local expected_code="$6"
    local expected_error_code="$7"
    local context="$8"

    local code
    code="$(_mfx_core_http_wasm_load_manifest_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":false" "$context should fail"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_stage\":\"$expected_stage\"" "$context failure stage"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_code\":\"$expected_code\"" "$context failure code"
    if [[ -n "$expected_error_code" ]]; then
        mfx_assert_file_contains "$output_file" "\"error_code\":\"$expected_error_code\"" "$context error code"
    fi
}

_mfx_core_http_assert_wasm_load_manifest_required_failure() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local context="$4"

    local code
    code="$(_mfx_core_http_wasm_load_manifest_payload_http_code "$output_file" "$base_url" "$token" '{}')"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":false" "$context should fail"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"manifest_path_required\"" "$context error code"
}

_mfx_core_http_assert_wasm_load_manifest_blank_path_required_failure() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local context="$4"

    local code
    code="$(_mfx_core_http_wasm_load_manifest_payload_http_code "$output_file" "$base_url" "$token" '{"manifest_path":"   "}')"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":false" "$context should fail"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"manifest_path_required\"" "$context error code"
}

_mfx_core_http_assert_wasm_reload_ok() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local context="$4"

    local code
    code="$(_mfx_core_http_wasm_reload_http_code "$output_file" "$base_url" "$token")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"\"" "$context error code clear"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_stage\":\"\"" "$context failure stage cleared"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_code\":\"\"" "$context failure code cleared"
}

_mfx_core_http_assert_wasm_reload_failure() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local expected_error_code="$4"
    local expected_stage="$5"
    local expected_code="$6"
    local context="$7"

    local code
    code="$(_mfx_core_http_wasm_reload_http_code "$output_file" "$base_url" "$token")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":false" "$context should fail"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"$expected_error_code\"" "$context error code"
    if [[ -n "$expected_stage" ]]; then
        mfx_assert_file_contains "$output_file" "\"last_load_failure_stage\":\"$expected_stage\"" "$context failure stage"
    fi
    if [[ -n "$expected_code" ]]; then
        mfx_assert_file_contains "$output_file" "\"last_load_failure_code\":\"$expected_code\"" "$context failure code"
    fi
}

_mfx_core_http_assert_wasm_test_reset_runtime_ok() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local context="$4"

    local code
    code="$(_mfx_core_http_wasm_test_reset_runtime_http_code "$output_file" "$base_url" "$token")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"plugin_loaded\":false" "$context plugin unloaded"
    mfx_assert_file_contains "$output_file" "\"has_active_manifest_path\":false" "$context manifest target cleared"
    mfx_assert_file_contains "$output_file" "\"has_active_wasm_path\":false" "$context wasm target cleared"
}
