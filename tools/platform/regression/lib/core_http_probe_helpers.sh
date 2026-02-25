#!/usr/bin/env bash

set -euo pipefail

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
