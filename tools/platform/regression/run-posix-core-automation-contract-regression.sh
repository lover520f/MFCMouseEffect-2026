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
Usage: run-posix-core-automation-contract-regression.sh [options]
  --platform <auto|macos|linux>   target platform package (default: auto)
  --build-dir <path>              build directory override

Env tuning:
  MFX_CORE_HTTP_START_WAIT_SECONDS    wait time before startup alive check (default: 1)
  MFX_CORE_HTTP_PROBE_TIMEOUT_SECONDS wait time for websettings probe file (default: 8)
  MFX_ENABLE_WASM_TEST_DISPATCH_API   enable test dispatch endpoint during regression (default: 1 in script)
  MFX_ENABLE_AUTOMATION_SCOPE_TEST_API enable app-scope/binding-priority test endpoints during regression (default: 1 in script)
  MFX_ENABLE_AUTOMATION_SHORTCUT_TEST_API enable mac keycode->shortcut test endpoint (default: 1 in script)
  MFX_ENABLE_AUTOMATION_INJECTION_TEST_API enable shortcut injector probe endpoint (default: 1 in script)
  MFX_ENABLE_INPUT_INDICATOR_TEST_API  enable input-indicator label probe endpoint (default: 1 in script)
  MFX_TEST_KEYBOARD_INJECTOR_DRY_RUN   enable keyboard injector dry-run mode for deterministic contract checks (default: 1 in script)
  MFX_CORE_WEB_SETTINGS_LAUNCH_PROBE_FILE optional launch-probe output file used by the script
  MFX_TEST_SETTINGS_LAUNCH_CAPTURE_FILE optional launcher-capture output file used by the script
  MFX_TEST_INPUT_CAPTURE_PERMISSION_SIM_FILE optional mac input-permission simulation file used by the script
  MFX_TEST_NOTIFICATION_CAPTURE_FILE optional shell notification capture file used by the script
  MFX_CORE_HTTP_INPUT_CAPTURE_TIMEOUT_SECONDS timeout for permission transition waits (default: 10)
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
    mfx_fail "cross-host core automation contract run is unsupported (host=$MFX_HOST_PLATFORM, requested=$MFX_PLATFORM)"
fi

if [[ "$MFX_PLATFORM" != "macos" ]]; then
    mfx_info "core automation HTTP contracts are macOS-only in current roadmap; skip on platform=$MFX_PLATFORM"
    exit 0
fi

if [[ -z "$MFX_BUILD_DIR" ]]; then
    MFX_BUILD_DIR="/tmp/mfx-platform-${MFX_PLATFORM}-core-automation-build"
fi

mfx_require_cmd cmake
mfx_require_cmd curl
mfx_require_cmd rg

mfx_info "repo root: $REPO_ROOT"
mfx_info "platform: $MFX_PLATFORM"
mfx_info "build dir: $MFX_BUILD_DIR"
mfx_info "enable core runtime lane: ON"

if command -v pgrep >/dev/null 2>&1 && command -v pkill >/dev/null 2>&1; then
    if pgrep -f mfx_entry_posix_host >/dev/null 2>&1; then
        mfx_info "terminate stale mfx_entry_posix_host processes before core automation contracts"
        pkill -f mfx_entry_posix_host >/dev/null 2>&1 || true
        sleep 0.2
    fi
fi

mfx_configure_and_build_entry_host \
    "$REPO_ROOT" \
    "$MFX_BUILD_DIR" \
    "$MFX_PLATFORM" \
    "-DMFX_ENABLE_POSIX_CORE_RUNTIME=ON"

mfx_run_core_http_contract_checks "$MFX_PLATFORM" "$MFX_BUILD_DIR"

mfx_ok "posix core automation contract regression passed"
