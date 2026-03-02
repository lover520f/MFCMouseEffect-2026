#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_parse_first_mapping_scope_count() {
    local input_file="$1"
    python3 - "$input_file" <<'PY'
import json
import sys

with open(sys.argv[1], "r", encoding="utf-8") as f:
    root = json.load(f)

mappings = (
    root.get("automation", {})
        .get("mouse_mappings", [])
)
if not mappings:
    print("0")
    sys.exit(0)

scopes = mappings[0].get("app_scopes", [])
if not isinstance(scopes, list):
    print("0")
    sys.exit(0)

print(len(scopes))
PY
}

_mfx_core_http_automation_parse_first_mapping_scope_value() {
    local input_file="$1"
    python3 - "$input_file" <<'PY'
import json
import sys

with open(sys.argv[1], "r", encoding="utf-8") as f:
    root = json.load(f)

mappings = (
    root.get("automation", {})
        .get("mouse_mappings", [])
)
if not mappings:
    print("")
    sys.exit(0)

scopes = mappings[0].get("app_scopes", [])
if not isinstance(scopes, list) or not scopes:
    print("")
    sys.exit(0)

value = scopes[0]
print(value if isinstance(value, str) else "")
PY
}

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

    local code_state_apply_scope_alias_dedupe
    code_state_apply_scope_alias_dedupe="$(mfx_http_code "$tmp_dir/state-apply-scope-alias-dedupe.out" "$base_url/api/state" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"automation":{"enabled":true,"mouse_mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["process:code.exe","process:code.app","process:code"],"keys":"Cmd+C"}]}}')"
    mfx_assert_eq "$code_state_apply_scope_alias_dedupe" "200" "core state apply app-scope alias dedupe status"

    local code_state_after_scope_alias_dedupe
    code_state_after_scope_alias_dedupe="$(mfx_http_code "$tmp_dir/state-after-scope-alias-dedupe.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_state_after_scope_alias_dedupe" "200" "core state after app-scope alias dedupe status"

    local dedupe_scope_count
    local dedupe_scope_value
    dedupe_scope_count="$(_mfx_core_http_automation_parse_first_mapping_scope_count "$tmp_dir/state-after-scope-alias-dedupe.out")"
    dedupe_scope_value="$(_mfx_core_http_automation_parse_first_mapping_scope_value "$tmp_dir/state-after-scope-alias-dedupe.out")"
    mfx_assert_eq "$dedupe_scope_count" "1" "core app-scope alias dedupe persisted count"
    if [[ -z "$dedupe_scope_value" || ! "$dedupe_scope_value" =~ ^process:code(\.app|\.exe)?$ ]]; then
        mfx_fail "core app-scope alias dedupe persisted value unexpected: ${dedupe_scope_value:-<empty>}"
    fi

    _mfx_core_http_automation_assert_scope_match "$base_url" "$token" "code" "process:code.exe" "true" "$tmp_dir/scope-code-vs-exe.out" "core app-scope code<->exe"
    _mfx_core_http_automation_assert_scope_match "$base_url" "$token" "code.app" "process:code" "true" "$tmp_dir/scope-app-vs-base.out" "core app-scope app<->base"
    _mfx_core_http_automation_assert_scope_match "$base_url" "$token" "code.exe" "process:code.app" "true" "$tmp_dir/scope-exe-vs-app.out" "core app-scope exe<->app"
    _mfx_core_http_automation_assert_scope_match "$base_url" "$token" "safari" "process:code" "false" "$tmp_dir/scope-negative.out" "core app-scope negative"
    mfx_assert_file_contains "$tmp_dir/scope-code-vs-exe.out" "\"process_aliases\":[\"code\",\"code.exe\",\"code.app\"]" "core app-scope process alias matrix code"
    mfx_assert_file_contains "$tmp_dir/scope-code-vs-exe.out" "\"app_scope_alias_matrix\":[{\"aliases\":[\"code.exe\",\"code\"]" "core app-scope scope alias matrix exe"
    mfx_assert_file_contains "$tmp_dir/scope-app-vs-base.out" "\"process_aliases\":[\"code.app\",\"code\"]" "core app-scope process alias matrix code.app"
    mfx_assert_file_contains "$tmp_dir/scope-exe-vs-app.out" "\"process_aliases\":[\"code.exe\",\"code\"]" "core app-scope process alias matrix code.exe"
}
