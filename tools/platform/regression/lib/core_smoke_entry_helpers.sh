#!/usr/bin/env bash

set -euo pipefail

_mfx_core_smoke_entry_pid=""
_mfx_core_smoke_fifo_path=""
_mfx_core_smoke_fifo_dir=""
_mfx_core_smoke_fifo_writer_pid=""

_mfx_core_smoke_resolve_single_instance_key() {
    local explicit_key="${MFX_CORE_SMOKE_SINGLE_INSTANCE_KEY:-}"
    if [[ -n "$explicit_key" ]]; then
        printf '%s' "$explicit_key"
        return 0
    fi

    local stamp="0"
    stamp="$(date +%s 2>/dev/null || printf '0')"
    printf 'Global\\MFCMouseEffect_CoreSmoke_%s_%s' "$$" "$stamp"
}

_mfx_core_smoke_prepare_fifo_runtime() {
    _mfx_core_smoke_fifo_dir="$(mktemp -d "/tmp/mfx-posix-core-smoke-fifo.XXXXXX")"
    _mfx_core_smoke_fifo_path="$_mfx_core_smoke_fifo_dir/entry.fifo"
    mkfifo "$_mfx_core_smoke_fifo_path"
}

_mfx_core_smoke_start_entry() {
    local entry_bin="$1"
    local log_file="$2"
    local single_instance_key=""
    single_instance_key="$(_mfx_core_smoke_resolve_single_instance_key)"

    _mfx_core_smoke_prepare_fifo_runtime

    tail -f /dev/null >"$_mfx_core_smoke_fifo_path" &
    _mfx_core_smoke_fifo_writer_pid="$!"

    MFX_SINGLE_INSTANCE_KEY="$single_instance_key" \
        "$entry_bin" -mode=background "--single-instance-key=$single_instance_key" <"$_mfx_core_smoke_fifo_path" >"$log_file" 2>&1 &
    _mfx_core_smoke_entry_pid="$!"

    sleep "${MFX_CORE_SMOKE_START_WAIT_SECONDS:-1}"
    if ! kill -0 "$_mfx_core_smoke_entry_pid" >/dev/null 2>&1; then
        mfx_info "core smoke startup log:"
        cat "$log_file" || true
        mfx_fail "core smoke entry exited before alive check"
    fi
}

_mfx_core_smoke_wait_for_exit() {
    local pid="$1"
    local timeout_seconds="${2:-5}"
    local deadline=$((SECONDS + timeout_seconds))
    while kill -0 "$pid" >/dev/null 2>&1; do
        if (( SECONDS >= deadline )); then
            return 1
        fi
        sleep 0.1
    done
    return 0
}

_mfx_core_smoke_stop_entry() {
    local exit_timeout_seconds="${MFX_CORE_SMOKE_EXIT_TIMEOUT_SECONDS:-5}"

    if [[ -n "$_mfx_core_smoke_fifo_path" && -p "$_mfx_core_smoke_fifo_path" ]]; then
        printf 'exit\n' >"$_mfx_core_smoke_fifo_path" || true
    fi

    if [[ -n "$_mfx_core_smoke_entry_pid" ]]; then
        if ! _mfx_core_smoke_wait_for_exit "$_mfx_core_smoke_entry_pid" "$exit_timeout_seconds"; then
            kill -TERM "$_mfx_core_smoke_entry_pid" >/dev/null 2>&1 || true
        fi
        wait "$_mfx_core_smoke_entry_pid" >/dev/null 2>&1 || true
    fi

    if [[ -n "$_mfx_core_smoke_fifo_writer_pid" ]]; then
        kill "$_mfx_core_smoke_fifo_writer_pid" >/dev/null 2>&1 || true
        wait "$_mfx_core_smoke_fifo_writer_pid" >/dev/null 2>&1 || true
    fi

    if [[ -n "$_mfx_core_smoke_fifo_path" ]]; then
        rm -f "$_mfx_core_smoke_fifo_path"
    fi
    if [[ -n "$_mfx_core_smoke_fifo_dir" ]]; then
        rm -rf "$_mfx_core_smoke_fifo_dir"
    fi

    _mfx_core_smoke_entry_pid=""
    _mfx_core_smoke_fifo_path=""
    _mfx_core_smoke_fifo_dir=""
    _mfx_core_smoke_fifo_writer_pid=""
}
