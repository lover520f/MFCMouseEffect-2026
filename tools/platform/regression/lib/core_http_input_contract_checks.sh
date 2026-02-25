#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_input_probe_value() {
    local key="$1"
    local file_path="$2"
    sed -n "s/^${key}=//p" "$file_path" | head -n 1
}

_mfx_core_http_input_write_permission_sim_state() {
    local file_path="$1"
    local trusted="$2"
    printf 'trusted=%s\n' "$trusted" >"$file_path"
}

_mfx_core_http_wait_input_capture_state() {
    local base_url="$1"
    local token="$2"
    local expected_active="$3"
    local expected_reason="$4"
    local output_file="$5"
    local context="$6"
    local timeout_seconds="${MFX_CORE_HTTP_INPUT_CAPTURE_TIMEOUT_SECONDS:-10}"
    local deadline=$((SECONDS + timeout_seconds))

    while (( SECONDS < deadline )); do
        local code
        code="$(mfx_http_code "$output_file" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
        if [[ "$code" == "200" ]] && \
           rg -q --fixed-strings "\"input_capture\"" "$output_file" && \
           rg -q --fixed-strings "\"active\":$expected_active" "$output_file" && \
           rg -q --fixed-strings "\"reason\":\"$expected_reason\"" "$output_file"; then
            return 0
        fi
        sleep 0.2
    done

    mfx_fail "$context timeout: expected input_capture.active=$expected_active reason=$expected_reason"
}

_mfx_core_http_notification_count() {
    local file_path="$1"
    if [[ ! -f "$file_path" ]]; then
        echo "0"
        return
    fi
    wc -l <"$file_path" | tr -d ' '
}

_mfx_core_http_run_input_capture_contract_checks() {
    local platform="$1"
    local tmp_dir="$2"
    local base_url="$3"
    local token="$4"
    local settings_url="$5"
    local launch_probe_file="$6"
    local launch_capture_file="$7"
    local permission_sim_file="$8"
    local notification_capture_file="$9"

    _mfx_core_http_wait_input_capture_state \
        "$base_url" \
        "$token" \
        "false" \
        "permission_denied" \
        "$tmp_dir/input-capture-startup-denied.out" \
        "core input-capture startup denied"
    mfx_assert_file_contains "$tmp_dir/input-capture-startup-denied.out" "\"effects_suspended\":true" "core input-capture startup denied effects suspended"

    sleep 1.2
    local startup_notification_count
    startup_notification_count="$(_mfx_core_http_notification_count "$notification_capture_file")"
    mfx_assert_eq "$startup_notification_count" "1" "core startup degraded notification dedup"

    _mfx_core_http_input_write_permission_sim_state "$permission_sim_file" "1"
    _mfx_core_http_wait_input_capture_state \
        "$base_url" \
        "$token" \
        "true" \
        "none" \
        "$tmp_dir/input-capture-startup-recovered.out" \
        "core input-capture startup recovery"
    mfx_assert_file_contains "$tmp_dir/input-capture-startup-recovered.out" "\"effects_suspended\":false" "core input-capture startup recovery effects resumed"

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

    _mfx_core_http_input_write_permission_sim_state "$permission_sim_file" "0"
    _mfx_core_http_wait_input_capture_state \
        "$base_url" \
        "$token" \
        "false" \
        "permission_denied" \
        "$tmp_dir/input-capture-runtime-revoke.out" \
        "core input-capture runtime revoke"
    mfx_assert_file_contains "$tmp_dir/input-capture-runtime-revoke.out" "\"effects_suspended\":true" "core input-capture runtime revoke effects suspended"

    _mfx_core_http_input_write_permission_sim_state "$permission_sim_file" "1"
    _mfx_core_http_wait_input_capture_state \
        "$base_url" \
        "$token" \
        "true" \
        "none" \
        "$tmp_dir/input-capture-runtime-regrant.out" \
        "core input-capture runtime regrant"
    mfx_assert_file_contains "$tmp_dir/input-capture-runtime-regrant.out" "\"effects_suspended\":false" "core input-capture runtime regrant effects resumed"
}
