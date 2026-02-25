#!/usr/bin/env bash

set -euo pipefail

_mfx_lock_dir=""
_mfx_lock_name=""

mfx_log() {
    local level="$1"
    shift
    printf '[mfx:%s] %s\n' "$level" "$*"
}

mfx_info() {
    mfx_log "info" "$@"
}

mfx_ok() {
    mfx_log "ok" "$@"
}

mfx_fail() {
    mfx_log "fail" "$@"
    exit 1
}

mfx_require_cmd() {
    local cmd="$1"
    if ! command -v "$cmd" >/dev/null 2>&1; then
        mfx_fail "missing required command: $cmd"
    fi
}

mfx_detect_posix_host_platform() {
    case "$(uname -s)" in
        Darwin) echo "macos" ;;
        Linux) echo "linux" ;;
        *)
            mfx_fail "unsupported host platform: $(uname -s). expected macOS or Linux."
            ;;
    esac
}

mfx_resolve_posix_platform() {
    local requested_platform="$1"
    local host_platform="$2"
    local cross_host_context="$3"

    local resolved_platform="$requested_platform"
    if [[ "$resolved_platform" == "auto" ]]; then
        resolved_platform="$host_platform"
    fi

    if [[ "$resolved_platform" != "macos" && "$resolved_platform" != "linux" ]]; then
        mfx_fail "invalid --platform value: $resolved_platform"
    fi

    if [[ "$resolved_platform" != "$host_platform" ]]; then
        mfx_fail "cross-host $cross_host_context is unsupported (host=$host_platform, requested=$resolved_platform)"
    fi

    printf '%s' "$resolved_platform"
}

mfx_assert_eq() {
    local actual="$1"
    local expected="$2"
    local context="$3"
    if [[ "$actual" != "$expected" ]]; then
        mfx_fail "$context: expected $expected, got $actual"
    fi
}

mfx_file_contains_fixed() {
    local file_path="$1"
    local pattern="$2"
    if command -v rg >/dev/null 2>&1; then
        rg -q --fixed-strings "$pattern" "$file_path"
        return $?
    fi
    grep -Fq -- "$pattern" "$file_path"
}

mfx_file_contains_regex() {
    local file_path="$1"
    local pattern="$2"
    if command -v rg >/dev/null 2>&1; then
        rg -q "$pattern" "$file_path"
        return $?
    fi
    grep -Eq -- "$pattern" "$file_path"
}

mfx_assert_file_contains() {
    local file_path="$1"
    local pattern="$2"
    local context="$3"
    if ! mfx_file_contains_fixed "$file_path" "$pattern"; then
        mfx_fail "$context: missing pattern '$pattern' in $file_path"
    fi
}

mfx_http_code() {
    local output_file="$1"
    local url="$2"
    shift 2
    curl -sS -o "$output_file" -w '%{http_code}' "$@" "$url"
}

mfx_terminate_stale_entry_host() {
    local context="${1:-}"
    if command -v pgrep >/dev/null 2>&1 && command -v pkill >/dev/null 2>&1; then
        if pgrep -f mfx_entry_posix_host >/dev/null 2>&1; then
            local message="terminate stale mfx_entry_posix_host processes"
            if [[ -n "$context" ]]; then
                message="$message $context"
            fi
            mfx_info "$message"
            pkill -f mfx_entry_posix_host >/dev/null 2>&1 || true
            sleep 0.2
        fi
    fi
}

mfx_acquire_lock() {
    local lock_name="$1"
    local timeout_seconds="${2:-180}"
    if [[ -z "$lock_name" ]]; then
        mfx_fail "lock name is required"
    fi
    if ! [[ "$timeout_seconds" =~ ^[0-9]+$ ]]; then
        mfx_fail "lock timeout must be a non-negative integer"
    fi

    local lock_root="${TMPDIR:-/tmp}/mfx-regression-locks"
    local lock_dir="$lock_root/${lock_name}.lock"
    mkdir -p "$lock_root"

    if [[ -n "$_mfx_lock_dir" && "$_mfx_lock_dir" != "$lock_dir" ]]; then
        mfx_fail "nested lock acquire is not supported: current=$_mfx_lock_name requested=$lock_name"
    fi
    if [[ "$_mfx_lock_dir" == "$lock_dir" ]]; then
        return 0
    fi

    local deadline=$((SECONDS + timeout_seconds))
    local announced_wait=0
    while true; do
        if mkdir "$lock_dir" 2>/dev/null; then
            _mfx_lock_dir="$lock_dir"
            _mfx_lock_name="$lock_name"
            {
                printf 'pid=%s\n' "$$"
                printf 'script=%s\n' "${BASH_SOURCE[1]:-unknown}"
            } >"$lock_dir/owner.env"
            return 0
        fi

        local owner_pid=""
        if [[ -f "$lock_dir/owner.env" ]]; then
            owner_pid="$(sed -n 's/^pid=//p' "$lock_dir/owner.env" | head -n 1)"
        fi
        if [[ -n "$owner_pid" && "$owner_pid" =~ ^[0-9]+$ ]]; then
            if ! kill -0 "$owner_pid" 2>/dev/null; then
                mfx_info "remove stale lock: $lock_name (owner pid=$owner_pid)"
                rm -rf "$lock_dir" || true
                continue
            fi
        fi

        if (( SECONDS >= deadline )); then
            mfx_fail "lock timeout: $lock_name (waited ${timeout_seconds}s)"
        fi
        if [[ "$announced_wait" -eq 0 ]]; then
            mfx_info "waiting for lock: $lock_name"
            announced_wait=1
        fi
        sleep 0.2
    done
}

mfx_release_lock() {
    if [[ -z "$_mfx_lock_dir" ]]; then
        return 0
    fi
    rm -rf "$_mfx_lock_dir" || true
    _mfx_lock_dir=""
    _mfx_lock_name=""
}

mfx_with_lock() {
    local lock_name="$1"
    local timeout_seconds="${2:-180}"
    shift 2
    if [[ $# -eq 0 ]]; then
        mfx_fail "mfx_with_lock requires a command"
    fi

    mfx_acquire_lock "$lock_name" "$timeout_seconds"
    local status=0
    "$@" || status=$?
    mfx_release_lock
    return "$status"
}
