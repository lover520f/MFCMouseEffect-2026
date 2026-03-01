#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

source "$SCRIPT_DIR/lib/common.sh"
source "$SCRIPT_DIR/lib/build.sh"
source "$SCRIPT_DIR/lib/smoke.sh"
source "$SCRIPT_DIR/lib/http.sh"

MFX_PLATFORM="auto"
MFX_BUILD_DIR=""
MFX_SKIP_HTTP=0
MFX_SKIP_SMOKE=0
MFX_HOST_PLATFORM="$(mfx_detect_posix_host_platform)"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --platform)
            mfx_require_option_value "$1" "${2:-}"
            MFX_PLATFORM="${2:-}"
            shift 2
            ;;
        --build-dir)
            mfx_require_option_value "$1" "${2:-}"
            MFX_BUILD_DIR="${2:-}"
            shift 2
            ;;
        --skip-http)
            MFX_SKIP_HTTP=1
            shift
            ;;
        --skip-smoke)
            MFX_SKIP_SMOKE=1
            shift
            ;;
        -h|--help)
            cat <<'USAGE'
Usage: run-posix-scaffold-regression.sh [options]
  --platform <auto|macos|linux>   target platform package (default: auto)
  --build-dir <path>              build directory override
  --skip-smoke                    skip smoke checks
  --skip-http                     skip scaffold HTTP checks

Env tuning:
  MFX_HTTP_ENTRY_START_RETRIES     scaffold HTTP entry startup retries on early exit (default: 1)
USAGE
            exit 0
            ;;
        *)
            mfx_fail "unknown argument: $1"
            ;;
    esac
done

MFX_PLATFORM="$(mfx_resolve_posix_platform "$MFX_PLATFORM" "$MFX_HOST_PLATFORM" "regression run")"

if [[ -z "$MFX_BUILD_DIR" ]]; then
    MFX_BUILD_DIR="/tmp/mfx-platform-${MFX_PLATFORM}-build"
fi

mfx_require_cmd cmake
mfx_require_cmd curl

mfx_info "repo root: $REPO_ROOT"
mfx_info "platform: $MFX_PLATFORM"
mfx_info "build dir: $MFX_BUILD_DIR"
mfx_info "entry host lock: mfx-entry-posix-host"

mfx_run_scaffold_regression_checks() {
    mfx_terminate_stale_entry_host "before scaffold regression"
    mfx_configure_and_build \
        "$REPO_ROOT" \
        "$MFX_BUILD_DIR" \
        "$MFX_PLATFORM" \
        "-DMFX_ENABLE_POSIX_CORE_RUNTIME=OFF"

    if [[ "$MFX_SKIP_SMOKE" -eq 0 ]]; then
        mfx_run_smoke_checks "$MFX_PLATFORM" "$MFX_BUILD_DIR"
    else
        mfx_info "skip smoke checks"
    fi

    if [[ "$MFX_SKIP_HTTP" -eq 0 ]]; then
        mfx_run_http_checks "$MFX_PLATFORM" "$MFX_BUILD_DIR"
    else
        mfx_info "skip HTTP checks"
    fi
}

mfx_run_with_entry_lock mfx_run_scaffold_regression_checks

mfx_ok "posix scaffold regression passed"
