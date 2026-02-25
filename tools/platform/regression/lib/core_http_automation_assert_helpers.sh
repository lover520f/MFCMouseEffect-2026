#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_assert_scope_match() {
    local base_url="$1"
    local token="$2"
    local process_name="$3"
    local scope_value="$4"
    local expected_matched="$5"
    local output_file="$6"
    local context="$7"

    local process_escaped
    local scope_escaped
    process_escaped="$(_mfx_core_http_automation_json_escape "$process_name")"
    scope_escaped="$(_mfx_core_http_automation_json_escape "$scope_value")"

    local code
    code="$(mfx_http_code "$output_file" "$base_url/api/automation/test-app-scope-match" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "{\"process\":\"$process_escaped\",\"app_scopes\":[\"$scope_escaped\"]}")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"matched\":$expected_matched" "$context matched"
}

_mfx_core_http_automation_assert_binding_priority() {
    local base_url="$1"
    local token="$2"
    local payload="$3"
    local expected_matched="$4"
    local expected_index="$5"
    local expected_keys="$6"
    local expected_scope_specificity="$7"
    local expected_chain_length="$8"
    local output_file="$9"
    local context="${10}"

    local code
    code="$(mfx_http_code "$output_file" "$base_url/api/automation/test-binding-priority" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "$payload")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"matched\":$expected_matched" "$context matched"
    mfx_assert_file_contains "$output_file" "\"selected_binding_index\":$expected_index" "$context selected index"
    mfx_assert_file_contains "$output_file" "\"selected_scope_specificity\":$expected_scope_specificity" "$context selected scope specificity"
    mfx_assert_file_contains "$output_file" "\"selected_chain_length\":$expected_chain_length" "$context selected chain length"
    if [[ "$expected_matched" == "true" ]]; then
        local expected_keys_escaped
        expected_keys_escaped="$(_mfx_core_http_automation_json_escape "$expected_keys")"
        mfx_assert_file_contains "$output_file" "\"selected_keys\":\"$expected_keys_escaped\"" "$context selected keys"
    fi
}
