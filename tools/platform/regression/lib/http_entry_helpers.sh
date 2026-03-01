#!/usr/bin/env bash

set -euo pipefail

_mfx_http_entry_pid=""
_mfx_http_fifo_path=""
_mfx_http_fifo_writer_pid=""
_mfx_http_startup_skip_reason=""

_mfx_http_skip_startup_failures_enabled() {
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

_mfx_http_entry_log_has_bind_denied() {
    local log_file="$1"
    [[ -s "$log_file" ]] || return 1
    rg -q "Scaffold settings server failed to start \\(stage=2,code=(1|13)\\)" "$log_file"
}

_mfx_http_cleanup_entry_runtime() {
    if [[ -n "$_mfx_http_fifo_writer_pid" ]]; then
        kill "$_mfx_http_fifo_writer_pid" >/dev/null 2>&1 || true
        wait "$_mfx_http_fifo_writer_pid" >/dev/null 2>&1 || true
    fi
    if [[ -n "$_mfx_http_fifo_path" ]]; then
        rm -f "$_mfx_http_fifo_path"
    fi
    _mfx_http_entry_pid=""
    _mfx_http_fifo_path=""
    _mfx_http_fifo_writer_pid=""
}

_mfx_http_start_entry() {
    local entry_bin="$1"
    local log_file="$2"
    shift 2
    local -a entry_env=("$@")
    local retry_count
    _mfx_http_startup_skip_reason=""
    retry_count="$(mfx_parse_non_negative_integer_or_default "${MFX_HTTP_ENTRY_START_RETRIES:-1}" "1")"
    local max_attempts=$((retry_count + 1))
    local attempt=1
    local server_wait_seconds
    server_wait_seconds="$(mfx_parse_positive_integer_or_default "${MFX_HTTP_SERVER_WAIT_SECONDS:-1}" "1")"

    while (( attempt <= max_attempts )); do
        _mfx_http_fifo_path="$(mktemp -u "/tmp/mfx-posix-http-fifo.XXXXXX")"
        mkfifo "$_mfx_http_fifo_path"

        tail -f /dev/null >"$_mfx_http_fifo_path" &
        _mfx_http_fifo_writer_pid="$!"

        if [[ ${#entry_env[@]} -gt 0 ]]; then
            env "${entry_env[@]}" "$entry_bin" -mode=background <"$_mfx_http_fifo_path" >"$log_file" 2>&1 &
        else
            "$entry_bin" -mode=background <"$_mfx_http_fifo_path" >"$log_file" 2>&1 &
        fi
        _mfx_http_entry_pid="$!"

        sleep "$server_wait_seconds"
        if kill -0 "$_mfx_http_entry_pid" >/dev/null 2>&1; then
            return 0
        fi

        mfx_info "entry startup attempt $attempt/$max_attempts failed before HTTP checks"
        mfx_info "entry startup log:"
        cat "$log_file" || true
        _mfx_http_cleanup_entry_runtime

        if (( attempt == max_attempts )); then
            if _mfx_http_skip_startup_failures_enabled; then
                if _mfx_http_entry_log_has_bind_denied "$log_file"; then
                    _mfx_http_startup_skip_reason="loopback bind denied under constrained runtime (stage=2,code=1|13)"
                    return 2
                fi
                if [[ ! -s "$log_file" ]]; then
                    _mfx_http_startup_skip_reason="entry exited before HTTP checks without startup log under constrained runtime"
                    return 2
                fi
            fi
            mfx_fail "entry process exited before HTTP checks"
        fi
        attempt=$((attempt + 1))
        sleep 0.2
    done
}

_mfx_http_stop_entry() {
    if [[ -n "$_mfx_http_fifo_path" && -p "$_mfx_http_fifo_path" ]]; then
        printf 'exit\n' >"$_mfx_http_fifo_path" || true
    fi

    if [[ -n "$_mfx_http_entry_pid" ]]; then
        wait "$_mfx_http_entry_pid" >/dev/null 2>&1 || true
    fi

    _mfx_http_cleanup_entry_runtime
}
