#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_try_recover_probe_file_from_launch_probe() {
    local probe_file="$1"
    local launch_probe_file="$2"
    if [[ -s "$probe_file" || ! -s "$launch_probe_file" ]]; then
        return 1
    fi

    local launch_url
    local token
    launch_url="$(_mfx_core_http_probe_value "url" "$launch_probe_file")"
    if [[ -z "$launch_url" ]]; then
        return 1
    fi
    token="$(printf '%s' "$launch_url" | sed -n 's/.*[?&]token=\([^&]*\).*/\1/p')"
    if [[ -z "$token" ]]; then
        return 1
    fi

    {
        printf 'url=%s\n' "$launch_url"
        printf 'token=%s\n' "$token"
    } > "${probe_file}.tmp"
    mv "${probe_file}.tmp" "$probe_file"
    mfx_info "core http recovered probe file from launch probe url"
    return 0
}

_mfx_core_http_resolve_single_instance_key() {
    local explicit_key="${MFX_CORE_HTTP_SINGLE_INSTANCE_KEY:-}"
    if [[ -n "$explicit_key" ]]; then
        printf '%s' "$explicit_key"
        return 0
    fi

    local stamp="0"
    stamp="$(date +%s 2>/dev/null || printf '0')"
    printf 'Global\\MFCMouseEffect_CoreHttp_%s_%s' "$$" "$stamp"
}

_mfx_core_http_early_exit_without_probe() {
    local probe_file="$1"
    local launch_probe_file="$2"
    local log_file="$3"
    local diagnostics_file="$4"
    local require_launch_probe="${5:-1}"

    if [[ -s "$probe_file" ]]; then
        return 1
    fi
    if [[ "$require_launch_probe" == "1" && -s "$launch_probe_file" ]]; then
        return 1
    fi
    if [[ -s "$log_file" ]]; then
        return 1
    fi
    if [[ -n "$diagnostics_file" && -s "$diagnostics_file" ]]; then
        return 1
    fi
    return 0
}

_mfx_core_http_cleanup_startup_runtime() {
    if [[ -n "${_mfx_core_http_entry_pid:-}" ]]; then
        kill -TERM "$_mfx_core_http_entry_pid" >/dev/null 2>&1 || true
        wait "$_mfx_core_http_entry_pid" >/dev/null 2>&1 || true
    fi
    if [[ -n "${_mfx_core_http_fifo_writer_pid:-}" ]]; then
        kill "$_mfx_core_http_fifo_writer_pid" >/dev/null 2>&1 || true
        wait "$_mfx_core_http_fifo_writer_pid" >/dev/null 2>&1 || true
    fi
    if [[ -n "${_mfx_core_http_fifo_path:-}" ]]; then
        rm -f "$_mfx_core_http_fifo_path"
    fi
    if [[ -n "${_mfx_core_http_fifo_dir:-}" ]]; then
        rm -rf "$_mfx_core_http_fifo_dir"
    fi
    _mfx_core_http_entry_pid=""
    _mfx_core_http_fifo_path=""
    _mfx_core_http_fifo_dir=""
    _mfx_core_http_fifo_writer_pid=""
}

_mfx_core_http_prepare_fifo_runtime() {
    _mfx_core_http_fifo_dir="$(mktemp -d "/tmp/mfx-posix-core-http-fifo.XXXXXX")"
    _mfx_core_http_fifo_path="$_mfx_core_http_fifo_dir/entry.fifo"
    mkfifo "$_mfx_core_http_fifo_path"
}

