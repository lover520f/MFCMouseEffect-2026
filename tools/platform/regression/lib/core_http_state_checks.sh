#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_run_state_checks() {
    local tmp_dir="$1"
    local settings_url="$2"
    local base_url="$3"
    local token="$4"

    local code_root
    code_root="$(mfx_http_code "$tmp_dir/root.out" "$settings_url")"
    mfx_assert_eq "$code_root" "200" "core root status"

    local code_js
    code_js="$(mfx_http_code "$tmp_dir/settings-js.out" "$base_url/settings-shell.svelte.js?token=$token")"
    mfx_assert_eq "$code_js" "200" "core settings-shell js status"

    local code_state
    code_state="$(mfx_http_code "$tmp_dir/state.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_state" "200" "core state status"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"automation\":" "core state automation section"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"input_capture\":" "core state input_capture section"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"wasm\":" "core state wasm section"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"effects_runtime\":" "core state effects_runtime section"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"click_active_overlay_windows\":" "core effects runtime click overlay count"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"trail_active_overlay_windows\":" "core effects runtime trail overlay count"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"scroll_active_overlay_windows\":" "core effects runtime scroll overlay count"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"hold_active_overlay_windows\":" "core effects runtime hold overlay count"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"hover_active_overlay_windows\":" "core effects runtime hover overlay count"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"active_overlay_windows_total\":" "core effects runtime total overlay count"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"invoke_supported\":" "core wasm invoke capability"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"render_supported\":" "core wasm render capability"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"last_throttled_render_commands\":" "core wasm throttled render diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"last_throttled_by_capacity_render_commands\":" "core wasm throttled-by-capacity diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"last_throttled_by_interval_render_commands\":" "core wasm throttled-by-interval diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"last_load_failure_stage\":" "core wasm load-failure stage diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"last_load_failure_code\":" "core wasm load-failure code diagnostics"

    local code_state_unauthorized
    code_state_unauthorized="$(mfx_http_code "$tmp_dir/state-unauth.out" "$base_url/api/state")"
    mfx_assert_eq "$code_state_unauthorized" "401" "core state unauthorized status"

    local code_schema
    code_schema="$(mfx_http_code "$tmp_dir/schema.out" "$base_url/api/schema" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_schema" "200" "core schema status"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"capabilities\":" "core schema capabilities section"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"wasm\":" "core schema wasm capabilities section"
}
