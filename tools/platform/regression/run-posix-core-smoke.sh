#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

source "$SCRIPT_DIR/lib/common.sh"
source "$SCRIPT_DIR/lib/build.sh"
source "$SCRIPT_DIR/lib/core_smoke.sh"

MFX_PLATFORM="auto"
MFX_BUILD_DIR=""
MFX_HOST_PLATFORM="$(mfx_detect_posix_host_platform)"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --platform)
            MFX_PLATFORM="${2:-}"
            shift 2
            ;;
        --build-dir)
            MFX_BUILD_DIR="${2:-}"
            shift 2
            ;;
        -h|--help)
            cat <<'USAGE'
Usage: run-posix-core-smoke.sh [options]
  --platform <auto|macos|linux>   target platform package (default: auto)
  --build-dir <path>              build directory override

Env tuning:
  MFX_CORE_SMOKE_START_WAIT_SECONDS   wait time before startup alive check (default: 1)
  MFX_CORE_SMOKE_ALIVE_SECONDS        required alive window before exit check (default: 1)
  MFX_CORE_SMOKE_EXIT_TIMEOUT_SECONDS max graceful exit wait after stdin exit command (default: 5)
USAGE
            exit 0
            ;;
        *)
            mfx_fail "unknown argument: $1"
            ;;
    esac
done

MFX_PLATFORM="$(mfx_resolve_posix_platform "$MFX_PLATFORM" "$MFX_HOST_PLATFORM" "core smoke run")"

if [[ -z "$MFX_BUILD_DIR" ]]; then
    MFX_BUILD_DIR="/tmp/mfx-platform-${MFX_PLATFORM}-core-build"
fi

mfx_require_cmd cmake

mfx_info "repo root: $REPO_ROOT"
mfx_info "platform: $MFX_PLATFORM"
mfx_info "build dir: $MFX_BUILD_DIR"
mfx_info "enable core runtime lane: ON"
mfx_info "entry host lock: mfx-entry-posix-host"

mfx_run_core_smoke_workflow() {
    mfx_terminate_stale_entry_host "before core smoke"

    mfx_configure_and_build_entry_host \
        "$REPO_ROOT" \
        "$MFX_BUILD_DIR" \
        "$MFX_PLATFORM" \
        "-DMFX_ENABLE_POSIX_CORE_RUNTIME=ON"

    mfx_run_core_lane_smoke "$MFX_PLATFORM" "$MFX_BUILD_DIR"
    mfx_ok "posix core-lane smoke regression passed"
}

MFX_ENTRY_LOCK_TIMEOUT_SECONDS="${MFX_ENTRY_LOCK_TIMEOUT_SECONDS:-180}"
mfx_with_lock "mfx-entry-posix-host" "$MFX_ENTRY_LOCK_TIMEOUT_SECONDS" \
    mfx_run_core_smoke_workflow
