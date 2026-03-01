#!/usr/bin/env bash

set -euo pipefail

_mfx_http_lib_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$_mfx_http_lib_dir/http_entry_helpers.sh"

_mfx_http_default_route_skipped=0
_mfx_http_route_skip_reason=""

_mfx_http_skip_bind_eacces_enabled() {
    local raw="${MFX_HTTP_SKIP_BIND_EACCES:-1}"
    case "$raw" in
        0|false|FALSE|False|no|NO|off|OFF)
            return 1
            ;;
        *)
            return 0
            ;;
    esac
}

_mfx_http_should_skip_due_bind_eacces() {
    local log_file="$1"
    if ! _mfx_http_skip_bind_eacces_enabled; then
        return 1
    fi
    [[ -f "$log_file" ]] || return 1
    rg -q "Scaffold settings server failed to start \\(stage=2,code=(1|13)\\)" "$log_file"
}

_mfx_http_default_route_checks() {
    local platform="$1"
    local entry_bin="$2"
    _mfx_http_default_route_skipped=0

    local tmp_dir
    tmp_dir="$(mktemp -d)"
    local log_file="$tmp_dir/default.log"
    trap "_mfx_http_stop_entry; rm -rf '$tmp_dir'" EXIT

    local start_status=0
    _mfx_http_start_entry "$entry_bin" "$log_file" || start_status=$?
    if [[ "$start_status" -eq 2 ]]; then
        _mfx_http_route_skip_reason="$_mfx_http_startup_skip_reason"
        mfx_info "skip scaffold HTTP checks (default route): ${_mfx_http_route_skip_reason:-constrained runtime startup skip}"
        _mfx_http_default_route_skipped=1
        trap - EXIT
        _mfx_http_stop_entry
        rm -rf "$tmp_dir"
        return 0
    fi
    mfx_assert_eq "$start_status" "0" "default route startup status"
    local base_url="http://127.0.0.1:9527"

    local code_root
    code_root="$(mfx_http_code "$tmp_dir/root.out" "$base_url/?token=scaffold")"
    if [[ "$code_root" != "200" ]] && _mfx_http_should_skip_due_bind_eacces "$log_file"; then
        mfx_info "skip scaffold HTTP checks: loopback bind denied (EPERM/EACCES stage=2 code=1|13)"
        _mfx_http_default_route_skipped=1
        trap - EXIT
        _mfx_http_stop_entry
        rm -rf "$tmp_dir"
        return 0
    fi
    mfx_assert_eq "$code_root" "200" "default root status"

    local code_js
    code_js="$(mfx_http_code "$tmp_dir/js.out" "$base_url/settings-shell.svelte.js?token=scaffold")"
    mfx_assert_eq "$code_js" "200" "default js status"

    local code_schema
    code_schema="$(mfx_http_code "$tmp_dir/schema.out" "$base_url/api/schema?token=scaffold")"
    mfx_assert_eq "$code_schema" "200" "default schema status"

    local code_state
    code_state="$(mfx_http_code "$tmp_dir/state.out" "$base_url/api/state?token=scaffold")"
    mfx_assert_eq "$code_state" "200" "default state status"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"platform\":\"$platform\"" "default state platform"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"runtime_mode\":\"background\"" "default state runtime_mode"

    local code_post
    code_post="$(mfx_http_code "$tmp_dir/post.out" "$base_url/api/state?token=scaffold" -X POST -H 'Content-Type: application/json' -d '{"ui_language":"en-US"}')"
    mfx_assert_eq "$code_post" "200" "default state post status"
    mfx_assert_file_contains "$tmp_dir/post.out" "\"ui_language\":\"en-US\"" "default state post ui_language"

    local code_active_process
    code_active_process="$(mfx_http_code "$tmp_dir/active-process.out" "$base_url/api/automation/active-process?token=scaffold" -X POST -H 'Content-Type: application/json' -d '{}')"
    mfx_assert_eq "$code_active_process" "404" "default active-process unsupported status"
    mfx_assert_file_contains "$tmp_dir/active-process.out" "not found" "default active-process unsupported body"

    local code_app_catalog
    code_app_catalog="$(mfx_http_code "$tmp_dir/app-catalog.out" "$base_url/api/automation/app-catalog?token=scaffold" -X POST -H 'Content-Type: application/json' -d '{"force":true}')"
    mfx_assert_eq "$code_app_catalog" "404" "default app-catalog unsupported status"
    mfx_assert_file_contains "$tmp_dir/app-catalog.out" "not found" "default app-catalog unsupported body"

    local code_bad_token
    code_bad_token="$(mfx_http_code "$tmp_dir/bad-token.out" "$base_url/settings-shell.svelte.js?token=bad")"
    mfx_assert_eq "$code_bad_token" "403" "default bad token status"

    trap - EXIT
    _mfx_http_stop_entry
    rm -rf "$tmp_dir"
}