_mfx_core_http_try_stop_entry_via_http() {
    local probe_file="$1"
    if [[ -z "$probe_file" || ! -s "$probe_file" ]]; then
        return 1
    fi

    local settings_url
    local token
    local base_url
    settings_url="$(_mfx_core_http_probe_value "url" "$probe_file")"
    token="$(_mfx_core_http_probe_value "token" "$probe_file")"
    if [[ -z "$settings_url" || -z "$token" ]]; then
        return 1
    fi

    base_url="${settings_url%%\?*}"
    while [[ "$base_url" == */ ]]; do
        base_url="${base_url%/}"
    done
    if [[ -z "$base_url" ]]; then
        return 1
    fi

    local stop_url="$base_url/api/stop"
    local tmp_file
    tmp_file="$(mktemp)"
    local http_code=""
    http_code="$(
        curl -sS -m 2 \
            -o "$tmp_file" \
            -w "%{http_code}" \
            -X POST \
            -H "x-mfcmouseeffect-token: $token" \
            "$stop_url" 2>/dev/null || true
    )"
    rm -f "$tmp_file" || true

    if [[ "$http_code" == "200" || "$http_code" == "204" ]]; then
        mfx_info "core entry stop requested via /api/stop"
        return 0
    fi
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
    local require_launch_probe="${8:-1}"
    local probe_diagnostics_file="${9:-}"
    local single_instance_key=""
    single_instance_key="$(_mfx_core_http_resolve_single_instance_key)"
    _mfx_core_http_startup_skip_reason=""

    _mfx_core_http_probe_file="$probe_file"
    _mfx_core_http_launch_probe_file="$launch_probe_file"
    _mfx_core_http_launch_capture_file="$launch_capture_file"
    _mfx_core_http_permission_sim_file="$permission_sim_file"
    _mfx_core_http_notification_capture_file="$notification_capture_file"
    _mfx_core_http_probe_diagnostics_file="$probe_diagnostics_file"
    rm -f "$probe_file" "${probe_file}.tmp" || true
    rm -f "$launch_probe_file" "${launch_probe_file}.tmp" || true
    rm -f "$launch_capture_file" "${launch_capture_file}.tmp" || true
    rm -f "$notification_capture_file" "${notification_capture_file}.tmp" || true
    if [[ -n "$probe_diagnostics_file" ]]; then
        rm -f "$probe_diagnostics_file" "${probe_diagnostics_file}.tmp" || true
    fi
    local retry_count
    retry_count="$(mfx_parse_non_negative_integer_or_default "${MFX_CORE_HTTP_ENTRY_START_RETRIES:-1}" "1")"
    local max_attempts=$((retry_count + 1))
    local attempt=1
    local start_wait_seconds
    start_wait_seconds="$(mfx_parse_positive_integer_or_default "${MFX_CORE_HTTP_START_WAIT_SECONDS:-1}" "1")"
    if [[ -n "$single_instance_key" ]]; then
        mfx_info "core http single-instance key: $single_instance_key"
    fi
    while (( attempt <= max_attempts )); do
        _mfx_core_http_prepare_fifo_runtime

        tail -f /dev/null >"$_mfx_core_http_fifo_path" &
        _mfx_core_http_fifo_writer_pid="$!"

        if [[ "$require_launch_probe" == "1" ]]; then
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
            MFX_CORE_WEB_SETTINGS_PROBE_DIAGNOSTICS_FILE="$probe_diagnostics_file" \
            MFX_SINGLE_INSTANCE_KEY="$single_instance_key" \
                "$entry_bin" -mode=background "--single-instance-key=$single_instance_key" <"$_mfx_core_http_fifo_path" >"$log_file" 2>&1 &
        else
            MFX_CORE_WEB_SETTINGS_PROBE_FILE="$probe_file" \
            MFX_TEST_INPUT_CAPTURE_PERMISSION_SIM_FILE="$permission_sim_file" \
            MFX_TEST_NOTIFICATION_CAPTURE_FILE="$notification_capture_file" \
            MFX_ENABLE_AUTOMATION_SCOPE_TEST_API="${MFX_ENABLE_AUTOMATION_SCOPE_TEST_API:-1}" \
            MFX_ENABLE_AUTOMATION_SHORTCUT_TEST_API="${MFX_ENABLE_AUTOMATION_SHORTCUT_TEST_API:-1}" \
            MFX_ENABLE_AUTOMATION_INJECTION_TEST_API="${MFX_ENABLE_AUTOMATION_INJECTION_TEST_API:-1}" \
            MFX_ENABLE_EFFECT_OVERLAY_TEST_API="${MFX_ENABLE_EFFECT_OVERLAY_TEST_API:-1}" \
            MFX_ENABLE_INPUT_INDICATOR_TEST_API="${MFX_ENABLE_INPUT_INDICATOR_TEST_API:-1}" \
            MFX_TEST_KEYBOARD_INJECTOR_DRY_RUN="${MFX_TEST_KEYBOARD_INJECTOR_DRY_RUN:-1}" \
            MFX_ENABLE_WASM_TEST_DISPATCH_API="${MFX_ENABLE_WASM_TEST_DISPATCH_API:-1}" \
            MFX_CORE_WEB_SETTINGS_PROBE_DIAGNOSTICS_FILE="$probe_diagnostics_file" \
            MFX_SINGLE_INSTANCE_KEY="$single_instance_key" \
                "$entry_bin" -mode=background "--single-instance-key=$single_instance_key" <"$_mfx_core_http_fifo_path" >"$log_file" 2>&1 &
        fi
        _mfx_core_http_entry_pid="$!"

        sleep "$start_wait_seconds"
        if kill -0 "$_mfx_core_http_entry_pid" >/dev/null 2>&1; then
            local launch_probe_ready=1
            if [[ "$require_launch_probe" == "1" ]]; then
                launch_probe_ready=0
                if _mfx_core_http_wait_launch_probe_file "$launch_probe_file"; then
                    launch_probe_ready=1
                fi
            fi
            if [[ "$launch_probe_ready" == "1" ]]; then
                if _mfx_core_http_wait_probe_file "$probe_file" || \
                   _mfx_core_http_try_recover_probe_file_from_launch_probe "$probe_file" "$launch_probe_file"; then
                    return 0
                fi
            fi
        fi

        mfx_info "core http startup attempt $attempt/$max_attempts failed"
        mfx_info "core http startup log:"
        cat "$log_file" || true
        if [[ -n "$probe_diagnostics_file" && -s "$probe_diagnostics_file" ]]; then
            mfx_info "core http probe diagnostics:"
            cat "$probe_diagnostics_file" || true
            if grep -Eq "reason=websettings_start_failed\\(stage=2,code=(1|13)\\)" "$probe_diagnostics_file"; then
                mfx_info "hint: websettings bind permission denied (EPERM/EACCES stage=2 code=1|13); check runtime sandbox/network permissions."
            fi
        fi

        local allow_bind_eacces_skip="${MFX_CORE_HTTP_ALLOW_BIND_EACCES_SKIP:-0}"
        if [[ "$allow_bind_eacces_skip" == "1" && -n "$probe_diagnostics_file" && -s "$probe_diagnostics_file" ]]; then
            if grep -Eq "reason=websettings_start_failed\\(stage=2,code=(1|13)\\)" "$probe_diagnostics_file"; then
                _mfx_core_http_cleanup_startup_runtime
                _mfx_core_http_startup_skip_reason="websettings bind permission denied under constrained runtime (stage=2,code=1|13)"
                return 2
            fi
        fi
        if [[ "$allow_bind_eacces_skip" == "1" ]]; then
            if _mfx_core_http_early_exit_without_probe \
                "$probe_file" \
                "$launch_probe_file" \
                "$log_file" \
                "$probe_diagnostics_file" \
                "$require_launch_probe"; then
                _mfx_core_http_cleanup_startup_runtime
                _mfx_core_http_startup_skip_reason="host exited before probe/log in constrained runtime (likely unavailable tray/gui session)"
                return 2
            fi
        fi
        _mfx_core_http_cleanup_startup_runtime

        if (( attempt == max_attempts )); then
            if [[ "$allow_bind_eacces_skip" == "1" && -n "$probe_diagnostics_file" && -s "$probe_diagnostics_file" ]]; then
                if grep -Eq "reason=websettings_start_failed\\(stage=2,code=(1|13)\\)" "$probe_diagnostics_file"; then
                    _mfx_core_http_startup_skip_reason="websettings bind permission denied under constrained runtime (stage=2,code=1|13)"
                    return 2
                fi
            fi
            if [[ "$allow_bind_eacces_skip" == "1" ]]; then
                if _mfx_core_http_early_exit_without_probe \
                    "$probe_file" \
                    "$launch_probe_file" \
                    "$log_file" \
                    "$probe_diagnostics_file" \
                    "$require_launch_probe"; then
                    _mfx_core_http_startup_skip_reason="host exited before probe/log in constrained runtime (likely unavailable tray/gui session)"
                    return 2
                fi
            fi
            if [[ ! -s "$probe_file" ]]; then
                return 1
            fi
            if [[ "$require_launch_probe" == "1" ]]; then
                return 1
            fi
            return 1
        fi

        attempt=$((attempt + 1))
        sleep 0.2
    done
}

