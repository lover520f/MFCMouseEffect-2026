#!/usr/bin/env bash

set -euo pipefail

MFX_MANUAL_HOST_PID=""
MFX_MANUAL_SETTINGS_URL=""
MFX_MANUAL_SETTINGS_TOKEN=""
MFX_MANUAL_BASE_URL=""
MFX_MANUAL_LOG_FILE=""
MFX_MANUAL_HOST_BIN=""

mfx_manual_probe_value() {
    local key="$1"
    local file_path="$2"
    sed -n "s/^${key}=//p" "$file_path" | head -n 1
}

mfx_manual_trim_trailing_slash() {
    local value="$1"
    while [[ "$value" == */ ]]; do
        value="${value%/}"
    done
    printf '%s' "$value"
}

mfx_manual_start_core_host() {
    local host_bin="$1"
    local probe_file="$2"
    local log_file="$3"
    shift 3
    local -a extra_env=("$@")

    if [[ ! -x "$host_bin" ]]; then
        mfx_fail "host binary missing or not executable: $host_bin"
    fi

    mfx_terminate_stale_entry_host "before manual host start"

    rm -f "$probe_file"

    mfx_info "start host (tray mode)"
    nohup env MFX_CORE_WEB_SETTINGS_PROBE_FILE="$probe_file" \
        "${extra_env[@]}" \
        "$host_bin" --mode=tray >"$log_file" 2>&1 &
    local pid="$!"

    for _ in $(seq 1 100); do
        if [[ -s "$probe_file" ]]; then
            break
        fi
        if ! kill -0 "$pid" 2>/dev/null; then
            break
        fi
        sleep 0.1
    done

    if ! kill -0 "$pid" 2>/dev/null; then
        tail -n 80 "$log_file" >&2 || true
        mfx_fail "host exited early"
    fi

    local settings_url
    settings_url="$(mfx_manual_probe_value "url" "$probe_file")"
    local token
    token="$(mfx_manual_probe_value "token" "$probe_file")"
    if [[ -z "$settings_url" || -z "$token" ]]; then
        tail -n 80 "$log_file" >&2 || true
        mfx_fail "probe file missing url/token: $probe_file"
    fi

    MFX_MANUAL_HOST_PID="$pid"
    MFX_MANUAL_SETTINGS_URL="$settings_url"
    MFX_MANUAL_SETTINGS_TOKEN="$token"
    MFX_MANUAL_BASE_URL="$(mfx_manual_trim_trailing_slash "${settings_url%%\?*}")"
    MFX_MANUAL_LOG_FILE="$log_file"
}

mfx_manual_stop_core_host() {
    local pid="${1:-$MFX_MANUAL_HOST_PID}"
    if [[ -z "$pid" ]]; then
        return 0
    fi
    if ! kill -0 "$pid" 2>/dev/null; then
        return 0
    fi

    local stop_timeout_seconds="${MFX_MANUAL_STOP_TIMEOUT_SECONDS:-5}"
    if ! [[ "$stop_timeout_seconds" =~ ^[0-9]+$ ]]; then
        stop_timeout_seconds=5
    fi

    kill -TERM "$pid" 2>/dev/null || true

    local deadline=$((SECONDS + stop_timeout_seconds))
    while kill -0 "$pid" 2>/dev/null; do
        if (( SECONDS >= deadline )); then
            mfx_info "manual host graceful stop timeout; send KILL (pid=$pid)"
            kill -KILL "$pid" 2>/dev/null || true
            break
        fi
        sleep 0.1
    done

    wait "$pid" 2>/dev/null || true
}

mfx_manual_schedule_auto_stop() {
    local pid="$1"
    local seconds="$2"
    if [[ -z "$pid" || "$seconds" -le 0 ]]; then
        return 0
    fi
    (
        sleep "$seconds"
        kill -TERM "$pid" 2>/dev/null || true
    ) >/dev/null 2>&1 &
}

mfx_manual_acquire_entry_host_lock() {
    local timeout_seconds="${MFX_ENTRY_LOCK_TIMEOUT_SECONDS:-180}"
    mfx_info "entry host lock: mfx-entry-posix-host"
    mfx_acquire_lock "mfx-entry-posix-host" "$timeout_seconds"
}

mfx_manual_validate_non_negative_integer() {
    local value="$1"
    local option_name="$2"
    if ! [[ "$value" =~ ^[0-9]+$ ]]; then
        mfx_fail "$option_name must be a non-negative integer"
    fi
}

mfx_manual_apply_build_jobs_env() {
    local build_jobs="$1"
    local option_name="${2:---jobs}"
    if [[ -z "$build_jobs" ]]; then
        return 0
    fi
    if ! [[ "$build_jobs" =~ ^[0-9]+$ ]] || [[ "$build_jobs" -le 0 ]]; then
        mfx_fail "$option_name must be a positive integer"
    fi
    export MFX_BUILD_JOBS="$build_jobs"
}

mfx_manual_prepare_core_host_binary() {
    local repo_root="$1"
    local build_dir="$2"
    local skip_build="${3:-0}"
    local host_bin="$build_dir/mfx_entry_posix_host"

    if [[ "$skip_build" -eq 0 ]]; then
        mfx_require_cmd cmake
        mfx_configure_platform_build_dir "$repo_root" "$build_dir" "macos" \
            -DMFX_ENABLE_CROSS_HOST_PACKAGES=ON \
            -DMFX_ENABLE_ENTRY_RUNTIME_TARGETS=ON \
            -DMFX_ENABLE_POSIX_CORE_RUNTIME=ON
        mfx_build_targets "$build_dir" "mfx_entry_posix_host"
    fi

    if [[ ! -x "$host_bin" ]]; then
        mfx_fail "host binary missing or not executable: $host_bin"
    fi

    MFX_MANUAL_HOST_BIN="$host_bin"
}

mfx_manual_print_stop_command() {
    local pid="${1:-$MFX_MANUAL_HOST_PID}"
    if [[ -z "$pid" ]]; then
        return 0
    fi
    printf 'stop_cmd=kill -TERM %s\n' "$pid"
}