_mfx_http_custom_route_checks() {
    local entry_bin="$1"

    local tmp_dir
    tmp_dir="$(mktemp -d)"
    local log_file="$tmp_dir/custom.log"
    trap "_mfx_http_stop_entry; rm -rf '$tmp_dir'" EXIT

    local start_status=0
    _mfx_http_start_entry "$entry_bin" "$log_file" "MFX_SCAFFOLD_SETTINGS_URL=http://127.0.0.1:18127/ui/settings?token=dev" || start_status=$?
    if [[ "$start_status" -eq 2 ]]; then
        _mfx_http_route_skip_reason="$_mfx_http_startup_skip_reason"
        mfx_info "skip scaffold HTTP checks (custom route): ${_mfx_http_route_skip_reason:-constrained runtime startup skip}"
        trap - EXIT
        _mfx_http_stop_entry
        rm -rf "$tmp_dir"
        return 2
    fi
    mfx_assert_eq "$start_status" "0" "custom route startup status"
    local base_url="http://127.0.0.1:18127"

    local code_root
    code_root="$(mfx_http_code "$tmp_dir/root.out" "$base_url/ui/settings?token=dev")"
    mfx_assert_eq "$code_root" "200" "custom root status"

    local code_js
    code_js="$(mfx_http_code "$tmp_dir/js.out" "$base_url/settings-shell.svelte.js?token=dev")"
    mfx_assert_eq "$code_js" "200" "custom js status"

    local code_post
    code_post="$(mfx_http_code "$tmp_dir/post.out" "$base_url/api/state?token=dev" -X POST -H 'Content-Type: application/json' -d '{"hold_follow_mode":"smooth"}')"
    mfx_assert_eq "$code_post" "200" "custom state post status"
    mfx_assert_file_contains "$tmp_dir/post.out" "\"entry_path\":\"/ui/settings\"" "custom state entry_path"
    mfx_assert_file_contains "$tmp_dir/post.out" "\"hold_follow_mode\":\"smooth\"" "custom state hold_follow_mode"

    local code_bad_token
    code_bad_token="$(mfx_http_code "$tmp_dir/bad-token.out" "$base_url/ui/settings?token=bad")"
    mfx_assert_eq "$code_bad_token" "403" "custom bad token status"

    trap - EXIT
    _mfx_http_stop_entry
    rm -rf "$tmp_dir"
}

_mfx_http_missing_webui_checks() {
    local entry_bin="$1"

    local tmp_dir
    tmp_dir="$(mktemp -d)"
    local empty_webui_dir="$tmp_dir/empty-webui"
    mkdir -p "$empty_webui_dir"
    local log_file="$tmp_dir/missing-webui.log"
    trap "_mfx_http_stop_entry; rm -rf '$tmp_dir'" EXIT

    local start_status=0
    _mfx_http_start_entry "$entry_bin" "$log_file" "MFX_SCAFFOLD_WEBUI_DIR=$empty_webui_dir" || start_status=$?
    if [[ "$start_status" -eq 2 ]]; then
        _mfx_http_route_skip_reason="$_mfx_http_startup_skip_reason"
        mfx_info "skip scaffold HTTP checks (missing-webui route): ${_mfx_http_route_skip_reason:-constrained runtime startup skip}"
        trap - EXIT
        _mfx_http_stop_entry
        rm -rf "$tmp_dir"
        return 2
    fi
    mfx_assert_eq "$start_status" "0" "missing-webui route startup status"
    local base_url="http://127.0.0.1:9527"

    local code_root
    code_root="$(mfx_http_code "$tmp_dir/root.out" "$base_url/?token=scaffold")"
    mfx_assert_eq "$code_root" "503" "missing webui root status"
    mfx_assert_file_contains "$tmp_dir/root.out" "Scaffold WebUI assets not found." "missing webui root message"

    local code_js
    code_js="$(mfx_http_code "$tmp_dir/js.out" "$base_url/settings-shell.svelte.js?token=scaffold")"
    mfx_assert_eq "$code_js" "404" "missing webui js status"

    trap - EXIT
    _mfx_http_stop_entry
    rm -rf "$tmp_dir"
}

mfx_run_http_checks() {
    local platform="$1"
    local build_dir="$2"
    local entry_bin="$build_dir/mfx_entry_posix_host"

    if [[ ! -x "$entry_bin" ]]; then
        mfx_fail "entry host executable missing: $entry_bin"
    fi

    mfx_info "run scaffold HTTP checks: default route"
    _mfx_http_default_route_checks "$platform" "$entry_bin"
    if [[ "$_mfx_http_default_route_skipped" -eq 1 ]]; then
        mfx_ok "HTTP checks skipped: ${_mfx_http_route_skip_reason:-loopback bind permission denial}"
        return 0
    fi

    mfx_info "run scaffold HTTP checks: custom route"
    local route_status=0
    _mfx_http_custom_route_checks "$entry_bin" || route_status=$?
    if [[ "$route_status" -eq 2 ]]; then
        mfx_ok "HTTP checks skipped: ${_mfx_http_route_skip_reason:-constrained runtime startup skip}"
        return 0
    fi
    mfx_assert_eq "$route_status" "0" "custom route check status"

    mfx_info "run scaffold HTTP checks: missing webui"
    route_status=0
    _mfx_http_missing_webui_checks "$entry_bin" || route_status=$?
    if [[ "$route_status" -eq 2 ]]; then
        mfx_ok "HTTP checks skipped: ${_mfx_http_route_skip_reason:-constrained runtime startup skip}"
        return 0
    fi
    mfx_assert_eq "$route_status" "0" "missing-webui route check status"

    mfx_ok "HTTP checks completed"
}