_mfx_core_http_stop_entry() {
    local stop_timeout_seconds
    stop_timeout_seconds="$(mfx_parse_non_negative_integer_or_default "${MFX_CORE_HTTP_STOP_TIMEOUT_SECONDS:-8}" "8")"
    local graceful_wait_seconds
    graceful_wait_seconds="$(mfx_parse_non_negative_integer_or_default "${MFX_CORE_HTTP_GRACEFUL_STOP_WAIT_SECONDS:-1}" "1")"
    local term_wait_seconds
    term_wait_seconds="$(mfx_parse_non_negative_integer_or_default "${MFX_CORE_HTTP_TERM_WAIT_SECONDS:-3}" "3")"

    local stop_requested_via_http=0
    if _mfx_core_http_try_stop_entry_via_http "${_mfx_core_http_probe_file:-}"; then
        stop_requested_via_http=1
    fi
    if [[ -n "$_mfx_core_http_fifo_path" && -p "$_mfx_core_http_fifo_path" ]]; then
        printf 'exit\n' >"$_mfx_core_http_fifo_path" || true
    fi

    if [[ -n "$_mfx_core_http_entry_pid" ]]; then
        if [[ "$stop_requested_via_http" -eq 1 && "$graceful_wait_seconds" -lt 3 ]]; then
            graceful_wait_seconds=3
        fi
        local deadline=$((SECONDS + stop_timeout_seconds))
        local graceful_deadline=$((SECONDS + graceful_wait_seconds))
        local term_sent=0
        local term_deadline=0

        while kill -0 "$_mfx_core_http_entry_pid" >/dev/null 2>&1; do
            if [[ "$term_sent" -eq 0 ]] && (( SECONDS >= graceful_deadline )); then
                mfx_info "core entry graceful stop timeout; send TERM (pid=$_mfx_core_http_entry_pid)"
                kill -TERM "$_mfx_core_http_entry_pid" >/dev/null 2>&1 || true
                term_sent=1
                term_deadline=$((SECONDS + term_wait_seconds))
            fi
            if [[ "$term_sent" -eq 1 ]] && (( SECONDS >= term_deadline )); then
                mfx_info "core entry TERM timeout; send KILL (pid=$_mfx_core_http_entry_pid)"
                kill -KILL "$_mfx_core_http_entry_pid" >/dev/null 2>&1 || true
                break
            fi
            if (( SECONDS >= deadline )); then
                mfx_info "core entry stop hard-timeout; force KILL (pid=$_mfx_core_http_entry_pid)"
                kill -KILL "$_mfx_core_http_entry_pid" >/dev/null 2>&1 || true
                break
            fi
            sleep 0.1
        done
        wait "$_mfx_core_http_entry_pid" >/dev/null 2>&1 || true
    fi

    if [[ -n "$_mfx_core_http_fifo_writer_pid" ]]; then
        kill "$_mfx_core_http_fifo_writer_pid" >/dev/null 2>&1 || true
        wait "$_mfx_core_http_fifo_writer_pid" >/dev/null 2>&1 || true
    fi

    if [[ -n "$_mfx_core_http_fifo_path" ]]; then
        rm -f "$_mfx_core_http_fifo_path"
    fi
    if [[ -n "${_mfx_core_http_fifo_dir:-}" ]]; then
        rm -rf "$_mfx_core_http_fifo_dir"
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
    if [[ -n "$_mfx_core_http_probe_diagnostics_file" ]]; then
        rm -f "$_mfx_core_http_probe_diagnostics_file" "${_mfx_core_http_probe_diagnostics_file}.tmp" || true
    fi

    _mfx_core_http_entry_pid=""
    _mfx_core_http_fifo_path=""
    _mfx_core_http_fifo_dir=""
    _mfx_core_http_fifo_writer_pid=""
    _mfx_core_http_probe_file=""
    _mfx_core_http_launch_probe_file=""
    _mfx_core_http_launch_capture_file=""
    _mfx_core_http_permission_sim_file=""
    _mfx_core_http_notification_capture_file=""
    _mfx_core_http_probe_diagnostics_file=""
}
