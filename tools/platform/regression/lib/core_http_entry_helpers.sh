#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_start_entry() {
    local entry_bin="$1"
    local log_file="$2"
    local probe_file="$3"
    local launch_probe_file="$4"
    local launch_capture_file="$5"
    local permission_sim_file="$6"
    local notification_capture_file="$7"

    _mfx_core_http_probe_file="$probe_file"
    _mfx_core_http_launch_probe_file="$launch_probe_file"
    _mfx_core_http_launch_capture_file="$launch_capture_file"
    _mfx_core_http_permission_sim_file="$permission_sim_file"
    _mfx_core_http_notification_capture_file="$notification_capture_file"
    rm -f "$probe_file" "${probe_file}.tmp" || true
    rm -f "$launch_probe_file" "${launch_probe_file}.tmp" || true
    rm -f "$launch_capture_file" "${launch_capture_file}.tmp" || true
    rm -f "$notification_capture_file" "${notification_capture_file}.tmp" || true

    _mfx_core_http_fifo_path="$(mktemp -u "/tmp/mfx-posix-core-http-fifo.XXXXXX")"
    mkfifo "$_mfx_core_http_fifo_path"

    tail -f /dev/null >"$_mfx_core_http_fifo_path" &
    _mfx_core_http_fifo_writer_pid="$!"

    MFX_CORE_WEB_SETTINGS_PROBE_FILE="$probe_file" \
    MFX_CORE_WEB_SETTINGS_LAUNCH_PROBE_FILE="$launch_probe_file" \
    MFX_TEST_SETTINGS_LAUNCH_CAPTURE_FILE="$launch_capture_file" \
    MFX_TEST_INPUT_CAPTURE_PERMISSION_SIM_FILE="$permission_sim_file" \
    MFX_TEST_NOTIFICATION_CAPTURE_FILE="$notification_capture_file" \
    MFX_ENABLE_AUTOMATION_SCOPE_TEST_API="${MFX_ENABLE_AUTOMATION_SCOPE_TEST_API:-1}" \
    MFX_ENABLE_AUTOMATION_SHORTCUT_TEST_API="${MFX_ENABLE_AUTOMATION_SHORTCUT_TEST_API:-1}" \
    MFX_ENABLE_AUTOMATION_INJECTION_TEST_API="${MFX_ENABLE_AUTOMATION_INJECTION_TEST_API:-1}" \
    MFX_ENABLE_EFFECT_OVERLAY_TEST_API="${MFX_ENABLE_EFFECT_OVERLAY_TEST_API:-1}" \
    MFX_ENABLE_INPUT_INDICATOR_TEST_API="${MFX_ENABLE_INPUT_INDICATOR_TEST_API:-1}" \
    MFX_TEST_KEYBOARD_INJECTOR_DRY_RUN="${MFX_TEST_KEYBOARD_INJECTOR_DRY_RUN:-1}" \
    MFX_ENABLE_WASM_TEST_DISPATCH_API="${MFX_ENABLE_WASM_TEST_DISPATCH_API:-1}" \
        "$entry_bin" -mode=background <"$_mfx_core_http_fifo_path" >"$log_file" 2>&1 &
    _mfx_core_http_entry_pid="$!"

    sleep "${MFX_CORE_HTTP_START_WAIT_SECONDS:-1}"
    if ! kill -0 "$_mfx_core_http_entry_pid" >/dev/null 2>&1; then
        mfx_info "core http startup log:"
        cat "$log_file" || true
        mfx_fail "core http entry exited before HTTP checks"
    fi

    if ! _mfx_core_http_wait_probe_file "$probe_file"; then
        mfx_info "core http startup log:"
        cat "$log_file" || true
        mfx_fail "core web settings probe file not ready: $probe_file"
    fi
    if ! _mfx_core_http_wait_launch_probe_file "$launch_probe_file"; then
        mfx_info "core http startup log:"
        cat "$log_file" || true
        mfx_fail "core web settings launch probe file not ready: $launch_probe_file"
    fi
}

_mfx_core_http_stop_entry() {
    if [[ -n "$_mfx_core_http_fifo_path" && -p "$_mfx_core_http_fifo_path" ]]; then
        printf 'exit\n' >"$_mfx_core_http_fifo_path" || true
    fi

    if [[ -n "$_mfx_core_http_entry_pid" ]]; then
        wait "$_mfx_core_http_entry_pid" >/dev/null 2>&1 || true
    fi

    if [[ -n "$_mfx_core_http_fifo_writer_pid" ]]; then
        kill "$_mfx_core_http_fifo_writer_pid" >/dev/null 2>&1 || true
        wait "$_mfx_core_http_fifo_writer_pid" >/dev/null 2>&1 || true
    fi

    if [[ -n "$_mfx_core_http_fifo_path" ]]; then
        rm -f "$_mfx_core_http_fifo_path"
    fi
    if [[ -n "$_mfx_core_http_probe_file" ]]; then
        rm -f "$_mfx_core_http_probe_file" "${_mfx_core_http_probe_file}.tmp" || true
    fi
    if [[ -n "$_mfx_core_http_launch_probe_file" ]]; then
        rm -f "$_mfx_core_http_launch_probe_file" "${_mfx_core_http_launch_probe_file}.tmp" || true
    fi
    if [[ -n "$_mfx_core_http_launch_capture_file" ]]; then
        rm -f "$_mfx_core_http_launch_capture_file" "${_mfx_core_http_launch_capture_file}.tmp" || true
    fi
    if [[ -n "$_mfx_core_http_permission_sim_file" ]]; then
        rm -f "$_mfx_core_http_permission_sim_file" "${_mfx_core_http_permission_sim_file}.tmp" || true
    fi
    if [[ -n "$_mfx_core_http_notification_capture_file" ]]; then
        rm -f "$_mfx_core_http_notification_capture_file" "${_mfx_core_http_notification_capture_file}.tmp" || true
    fi

    _mfx_core_http_entry_pid=""
    _mfx_core_http_fifo_path=""
    _mfx_core_http_fifo_writer_pid=""
    _mfx_core_http_probe_file=""
    _mfx_core_http_launch_probe_file=""
    _mfx_core_http_launch_capture_file=""
    _mfx_core_http_permission_sim_file=""
    _mfx_core_http_notification_capture_file=""
}
