#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_contract_priority_checks() {
    local platform="$1"
    local tmp_dir="$2"
    local base_url="$3"
    local token="$4"
    local expected_process_scope="process:code.exe"
    if [[ "$platform" != "windows" ]]; then
        expected_process_scope="process:code"
    fi

    local payload_priority_scope_specific
    payload_priority_scope_specific='{"process":"code.app","history":["left_click"],"mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["all"],"keys":"Cmd+V"},{"enabled":true,"trigger":"left_click","app_scopes":["process:code.exe"],"keys":"Cmd+C"}]}'
    _mfx_core_http_automation_assert_binding_priority \
        "$base_url" \
        "$token" \
        "$payload_priority_scope_specific" \
        "true" \
        "1" \
        "Cmd+C" \
        "1" \
        "1" \
        "$tmp_dir/scope-priority-specific.out" \
        "core app-scope priority process-over-all"
    mfx_assert_file_contains "$tmp_dir/scope-priority-specific.out" "\"app_scopes_normalized\":[\"$expected_process_scope\"]" "core app-scope priority process-over-all normalized scope"

    local payload_priority_scope_fallback
    payload_priority_scope_fallback='{"process":"safari","history":["left_click"],"mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["all"],"keys":"Cmd+V"},{"enabled":true,"trigger":"left_click","app_scopes":["process:code.exe"],"keys":"Cmd+C"}]}'
    _mfx_core_http_automation_assert_binding_priority \
        "$base_url" \
        "$token" \
        "$payload_priority_scope_fallback" \
        "true" \
        "0" \
        "Cmd+V" \
        "0" \
        "1" \
        "$tmp_dir/scope-priority-fallback.out" \
        "core app-scope priority all-fallback"
    mfx_assert_file_contains "$tmp_dir/scope-priority-fallback.out" "\"app_scopes_normalized\":[\"all\"]" "core app-scope priority all-fallback normalized scope"

    local payload_priority_long_chain
    payload_priority_long_chain='{"process":"code","history":["left_click","left_click"],"mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["process:code.app"],"keys":"Cmd+V"},{"enabled":true,"trigger":"left_click>left_click","app_scopes":["all"],"keys":"Cmd+T"}]}'
    _mfx_core_http_automation_assert_binding_priority \
        "$base_url" \
        "$token" \
        "$payload_priority_long_chain" \
        "true" \
        "1" \
        "Cmd+T" \
        "0" \
        "2" \
        "$tmp_dir/scope-priority-long-chain.out" \
        "core app-scope priority longest-chain-first"
    mfx_assert_file_contains "$tmp_dir/scope-priority-long-chain.out" "\"app_scopes_normalized\":[\"all\"]" "core app-scope priority longest-chain-first normalized scope"

    local payload_priority_scope_alias_tie
    payload_priority_scope_alias_tie='{"process":"code.exe","history":["left_click"],"mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["process:code"],"keys":"Cmd+B"},{"enabled":true,"trigger":"left_click","app_scopes":["process:code.app"],"keys":"Cmd+A"}]}'
    _mfx_core_http_automation_assert_binding_priority \
        "$base_url" \
        "$token" \
        "$payload_priority_scope_alias_tie" \
        "true" \
        "0" \
        "Cmd+B" \
        "1" \
        "1" \
        "$tmp_dir/scope-priority-alias-tie.out" \
        "core app-scope priority alias-tie first-wins"
    mfx_assert_file_contains "$tmp_dir/scope-priority-alias-tie.out" "\"app_scopes_normalized\":[\"$expected_process_scope\"]" "core app-scope priority alias-tie normalized scope"
}
