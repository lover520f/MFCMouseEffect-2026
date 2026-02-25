#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_contract_basic_checks() {
    local tmp_dir="$1"
    local base_url="$2"
    local token="$3"

    local code_active_process
    code_active_process="$(mfx_http_code "$tmp_dir/active-process.out" "$base_url/api/automation/active-process" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{}')"
    mfx_assert_eq "$code_active_process" "200" "core active-process status"
    mfx_assert_file_contains "$tmp_dir/active-process.out" "\"ok\":true" "core active-process ok"
    mfx_assert_file_contains "$tmp_dir/active-process.out" "\"process\":\"" "core active-process process field"

    local code_test_inject_shortcut
    code_test_inject_shortcut="$(mfx_http_code "$tmp_dir/test-inject-shortcut.out" "$base_url/api/automation/test-inject-shortcut" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"keys":"Cmd+C"}')"
    mfx_assert_eq "$code_test_inject_shortcut" "200" "core test-inject-shortcut status"
    mfx_assert_file_contains "$tmp_dir/test-inject-shortcut.out" "\"ok\":true" "core test-inject-shortcut ok"
    mfx_assert_file_contains "$tmp_dir/test-inject-shortcut.out" "\"keys\":\"Cmd+C\"" "core test-inject-shortcut keys echo"
}
