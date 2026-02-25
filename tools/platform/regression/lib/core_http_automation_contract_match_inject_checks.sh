#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_contract_match_inject_checks() {
    local tmp_dir="$1"
    local base_url="$2"
    local token="$3"

    local code_match_and_inject
    code_match_and_inject="$(mfx_http_code "$tmp_dir/match-and-inject.out" "$base_url/api/automation/test-match-and-inject" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"process":"code.app","history":["left_click"],"mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["all"],"keys":"Cmd+C"}]}')"
    mfx_assert_eq "$code_match_and_inject" "200" "core test-match-and-inject status"
    mfx_assert_file_contains "$tmp_dir/match-and-inject.out" "\"ok\":true" "core test-match-and-inject ok"
    mfx_assert_file_contains "$tmp_dir/match-and-inject.out" "\"matched\":true" "core test-match-and-inject matched"
    mfx_assert_file_contains "$tmp_dir/match-and-inject.out" "\"injected\":true" "core test-match-and-inject injected"
    mfx_assert_file_contains "$tmp_dir/match-and-inject.out" "\"selected_keys\":\"Cmd+C\"" "core test-match-and-inject selected keys"
}
