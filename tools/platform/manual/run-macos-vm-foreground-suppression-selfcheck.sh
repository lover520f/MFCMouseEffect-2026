#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../../.." && pwd)"

source "$repo_root/tools/platform/regression/lib/common.sh"
source "$repo_root/tools/platform/regression/lib/build.sh"
source "$repo_root/tools/platform/manual/lib/macos_core_host.sh"

usage() {
    cat <<'EOF'
Usage:
  tools/platform/manual/run-macos-vm-foreground-suppression-selfcheck.sh [options]

Options:
  --build-dir <path>          Build directory (default: /tmp/mfx-platform-macos-core-build)
  --log-prefix <path>         Host log file prefix (default: /tmp/mfx-core-vm-suppression-selfcheck)
  --probe-prefix <path>       Probe file prefix (default: /tmp/mfx-core-vm-suppression-selfcheck)
  --check-interval-ms <num>   Override MFX_VM_FOREGROUND_SUPPRESSION_CHECK_INTERVAL_MS for test run (default: 50)
  --jobs <num>                Build jobs (sets MFX_BUILD_JOBS)
  --skip-build                Skip cmake configure/build
  -h, --help                  Show this help
EOF
}

build_dir="/tmp/mfx-platform-macos-core-build"
log_prefix="/tmp/mfx-core-vm-suppression-selfcheck"
probe_prefix="/tmp/mfx-core-vm-suppression-selfcheck"
check_interval_ms="50"
skip_build=0
build_jobs=""

while [[ $# -gt 0 ]]; do
    case "$1" in
    --build-dir)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --build-dir"
        build_dir="$2"
        shift 2
        ;;
    --log-prefix)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --log-prefix"
        log_prefix="$2"
        shift 2
        ;;
    --probe-prefix)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --probe-prefix"
        probe_prefix="$2"
        shift 2
        ;;
    --check-interval-ms)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --check-interval-ms"
        check_interval_ms="$2"
        shift 2
        ;;
    --jobs)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --jobs"
        build_jobs="$2"
        shift 2
        ;;
    --skip-build)
        skip_build=1
        shift
        ;;
    -h|--help)
        usage
        exit 0
        ;;
    *)
        mfx_fail "unknown argument: $1"
        ;;
    esac
done

if [[ "$OSTYPE" != darwin* ]]; then
    mfx_fail "this script is macOS-only"
fi

mfx_manual_apply_build_jobs_env "$build_jobs" "--jobs"
mfx_require_cmd curl
if ! [[ "$check_interval_ms" =~ ^[0-9]+$ ]] || [[ "$check_interval_ms" -le 0 ]]; then
    mfx_fail "--check-interval-ms must be a positive integer"
fi

tmp_dir="$(mktemp -d)"
cleanup() {
    mfx_manual_stop_core_host "$MFX_MANUAL_HOST_PID"
    rm -rf "$tmp_dir" >/dev/null 2>&1 || true
    mfx_release_lock
}
trap cleanup EXIT

wait_vm_suppression_state() {
    local expected="$1"
    local output_file="$2"
    local context="$3"
    local token_header="x-mfcmouseeffect-token: $MFX_MANUAL_SETTINGS_TOKEN"
    local retries=80

    while (( retries > 0 )); do
        local code
        code="$(mfx_http_code "$output_file" "$MFX_MANUAL_BASE_URL/api/state" -X GET -H "$token_header")"
        if [[ "$code" == "200" ]] && mfx_file_contains_fixed "$output_file" "\"effects_suspended_vm\":$expected"; then
            return 0
        fi
        retries=$((retries - 1))
        sleep 0.1
    done

    cat "$output_file" >&2 || true
    mfx_fail "$context: expected effects_suspended_vm=$expected"
}

run_case() {
    local force_value="$1"
    local expected_state="$2"
    local case_name="$3"
    local log_file="${log_prefix}-${case_name}.log"
    local probe_file="${probe_prefix}-${case_name}.probe"
    local state_file="$tmp_dir/state-${case_name}.out"

    rm -f "$log_file" "$probe_file" "$state_file"
    MFX_MANUAL_HOST_PID=""
    MFX_MANUAL_SETTINGS_URL=""
    MFX_MANUAL_SETTINGS_TOKEN=""
    MFX_MANUAL_BASE_URL=""
    MFX_MANUAL_LOG_FILE=""

    declare -a host_env
    host_env+=("MFX_TEST_INPUT_CAPTURE_PERMISSION_SIM_FILE=$permission_sim_file")
    host_env+=("MFX_VM_FOREGROUND_SUPPRESSION_FORCE=$force_value")
    host_env+=("MFX_VM_FOREGROUND_SUPPRESSION_CHECK_INTERVAL_MS=$check_interval_ms")
    mfx_manual_start_core_host "$host_bin" "$probe_file" "$log_file" "${host_env[@]}"
    wait_vm_suppression_state "$expected_state" "$state_file" "vm suppression selfcheck ($case_name)"
    mfx_manual_stop_core_host "$MFX_MANUAL_HOST_PID"
}

mfx_manual_acquire_entry_host_lock
mfx_manual_prepare_core_host_binary "$repo_root" "$build_dir" "$skip_build"
host_bin="$MFX_MANUAL_HOST_BIN"

permission_sim_file="$tmp_dir/input-permission.sim"
printf '1\n' > "$permission_sim_file"

run_case "1" "true" "force-on"
run_case "0" "false" "force-off"

mfx_ok "macos vm foreground suppression selfcheck passed"
