#!/usr/bin/env bash

set -euo pipefail

_mfx_core_smoke_lib_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$_mfx_core_smoke_lib_dir/core_smoke_entry_helpers.sh"

mfx_run_core_lane_smoke() {
    local platform="$1"
    local build_dir="$2"
    local entry_bin="$build_dir/mfx_entry_posix_host"

    if [[ ! -x "$entry_bin" ]]; then
        mfx_fail "core smoke entry host executable missing: $entry_bin"
    fi

    local tmp_dir
    tmp_dir="$(mktemp -d)"
    local log_file="$tmp_dir/core-smoke.log"
    trap "_mfx_core_smoke_stop_entry; rm -rf '$tmp_dir'" EXIT

    mfx_info "run core lane smoke entry on platform: $platform"
    _mfx_core_smoke_start_entry "$entry_bin" "$log_file"

    sleep "${MFX_CORE_SMOKE_ALIVE_SECONDS:-1}"
    if ! kill -0 "$_mfx_core_smoke_entry_pid" >/dev/null 2>&1; then
        mfx_info "core smoke runtime log:"
        cat "$log_file" || true
        mfx_fail "core smoke entry exited during alive window"
    fi

    trap - EXIT
    _mfx_core_smoke_stop_entry
    rm -rf "$tmp_dir"
    mfx_ok "core lane smoke checks completed"
}
