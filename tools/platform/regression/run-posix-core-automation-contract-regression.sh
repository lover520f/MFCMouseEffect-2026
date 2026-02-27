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
MFX_CHECK_SCOPE="all"

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
        --check-scope)
            mfx_require_option_value "$1" "${2:-}"
            MFX_CHECK_SCOPE="${2:-}"
            shift 2
            ;;
        --check-scope=*)
            MFX_CHECK_SCOPE="${1#*=}"
            shift
            ;;
        -h|--help)
            cat <<'USAGE'
Usage: run-posix-core-automation-contract-regression.sh [options]
  --platform <auto|macos|linux>   target platform package (default: auto)
  --build-dir <path>              build directory override
  --check-scope <all|wasm|effects> check scope (default: all)

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
  MFX_CORE_HTTP_WASM_DISPATCH_TIMEOUT_SECONDS max wait for invoke/render ready in test-dispatch assertion (default: 5)
  MFX_CORE_HTTP_WASM_DISPATCH_RETRY_INTERVAL_SECONDS retry interval for test-dispatch assertion (default: 0.2)
USAGE
            exit 0
            ;;
        *)
            mfx_fail "unknown argument: $1"
            ;;
    esac
done

MFX_PLATFORM="$(mfx_resolve_posix_platform "$MFX_PLATFORM" "$MFX_HOST_PLATFORM" "core automation contract run")"
MFX_CHECK_SCOPE="$(mfx_normalize_core_automation_check_scope "$MFX_CHECK_SCOPE" "--check-scope")"

if [[ "$MFX_PLATFORM" != "macos" ]]; then
    mfx_info "core automation HTTP contracts are macOS-only in current roadmap; skip on platform=$MFX_PLATFORM"
    exit 0
fi

if [[ -z "$MFX_BUILD_DIR" ]]; then
    MFX_BUILD_DIR="/tmp/mfx-platform-${MFX_PLATFORM}-core-automation-build"
fi

mfx_require_cmd cmake
mfx_require_cmd curl

mfx_info "repo root: $REPO_ROOT"
mfx_info "platform: $MFX_PLATFORM"
mfx_info "build dir: $MFX_BUILD_DIR"
mfx_info "enable core runtime lane: ON"
mfx_info "entry host lock: mfx-entry-posix-host"
mfx_info "check scope: $MFX_CHECK_SCOPE"

mfx_run_core_automation_contract_workflow() {
    mfx_prepare_core_entry_runtime "core automation contracts" "$REPO_ROOT" "$MFX_BUILD_DIR" "$MFX_PLATFORM"

    mfx_run_core_http_contract_checks "$MFX_PLATFORM" "$MFX_BUILD_DIR" "$MFX_CHECK_SCOPE"
    mfx_ok "posix core automation contract regression passed"
}

mfx_run_with_entry_lock mfx_run_core_automation_contract_workflow
