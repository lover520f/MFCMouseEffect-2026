#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_lib_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$_mfx_core_http_lib_dir/core_http_wasm_helpers.sh"
source "$_mfx_core_http_lib_dir/core_http_wasm_contract_checks.sh"
source "$_mfx_core_http_lib_dir/core_http_input_contract_checks.sh"
source "$_mfx_core_http_lib_dir/core_http_automation_contract_checks.sh"

_mfx_core_http_entry_pid=""
_mfx_core_http_fifo_path=""
_mfx_core_http_fifo_writer_pid=""
_mfx_core_http_probe_file=""
_mfx_core_http_launch_probe_file=""
_mfx_core_http_launch_capture_file=""
_mfx_core_http_permission_sim_file=""
_mfx_core_http_notification_capture_file=""

_mfx_core_http_probe_value() {
    local key="$1"
    local file_path="$2"
    sed -n "s/^${key}=//p" "$file_path" | head -n 1
}

_mfx_core_http_repo_root() {
    cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd
}

_mfx_core_http_wait_probe_file() {
    local probe_file="$1"
    local timeout_seconds="${MFX_CORE_HTTP_PROBE_TIMEOUT_SECONDS:-8}"
    local deadline=$((SECONDS + timeout_seconds))
    while (( SECONDS < deadline )); do
        if [[ -s "$probe_file" ]]; then
            local probe_url
            local probe_token
            probe_url="$(_mfx_core_http_probe_value "url" "$probe_file")"
            probe_token="$(_mfx_core_http_probe_value "token" "$probe_file")"
            if [[ -n "$probe_url" && -n "$probe_token" ]]; then
                return 0
            fi
        fi
        sleep 0.1
    done
    return 1
}

_mfx_core_http_wait_launch_probe_file() {
    local launch_probe_file="$1"
    local timeout_seconds="${MFX_CORE_HTTP_PROBE_TIMEOUT_SECONDS:-8}"
    local deadline=$((SECONDS + timeout_seconds))
    while (( SECONDS < deadline )); do
        if [[ -s "$launch_probe_file" ]]; then
            local probe_url
            local opened
            probe_url="$(_mfx_core_http_probe_value "url" "$launch_probe_file")"
            opened="$(_mfx_core_http_probe_value "opened" "$launch_probe_file")"
            if [[ -n "$probe_url" && -n "$opened" ]]; then
                return 0
            fi
        fi
        sleep 0.1
    done
    return 1
}

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

mfx_run_core_http_contract_checks() {
    local platform="$1"
    local build_dir="$2"
    local check_scope="${3:-all}"
    local entry_bin="$build_dir/mfx_entry_posix_host"

    if [[ "$check_scope" != "all" && "$check_scope" != "wasm" ]]; then
        mfx_fail "core http check scope must be one of: all, wasm (got: $check_scope)"
    fi

    if [[ ! -x "$entry_bin" ]]; then
        mfx_fail "entry host executable missing: $entry_bin"
    fi

    local tmp_dir
    tmp_dir="$(mktemp -d)"
    local log_file="$tmp_dir/core-http.log"
    local probe_file="$tmp_dir/core-websettings-probe.env"
    local launch_probe_file="$tmp_dir/core-websettings-launch-probe.env"
    local launch_capture_file="$tmp_dir/settings-launch-capture.env"
    local permission_sim_file="$tmp_dir/input-capture-permission.env"
    local notification_capture_file="$tmp_dir/notifications-capture.log"
    trap "_mfx_core_http_stop_entry; rm -rf '$tmp_dir'" EXIT

    local initial_permission_state="0"
    if [[ "$check_scope" == "wasm" ]]; then
        initial_permission_state="1"
    fi
    _mfx_core_http_input_write_permission_sim_state "$permission_sim_file" "$initial_permission_state"
    _mfx_core_http_start_entry \
        "$entry_bin" \
        "$log_file" \
        "$probe_file" \
        "$launch_probe_file" \
        "$launch_capture_file" \
        "$permission_sim_file" \
        "$notification_capture_file"

    local settings_url
    local token
    local base_url
    settings_url="$(_mfx_core_http_probe_value "url" "$probe_file")"
    token="$(_mfx_core_http_probe_value "token" "$probe_file")"
    base_url="${settings_url%%\?*}"
    while [[ "$base_url" == */ ]]; do
        base_url="${base_url%/}"
    done

    if [[ -z "$settings_url" || -z "$token" || -z "$base_url" ]]; then
        mfx_fail "invalid core web settings probe content: $probe_file"
    fi

    if [[ "$check_scope" == "all" ]]; then
        _mfx_core_http_run_input_capture_contract_checks \
            "$platform" \
            "$tmp_dir" \
            "$base_url" \
            "$token" \
            "$settings_url" \
            "$launch_probe_file" \
            "$launch_capture_file" \
            "$permission_sim_file" \
            "$notification_capture_file"
    fi

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

    if [[ "$check_scope" == "all" ]]; then
        _mfx_core_http_run_automation_contract_checks \
            "$platform" \
            "$tmp_dir" \
            "$base_url" \
            "$token"
    fi

    local repo_root
    repo_root="$(_mfx_core_http_repo_root)"
    _mfx_core_http_run_wasm_contract_checks \
        "$platform" \
        "$tmp_dir" \
        "$base_url" \
        "$token" \
        "$repo_root"

    trap - EXIT
    _mfx_core_http_stop_entry
    rm -rf "$tmp_dir"
    mfx_ok "core HTTP contract checks completed (scope=$check_scope)"
}
