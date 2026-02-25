#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

source "$SCRIPT_DIR/lib/common.sh"
source "$SCRIPT_DIR/lib/build.sh"
source "$SCRIPT_DIR/lib/core_http.sh"

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
Usage: run-posix-core-wasm-path-contract-regression.sh [options]
  --platform <auto|macos|linux>   target platform package (default: auto)
  --build-dir <path>              build directory override

Env tuning:
  MFX_CORE_HTTP_CHECK_SCOPE                   forced to wasm by this script
  MFX_CORE_HTTP_WASM_CHECK_PROFILE            forced to path by this script
  MFX_CORE_HTTP_START_WAIT_SECONDS            wait time before startup alive check (default: 1)
  MFX_CORE_HTTP_PROBE_TIMEOUT_SECONDS         wait time for websettings probe file (default: 8)
USAGE
            exit 0
            ;;
        *)
            mfx_fail "unknown argument: $1"
            ;;
    esac
done

MFX_PLATFORM="$(mfx_resolve_posix_platform "$MFX_PLATFORM" "$MFX_HOST_PLATFORM" "core wasm path contract run")"

if [[ "$MFX_PLATFORM" != "macos" ]]; then
    mfx_info "core wasm path contract regression is macOS-only in current roadmap; skip on platform=$MFX_PLATFORM"
    exit 0
fi

if [[ -z "$MFX_BUILD_DIR" ]]; then
    MFX_BUILD_DIR="/tmp/mfx-platform-${MFX_PLATFORM}-core-wasm-path-contract-build"
fi

mfx_require_cmd cmake
mfx_require_cmd curl

mfx_info "repo root: $REPO_ROOT"
mfx_info "platform: $MFX_PLATFORM"
mfx_info "build dir: $MFX_BUILD_DIR"
mfx_info "check scope: wasm"
mfx_info "wasm check profile: path"
mfx_info "enable core runtime lane: ON"
mfx_info "entry host lock: mfx-entry-posix-host"

mfx_run_core_wasm_path_contract_workflow() {
    mfx_prepare_core_entry_runtime "core wasm path contracts" "$REPO_ROOT" "$MFX_BUILD_DIR" "$MFX_PLATFORM"

    MFX_CORE_HTTP_WASM_CHECK_PROFILE="path" \
        mfx_run_core_http_contract_checks "$MFX_PLATFORM" "$MFX_BUILD_DIR" "wasm"
    mfx_ok "posix core wasm path contract regression passed"
}

mfx_run_with_entry_lock mfx_run_core_wasm_path_contract_workflow
