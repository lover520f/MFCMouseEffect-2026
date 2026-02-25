#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_input_assert_settings_launch() {
    local platform="$1"
    local settings_url="$2"
    local launch_probe_file="$3"
    local launch_capture_file="$4"

    local launch_probe_url
    local launch_probe_opened
    launch_probe_url="$(_mfx_core_http_input_probe_value "url" "$launch_probe_file")"
    launch_probe_opened="$(_mfx_core_http_input_probe_value "opened" "$launch_probe_file")"
    mfx_assert_eq "$launch_probe_opened" "1" "core settings launch probe opened"
    mfx_assert_eq "$launch_probe_url" "$settings_url" "core settings launch probe url"

    local expected_launcher_command="open"
    if [[ "$platform" == "linux" ]]; then
        expected_launcher_command="xdg-open"
    fi
    mfx_assert_file_contains "$launch_capture_file" "captured=1" "core settings launcher capture flag"
    mfx_assert_file_contains "$launch_capture_file" "command=$expected_launcher_command" "core settings launcher command"
    mfx_assert_file_contains "$launch_capture_file" "url=$settings_url" "core settings launcher url"
}

_mfx_core_http_input_assert_capture_transition() {
    local base_url="$1"
    local token="$2"
    local expected_active="$3"
    local expected_reason="$4"
    local output_file="$5"
    local context="$6"
    local expected_suspended="$7"

    _mfx_core_http_wait_input_capture_state \
        "$base_url" \
        "$token" \
        "$expected_active" \
        "$expected_reason" \
        "$output_file" \
        "$context"
    mfx_assert_file_contains "$output_file" "\"effects_suspended\":$expected_suspended" "$context effects suspended"
}
