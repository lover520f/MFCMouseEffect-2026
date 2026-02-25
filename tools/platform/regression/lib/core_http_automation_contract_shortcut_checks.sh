#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_contract_shortcut_checks() {
    local tmp_dir="$1"
    local base_url="$2"
    local token="$3"

    local code_shortcut_map_cmd_v
    code_shortcut_map_cmd_v="$(mfx_http_code "$tmp_dir/shortcut-map-cmd-v.out" "$base_url/api/automation/test-shortcut-from-mac-keycode" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"mac_key_code":9,"cmd":true}')"
    mfx_assert_eq "$code_shortcut_map_cmd_v" "200" "core shortcut-map cmd+v status"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-v.out" "\"ok\":true" "core shortcut-map cmd+v ok"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-v.out" "\"supported\":true" "core shortcut-map cmd+v supported"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-v.out" "\"vk_code\":86" "core shortcut-map cmd+v vk"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-v.out" "\"shortcut\":\"Win+V\"" "core shortcut-map cmd+v text"

    local code_shortcut_map_cmd_tab
    code_shortcut_map_cmd_tab="$(mfx_http_code "$tmp_dir/shortcut-map-cmd-tab.out" "$base_url/api/automation/test-shortcut-from-mac-keycode" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"mac_key_code":48,"cmd":true}')"
    mfx_assert_eq "$code_shortcut_map_cmd_tab" "200" "core shortcut-map cmd+tab status"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-tab.out" "\"ok\":true" "core shortcut-map cmd+tab ok"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-tab.out" "\"supported\":true" "core shortcut-map cmd+tab supported"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-tab.out" "\"vk_code\":9" "core shortcut-map cmd+tab vk"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-tab.out" "\"shortcut\":\"Win+Tab\"" "core shortcut-map cmd+tab text"

    local code_shortcut_map_invalid
    code_shortcut_map_invalid="$(mfx_http_code "$tmp_dir/shortcut-map-invalid.out" "$base_url/api/automation/test-shortcut-from-mac-keycode" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"mac_key_code":-1,"cmd":true}')"
    mfx_assert_eq "$code_shortcut_map_invalid" "200" "core shortcut-map invalid status"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-invalid.out" "\"ok\":true" "core shortcut-map invalid ok"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-invalid.out" "\"supported\":false" "core shortcut-map invalid supported flag"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-invalid.out" "\"vk_code\":0" "core shortcut-map invalid vk"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-invalid.out" "\"reason\":\"invalid_or_unmapped_keycode\"" "core shortcut-map invalid reason"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-invalid.out" "\"shortcut\":\"\"" "core shortcut-map invalid shortcut empty"
}
