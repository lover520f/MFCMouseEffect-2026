#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

source "$SCRIPT_DIR/lib/common.sh"
source "$SCRIPT_DIR/lib/build.sh"
source "$SCRIPT_DIR/lib/core_http.sh"

mfx_detect_host_platform() {
    case "$(uname -s)" in
        Darwin) echo "macos" ;;
        Linux) echo "linux" ;;
        *)
            mfx_fail "unsupported host platform: $(uname -s). expected macOS or Linux."
            ;;
    esac
}

MFX_PLATFORM="auto"
MFX_BUILD_DIR=""
MFX_HOST_PLATFORM="$(mfx_detect_host_platform)"

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
Usage: run-posix-core-wasm-contract-regression.sh [options]
  --platform <auto|macos|linux>   target platform package (default: auto)
  --build-dir <path>              build directory override

Env tuning:
  MFX_CORE_HTTP_CHECK_SCOPE             forced to wasm by this script
  MFX_CORE_HTTP_START_WAIT_SECONDS      wait time before startup alive check (default: 1)
  MFX_CORE_HTTP_PROBE_TIMEOUT_SECONDS   wait time for websettings probe file (default: 8)
  MFX_ENABLE_WASM_TEST_DISPATCH_API     enable test dispatch endpoint during regression (default: 1 in script)
USAGE
            exit 0
            ;;
        *)
            mfx_fail "unknown argument: $1"
            ;;
    esac
done

if [[ "$MFX_PLATFORM" == "auto" ]]; then
    MFX_PLATFORM="$MFX_HOST_PLATFORM"
fi
if [[ "$MFX_PLATFORM" != "macos" && "$MFX_PLATFORM" != "linux" ]]; then
    mfx_fail "invalid --platform value: $MFX_PLATFORM"
fi
if [[ "$MFX_PLATFORM" != "$MFX_HOST_PLATFORM" ]]; then
    mfx_fail "cross-host core wasm contract run is unsupported (host=$MFX_HOST_PLATFORM, requested=$MFX_PLATFORM)"
fi

if [[ "$MFX_PLATFORM" != "macos" ]]; then
    mfx_info "core wasm contract regression is macOS-only in current roadmap; skip on platform=$MFX_PLATFORM"
    exit 0
fi

if [[ -z "$MFX_BUILD_DIR" ]]; then
    MFX_BUILD_DIR="/tmp/mfx-platform-${MFX_PLATFORM}-core-wasm-contract-build"
fi

mfx_require_cmd cmake
mfx_require_cmd curl
mfx_require_cmd rg

mfx_info "repo root: $REPO_ROOT"
mfx_info "platform: $MFX_PLATFORM"
mfx_info "build dir: $MFX_BUILD_DIR"
mfx_info "check scope: wasm"
mfx_info "enable core runtime lane: ON"
mfx_info "entry host lock: mfx-entry-posix-host"

mfx_run_core_wasm_contract_workflow() {
    mfx_terminate_stale_entry_host "before core wasm contracts"

    mfx_configure_and_build_entry_host \
        "$REPO_ROOT" \
        "$MFX_BUILD_DIR" \
        "$MFX_PLATFORM" \
        "-DMFX_ENABLE_POSIX_CORE_RUNTIME=ON"

    mfx_run_core_http_contract_checks "$MFX_PLATFORM" "$MFX_BUILD_DIR" "wasm"
    mfx_ok "posix core wasm contract regression passed"
}

MFX_ENTRY_LOCK_TIMEOUT_SECONDS="${MFX_ENTRY_LOCK_TIMEOUT_SECONDS:-180}"
mfx_with_lock "mfx-entry-posix-host" "$MFX_ENTRY_LOCK_TIMEOUT_SECONDS" \
    mfx_run_core_wasm_contract_workflow
