#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_contract_app_scope_checks() {
    local tmp_dir="$1"
    local base_url="$2"
    local token="$3"

    local code_app_catalog
    code_app_catalog="$(mfx_http_code "$tmp_dir/app-catalog.out" "$base_url/api/automation/app-catalog" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{"force":true}')"
    mfx_assert_eq "$code_app_catalog" "200" "core app-catalog status"
    mfx_assert_file_contains "$tmp_dir/app-catalog.out" "\"ok\":true" "core app-catalog ok"
    mfx_assert_file_contains "$tmp_dir/app-catalog.out" "\"count\":" "core app-catalog count field"

    local code_app_catalog_cached
    code_app_catalog_cached="$(mfx_http_code "$tmp_dir/app-catalog-cached.out" "$base_url/api/automation/app-catalog" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{"force":false}')"
    mfx_assert_eq "$code_app_catalog_cached" "200" "core app-catalog cached status"
    mfx_assert_file_contains "$tmp_dir/app-catalog-cached.out" "\"ok\":true" "core app-catalog cached ok"
    mfx_assert_file_contains "$tmp_dir/app-catalog-cached.out" "\"count\":" "core app-catalog cached count field"

    local selected_process
    selected_process="$(_mfx_core_http_automation_first_catalog_process "$tmp_dir/app-catalog.out")"
    if [[ -n "$selected_process" ]]; then
        local selected_scope="process:$selected_process"
        local selected_scope_escaped
        selected_scope_escaped="$(_mfx_core_http_automation_json_escape "$selected_scope")"

        local code_state_apply_scope
        code_state_apply_scope="$(mfx_http_code "$tmp_dir/state-apply-scope.out" "$base_url/api/state" \
            -X POST \
            -H "x-mfcmouseeffect-token: $token" \
            -H "Content-Type: application/json" \
            -d "{\"automation\":{\"enabled\":true,\"mouse_mappings\":[{\"enabled\":true,\"trigger\":\"left_click\",\"app_scopes\":[\"$selected_scope_escaped\"],\"keys\":\"Cmd+C\"}]}}")"
        mfx_assert_eq "$code_state_apply_scope" "200" "core state apply selected-scope status"
        mfx_assert_file_contains "$tmp_dir/state-apply-scope.out" "\"ok\":true" "core state apply selected-scope ok"

        local code_state_after_scope
        code_state_after_scope="$(mfx_http_code "$tmp_dir/state-after-scope.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
        mfx_assert_eq "$code_state_after_scope" "200" "core state after selected-scope status"
        mfx_assert_file_contains "$tmp_dir/state-after-scope.out" "\"app_scopes\":[\"$selected_scope\"]" "core selected-scope persistence"
    fi

    _mfx_core_http_automation_assert_scope_match "$base_url" "$token" "code" "process:code.exe" "true" "$tmp_dir/scope-code-vs-exe.out" "core app-scope code<->exe"
    _mfx_core_http_automation_assert_scope_match "$base_url" "$token" "code.app" "process:code" "true" "$tmp_dir/scope-app-vs-base.out" "core app-scope app<->base"
    _mfx_core_http_automation_assert_scope_match "$base_url" "$token" "code.exe" "process:code.app" "true" "$tmp_dir/scope-exe-vs-app.out" "core app-scope exe<->app"
    _mfx_core_http_automation_assert_scope_match "$base_url" "$token" "safari" "process:code" "false" "$tmp_dir/scope-negative.out" "core app-scope negative"
}
