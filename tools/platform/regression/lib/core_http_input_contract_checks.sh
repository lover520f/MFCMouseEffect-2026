#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_input_contract_checks_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$_mfx_core_http_input_contract_checks_dir/core_http_input_helpers.sh"

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

    _mfx_core_http_input_assert_capture_transition \
        "$base_url" \
        "$token" \
        "false" \
        "permission_denied" \
        "$tmp_dir/input-capture-startup-denied.out" \
        "core input-capture startup denied" \
        "true"

    sleep 1.2
    local startup_notification_count
    startup_notification_count="$(_mfx_core_http_notification_count "$notification_capture_file")"
    mfx_assert_eq "$startup_notification_count" "1" "core startup degraded notification dedup"

    _mfx_core_http_input_write_permission_sim_state "$permission_sim_file" "1"
    _mfx_core_http_input_assert_capture_transition \
        "$base_url" \
        "$token" \
        "true" \
        "none" \
        "$tmp_dir/input-capture-startup-recovered.out" \
        "core input-capture startup recovery" \
        "false"

    _mfx_core_http_input_assert_settings_launch \
        "$platform" \
        "$settings_url" \
        "$launch_probe_file" \
        "$launch_capture_file"

    _mfx_core_http_input_write_permission_sim_state "$permission_sim_file" "0"
    _mfx_core_http_input_assert_capture_transition \
        "$base_url" \
        "$token" \
        "false" \
        "permission_denied" \
        "$tmp_dir/input-capture-runtime-revoke.out" \
        "core input-capture runtime revoke" \
        "true"

    _mfx_core_http_input_write_permission_sim_state "$permission_sim_file" "1"
    _mfx_core_http_input_assert_capture_transition \
        "$base_url" \
        "$token" \
        "true" \
        "none" \
        "$tmp_dir/input-capture-runtime-regrant.out" \
        "core input-capture runtime regrant" \
        "false"
}
