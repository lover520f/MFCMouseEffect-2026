#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_lib_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$_mfx_core_http_lib_dir/core_http_wasm_helpers.sh"
source "$_mfx_core_http_lib_dir/core_http_wasm_contract_checks.sh"
source "$_mfx_core_http_lib_dir/core_http_input_contract_checks.sh"
source "$_mfx_core_http_lib_dir/core_http_automation_contract_checks.sh"
source "$_mfx_core_http_lib_dir/core_http_probe_helpers.sh"
source "$_mfx_core_http_lib_dir/core_http_entry_helpers.sh"
source "$_mfx_core_http_lib_dir/core_http_state_checks.sh"

_mfx_core_http_entry_pid=""
_mfx_core_http_fifo_path=""
_mfx_core_http_fifo_writer_pid=""
_mfx_core_http_probe_file=""
_mfx_core_http_launch_probe_file=""
_mfx_core_http_launch_capture_file=""
_mfx_core_http_permission_sim_file=""
_mfx_core_http_notification_capture_file=""


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

    _mfx_core_http_run_state_checks \
        "$tmp_dir" \
        "$settings_url" \
        "$base_url" \
        "$token"

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
