#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_contract_priority_checks() {
    local tmp_dir="$1"
    local base_url="$2"
    local token="$3"

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
}
