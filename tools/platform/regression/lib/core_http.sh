#!/usr/bin/env bash

set -euo pipefail

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

_mfx_core_http_json_escape() {
    local value="$1"
    printf '%s' "$value" | sed 's/\\/\\\\/g; s/"/\\"/g'
}

_mfx_core_http_first_catalog_process() {
    local file_path="$1"
    sed -n 's/.*"exe":"\([^"]*\)".*/\1/p' "$file_path" | head -n 1
}

_mfx_core_http_assert_scope_match() {
    local base_url="$1"
    local token="$2"
    local process_name="$3"
    local scope_value="$4"
    local expected_matched="$5"
    local output_file="$6"
    local context="$7"

    local process_escaped
    local scope_escaped
    process_escaped="$(_mfx_core_http_json_escape "$process_name")"
    scope_escaped="$(_mfx_core_http_json_escape "$scope_value")"

    local code
    code="$(mfx_http_code "$output_file" "$base_url/api/automation/test-app-scope-match" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "{\"process\":\"$process_escaped\",\"app_scopes\":[\"$scope_escaped\"]}")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"matched\":$expected_matched" "$context matched"
}

_mfx_core_http_assert_binding_priority() {
    local base_url="$1"
    local token="$2"
    local payload="$3"
    local expected_matched="$4"
    local expected_index="$5"
    local expected_keys="$6"
    local expected_scope_specificity="$7"
    local expected_chain_length="$8"
    local output_file="$9"
    local context="${10}"

    local code
    code="$(mfx_http_code "$output_file" "$base_url/api/automation/test-binding-priority" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "$payload")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"matched\":$expected_matched" "$context matched"
    mfx_assert_file_contains "$output_file" "\"selected_binding_index\":$expected_index" "$context selected index"
    mfx_assert_file_contains "$output_file" "\"selected_scope_specificity\":$expected_scope_specificity" "$context selected scope specificity"
    mfx_assert_file_contains "$output_file" "\"selected_chain_length\":$expected_chain_length" "$context selected chain length"
    if [[ "$expected_matched" == "true" ]]; then
        local expected_keys_escaped
        expected_keys_escaped="$(_mfx_core_http_json_escape "$expected_keys")"
        mfx_assert_file_contains "$output_file" "\"selected_keys\":\"$expected_keys_escaped\"" "$context selected keys"
    fi
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

_mfx_core_http_write_permission_sim_state() {
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
    local entry_bin="$build_dir/mfx_entry_posix_host"

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

    _mfx_core_http_write_permission_sim_state "$permission_sim_file" "0"
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

    _mfx_core_http_wait_input_capture_state \
        "$base_url" \
        "$token" \
        "false" \
        "permission_denied" \
        "$tmp_dir/input-capture-startup-denied.out" \
        "core input-capture startup denied"

    sleep 1.2
    local startup_notification_count
    startup_notification_count="$(_mfx_core_http_notification_count "$notification_capture_file")"
    mfx_assert_eq "$startup_notification_count" "1" "core startup degraded notification dedup"

    _mfx_core_http_write_permission_sim_state "$permission_sim_file" "1"
    _mfx_core_http_wait_input_capture_state \
        "$base_url" \
        "$token" \
        "true" \
        "none" \
        "$tmp_dir/input-capture-startup-recovered.out" \
        "core input-capture startup recovery"

    local launch_probe_url
    local launch_probe_opened
    launch_probe_url="$(_mfx_core_http_probe_value "url" "$launch_probe_file")"
    launch_probe_opened="$(_mfx_core_http_probe_value "opened" "$launch_probe_file")"
    mfx_assert_eq "$launch_probe_opened" "1" "core settings launch probe opened"
    mfx_assert_eq "$launch_probe_url" "$settings_url" "core settings launch probe url"

    local expected_launcher_command="open"
    if [[ "$platform" == "linux" ]]; then
        expected_launcher_command="xdg-open"
    fi
    mfx_assert_file_contains "$launch_capture_file" "captured=1" "core settings launcher capture flag"
    mfx_assert_file_contains "$launch_capture_file" "command=$expected_launcher_command" "core settings launcher command"
    mfx_assert_file_contains "$launch_capture_file" "url=$settings_url" "core settings launcher url"

    _mfx_core_http_write_permission_sim_state "$permission_sim_file" "0"
    _mfx_core_http_wait_input_capture_state \
        "$base_url" \
        "$token" \
        "false" \
        "permission_denied" \
        "$tmp_dir/input-capture-runtime-revoke.out" \
        "core input-capture runtime revoke"

    _mfx_core_http_write_permission_sim_state "$permission_sim_file" "1"
    _mfx_core_http_wait_input_capture_state \
        "$base_url" \
        "$token" \
        "true" \
        "none" \
        "$tmp_dir/input-capture-runtime-regrant.out" \
        "core input-capture runtime regrant"

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

    local code_state_unauthorized
    code_state_unauthorized="$(mfx_http_code "$tmp_dir/state-unauth.out" "$base_url/api/state")"
    mfx_assert_eq "$code_state_unauthorized" "401" "core state unauthorized status"

    local code_schema
    code_schema="$(mfx_http_code "$tmp_dir/schema.out" "$base_url/api/schema" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_schema" "200" "core schema status"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"capabilities\":" "core schema capabilities section"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"wasm\":" "core schema wasm capabilities section"

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

    local code_app_catalog
    code_app_catalog="$(mfx_http_code "$tmp_dir/app-catalog.out" "$base_url/api/automation/app-catalog" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{"force":true}')"
    mfx_assert_eq "$code_app_catalog" "200" "core app-catalog status"
    mfx_assert_file_contains "$tmp_dir/app-catalog.out" "\"ok\":true" "core app-catalog ok"
    mfx_assert_file_contains "$tmp_dir/app-catalog.out" "\"count\":" "core app-catalog count field"

    local code_app_catalog_cached
    code_app_catalog_cached="$(mfx_http_code "$tmp_dir/app-catalog-cached.out" "$base_url/api/automation/app-catalog" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{"force":false}')"
    mfx_assert_eq "$code_app_catalog_cached" "200" "core app-catalog cached status"
    mfx_assert_file_contains "$tmp_dir/app-catalog-cached.out" "\"ok\":true" "core app-catalog cached ok"
    mfx_assert_file_contains "$tmp_dir/app-catalog-cached.out" "\"count\":" "core app-catalog cached count field"

    local selected_process
    selected_process="$(_mfx_core_http_first_catalog_process "$tmp_dir/app-catalog.out")"
    if [[ -n "$selected_process" ]]; then
        local selected_scope="process:$selected_process"
        local selected_scope_escaped
        selected_scope_escaped="$(_mfx_core_http_json_escape "$selected_scope")"

        local code_state_apply_scope
        code_state_apply_scope="$(mfx_http_code "$tmp_dir/state-apply-scope.out" "$base_url/api/state" \
            -X POST \
            -H "x-mfcmouseeffect-token: $token" \
            -H "Content-Type: application/json" \
            -d "{\"automation\":{\"enabled\":true,\"mouse_mappings\":[{\"enabled\":true,\"trigger\":\"left_click\",\"app_scopes\":[\"$selected_scope_escaped\"],\"keys\":\"Cmd+C\"}]}}")"
        mfx_assert_eq "$code_state_apply_scope" "200" "core state apply selected-scope status"
        mfx_assert_file_contains "$tmp_dir/state-apply-scope.out" "\"ok\":true" "core state apply selected-scope ok"

        local code_state_after_scope
        code_state_after_scope="$(mfx_http_code "$tmp_dir/state-after-scope.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
        mfx_assert_eq "$code_state_after_scope" "200" "core state after selected-scope status"
        mfx_assert_file_contains "$tmp_dir/state-after-scope.out" "\"app_scopes\":[\"$selected_scope\"]" "core selected-scope persistence"
    fi

    _mfx_core_http_assert_scope_match "$base_url" "$token" "code" "process:code.exe" "true" "$tmp_dir/scope-code-vs-exe.out" "core app-scope code<->exe"
    _mfx_core_http_assert_scope_match "$base_url" "$token" "code.app" "process:code" "true" "$tmp_dir/scope-app-vs-base.out" "core app-scope app<->base"
    _mfx_core_http_assert_scope_match "$base_url" "$token" "code.exe" "process:code.app" "true" "$tmp_dir/scope-exe-vs-app.out" "core app-scope exe<->app"
    _mfx_core_http_assert_scope_match "$base_url" "$token" "safari" "process:code" "false" "$tmp_dir/scope-negative.out" "core app-scope negative"

    local payload_priority_scope_specific
    payload_priority_scope_specific='{"process":"code.app","history":["left_click"],"mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["all"],"keys":"Cmd+V"},{"enabled":true,"trigger":"left_click","app_scopes":["process:code.exe"],"keys":"Cmd+C"}]}'
    _mfx_core_http_assert_binding_priority \
        "$base_url" \
        "$token" \
        "$payload_priority_scope_specific" \
        "true" \
        "1" \
        "Cmd+C" \
        "1" \
        "1" \
        "$tmp_dir/scope-priority-specific.out" \
        "core app-scope priority process-over-all"

    local payload_priority_scope_fallback
    payload_priority_scope_fallback='{"process":"safari","history":["left_click"],"mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["all"],"keys":"Cmd+V"},{"enabled":true,"trigger":"left_click","app_scopes":["process:code.exe"],"keys":"Cmd+C"}]}'
    _mfx_core_http_assert_binding_priority \
        "$base_url" \
        "$token" \
        "$payload_priority_scope_fallback" \
        "true" \
        "0" \
        "Cmd+V" \
        "0" \
        "1" \
        "$tmp_dir/scope-priority-fallback.out" \
        "core app-scope priority all-fallback"

    local payload_priority_long_chain
    payload_priority_long_chain='{"process":"code","history":["left_click","left_click"],"mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["process:code.app"],"keys":"Cmd+V"},{"enabled":true,"trigger":"left_click>left_click","app_scopes":["all"],"keys":"Cmd+T"}]}'
    _mfx_core_http_assert_binding_priority \
        "$base_url" \
        "$token" \
        "$payload_priority_long_chain" \
        "true" \
        "1" \
        "Cmd+T" \
        "0" \
        "2" \
        "$tmp_dir/scope-priority-long-chain.out" \
        "core app-scope priority longest-chain-first"

    local code_match_and_inject
    code_match_and_inject="$(mfx_http_code "$tmp_dir/match-and-inject.out" "$base_url/api/automation/test-match-and-inject" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"process":"code.app","history":["left_click"],"mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["all"],"keys":"Cmd+C"}]}')"
    mfx_assert_eq "$code_match_and_inject" "200" "core test-match-and-inject status"
    mfx_assert_file_contains "$tmp_dir/match-and-inject.out" "\"ok\":true" "core test-match-and-inject ok"
    mfx_assert_file_contains "$tmp_dir/match-and-inject.out" "\"matched\":true" "core test-match-and-inject matched"
    mfx_assert_file_contains "$tmp_dir/match-and-inject.out" "\"injected\":true" "core test-match-and-inject injected"
    mfx_assert_file_contains "$tmp_dir/match-and-inject.out" "\"selected_keys\":\"Cmd+C\"" "core test-match-and-inject selected keys"

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

    local code_input_indicator_labels
    code_input_indicator_labels="$(mfx_http_code "$tmp_dir/input-indicator-labels.out" "$base_url/api/input-indicator/test-mouse-labels" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{}')"
    mfx_assert_eq "$code_input_indicator_labels" "200" "core input-indicator labels probe status"
    mfx_assert_file_contains "$tmp_dir/input-indicator-labels.out" "\"ok\":true" "core input-indicator labels probe ok"
    mfx_assert_file_contains "$tmp_dir/input-indicator-labels.out" "\"supported\":true" "core input-indicator labels probe supported"
    mfx_assert_file_contains "$tmp_dir/input-indicator-labels.out" "\"matched\":true" "core input-indicator labels probe matched"
    mfx_assert_file_contains "$tmp_dir/input-indicator-labels.out" "\"labels\":[\"L\",\"R\",\"M\"]" "core input-indicator labels probe labels"

    local code_input_indicator_keyboard_labels
    code_input_indicator_keyboard_labels="$(mfx_http_code "$tmp_dir/input-indicator-keyboard-labels.out" "$base_url/api/input-indicator/test-keyboard-labels" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{}')"
    mfx_assert_eq "$code_input_indicator_keyboard_labels" "200" "core input-indicator keyboard labels probe status"
    mfx_assert_file_contains "$tmp_dir/input-indicator-keyboard-labels.out" "\"ok\":true" "core input-indicator keyboard labels probe ok"
    mfx_assert_file_contains "$tmp_dir/input-indicator-keyboard-labels.out" "\"supported\":true" "core input-indicator keyboard labels probe supported"
    mfx_assert_file_contains "$tmp_dir/input-indicator-keyboard-labels.out" "\"matched\":true" "core input-indicator keyboard labels probe matched"
    mfx_assert_file_contains "$tmp_dir/input-indicator-keyboard-labels.out" "\"labels\":[\"A\",\"Cmd+K9\",\"K6\"]" "core input-indicator keyboard labels probe labels"

    local code_wasm_catalog
    code_wasm_catalog="$(mfx_http_code "$tmp_dir/wasm-catalog.out" "$base_url/api/wasm/catalog" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{}')"
    mfx_assert_eq "$code_wasm_catalog" "200" "core wasm catalog status"
    mfx_assert_file_contains "$tmp_dir/wasm-catalog.out" "\"ok\":true" "core wasm catalog ok"
    mfx_assert_file_contains "$tmp_dir/wasm-catalog.out" "\"plugins\":" "core wasm catalog plugins field"
    mfx_assert_file_contains "$tmp_dir/wasm-catalog.out" "\"search_roots\":" "core wasm catalog search_roots field"

    local code_wasm_import_dialog_probe
    code_wasm_import_dialog_probe="$(mfx_http_code "$tmp_dir/wasm-import-dialog-probe.out" "$base_url/api/wasm/import-from-folder-dialog" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{"probe_only":true}')"
    mfx_assert_eq "$code_wasm_import_dialog_probe" "200" "core wasm import dialog probe status"
    mfx_assert_file_contains "$tmp_dir/wasm-import-dialog-probe.out" "\"ok\":true" "core wasm import dialog probe ok"
    mfx_assert_file_contains "$tmp_dir/wasm-import-dialog-probe.out" "\"probe_only\":true" "core wasm import dialog probe mode"
    mfx_assert_file_contains "$tmp_dir/wasm-import-dialog-probe.out" "\"supported\":" "core wasm import dialog supported field"

    local repo_root
    repo_root="$(_mfx_core_http_repo_root)"
    local wasm_manifest_path="$repo_root/examples/wasm-plugin-template/dist/plugin.json"
    if [[ ! -f "$wasm_manifest_path" ]]; then
        wasm_manifest_path="$(sed -n 's/.*"manifest_path":"\([^"]*\)".*/\1/p' "$tmp_dir/wasm-catalog.out" | head -n 1)"
    fi
    if [[ -n "$wasm_manifest_path" ]]; then
        local wasm_manifest_path_escaped
        wasm_manifest_path_escaped="$(_mfx_core_http_json_escape "$wasm_manifest_path")"

        local code_wasm_import_selected
        code_wasm_import_selected="$(mfx_http_code "$tmp_dir/wasm-import-selected.out" "$base_url/api/wasm/import-selected" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d "{\"manifest_path\":\"$wasm_manifest_path_escaped\"}")"
        mfx_assert_eq "$code_wasm_import_selected" "200" "core wasm import-selected status"
        mfx_assert_file_contains "$tmp_dir/wasm-import-selected.out" "\"ok\":true" "core wasm import-selected ok"
        mfx_assert_file_contains "$tmp_dir/wasm-import-selected.out" "\"manifest_path\":\"" "core wasm import-selected manifest_path"

        local code_wasm_load_manifest
        code_wasm_load_manifest="$(mfx_http_code "$tmp_dir/wasm-load-manifest.out" "$base_url/api/wasm/load-manifest" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d "{\"manifest_path\":\"$wasm_manifest_path_escaped\"}")"
        mfx_assert_eq "$code_wasm_load_manifest" "200" "core wasm load-manifest status"
        mfx_assert_file_contains "$tmp_dir/wasm-load-manifest.out" "\"ok\":true" "core wasm load-manifest ok"

        local code_wasm_enable
        code_wasm_enable="$(mfx_http_code "$tmp_dir/wasm-enable.out" "$base_url/api/wasm/enable" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{}')"
        mfx_assert_eq "$code_wasm_enable" "200" "core wasm enable status"
        mfx_assert_file_contains "$tmp_dir/wasm-enable.out" "\"ok\":true" "core wasm enable ok"

        local code_wasm_test_dispatch
        code_wasm_test_dispatch="$(mfx_http_code "$tmp_dir/wasm-test-dispatch.out" "$base_url/api/wasm/test-dispatch-click" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{"x":640,"y":360,"button":1}')"
        mfx_assert_eq "$code_wasm_test_dispatch" "200" "core wasm test-dispatch status"
        mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch.out" "\"ok\":true" "core wasm test-dispatch ok"
        mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch.out" "\"route_active\":true" "core wasm test-dispatch route_active"
        mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch.out" "\"invoke_ok\":true" "core wasm test-dispatch invoke_ok"
    else
        mfx_info "skip wasm dispatch test: no plugin manifest found"
    fi

    if [[ "$platform" == "macos" ]]; then
        if ! rg -q --fixed-strings "\"runtime_backend\":\"wasm3_static\"" "$tmp_dir/state.out"; then
            mfx_fail "core wasm runtime backend on macos: expected wasm3_static"
        fi
        if ! rg -q --fixed-strings "\"render_supported\":true" "$tmp_dir/state.out"; then
            mfx_fail "core wasm render capability on macos: expected render_supported=true"
        fi
        if rg -q --fixed-strings "\"wasm_catalog_not_supported_on_this_platform\"" "$tmp_dir/wasm-catalog.out"; then
            mfx_fail "core wasm catalog support on macos: unexpected unsupported marker"
        fi
        if ! rg -q --fixed-strings "\"supported\":true" "$tmp_dir/wasm-import-dialog-probe.out"; then
            mfx_fail "core wasm import dialog support on macos: expected supported=true"
        fi
        if [[ -f "$tmp_dir/wasm-test-dispatch.out" ]]; then
            if ! rg -q --fixed-strings "\"rendered_any\":true" "$tmp_dir/wasm-test-dispatch.out"; then
                mfx_fail "core wasm test-dispatch render on macos: expected rendered_any=true"
            fi
        fi
        if rg -q --fixed-strings "\"count\":0" "$tmp_dir/app-catalog.out"; then
            mfx_fail "core app-catalog non-empty on macos: unexpected count=0"
        fi
        if ! rg -q "\"exe\":\"[^\"]+\\.app\"" "$tmp_dir/app-catalog.out"; then
            mfx_fail "core app-catalog app suffix on macos: missing .app process entry"
        fi
        if ! rg -q "\"process\":\"[^\"]+\"" "$tmp_dir/active-process.out"; then
            mfx_fail "core active-process non-empty on macos: expected non-empty process base name"
        fi
        if ! rg -q --fixed-strings "\"keyboard_injector\":true" "$tmp_dir/schema.out"; then
            mfx_fail "core schema keyboard injector capability on macos: expected true"
        fi
        if ! rg -q --fixed-strings "\"accepted\":true" "$tmp_dir/test-inject-shortcut.out"; then
            mfx_fail "core test-inject-shortcut on macos: expected accepted=true (dry-run injector mode)"
        fi
    fi

    trap - EXIT
    _mfx_core_http_stop_entry
    rm -rf "$tmp_dir"
    mfx_ok "core HTTP contract checks completed"
}
