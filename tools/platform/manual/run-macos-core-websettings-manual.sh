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
  tools/platform/manual/run-macos-core-websettings-manual.sh [options]

Options:
  --build-dir <path>          Build directory (default: /tmp/mfx-platform-macos-core-build)
  --auto-stop-seconds <num>   Auto stop host after N seconds (default: 120, 0 = disable)
  --log-file <path>           Host log path (default: /tmp/mfx-core-manual.log)
  --probe-file <path>         Probe file path (default: /tmp/mfx-core-websettings.probe)
  --jobs <num>                Build jobs (sets MFX_BUILD_JOBS)
  --skip-build                Skip cmake configure/build
  --no-open                   Do not open browser automatically
  -h, --help                  Show this help
EOF
}

build_dir="/tmp/mfx-platform-macos-core-build"
auto_stop_seconds=120
log_file="/tmp/mfx-core-manual.log"
probe_file="/tmp/mfx-core-websettings.probe"
skip_build=0
open_browser=1
build_jobs=""

while [[ $# -gt 0 ]]; do
    case "$1" in
    --build-dir)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --build-dir"
        build_dir="$2"
        shift 2
        ;;
    --auto-stop-seconds)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --auto-stop-seconds"
        auto_stop_seconds="$2"
        shift 2
        ;;
    --log-file)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --log-file"
        log_file="$2"
        shift 2
        ;;
    --probe-file)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --probe-file"
        probe_file="$2"
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
    --no-open)
        open_browser=0
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

if ! [[ "$auto_stop_seconds" =~ ^[0-9]+$ ]]; then
    mfx_fail "--auto-stop-seconds must be a non-negative integer"
fi

if [[ -n "$build_jobs" ]]; then
    if ! [[ "$build_jobs" =~ ^[0-9]+$ ]]; then
        mfx_fail "--jobs must be a positive integer"
    fi
    export MFX_BUILD_JOBS="$build_jobs"
fi

mfx_require_cmd cmake
mfx_require_cmd sed

if [[ "$skip_build" -eq 0 ]]; then
    mfx_configure_platform_build_dir "$repo_root" "$build_dir" "macos" \
        -DMFX_ENABLE_CROSS_HOST_PACKAGES=ON \
        -DMFX_ENABLE_ENTRY_RUNTIME_TARGETS=ON \
        -DMFX_ENABLE_POSIX_CORE_RUNTIME=ON
    mfx_build_targets "$build_dir" "mfx_entry_posix_host"
fi

host_bin="$build_dir/mfx_entry_posix_host"
mfx_manual_acquire_entry_host_lock
cleanup_lock() {
    mfx_release_lock
}
trap cleanup_lock EXIT

mfx_manual_start_core_host "$host_bin" "$probe_file" "$log_file"

printf 'mfx_pid=%s\n' "$MFX_MANUAL_HOST_PID"
printf 'settings_url=%s\n' "$MFX_MANUAL_SETTINGS_URL"
printf 'log_file=%s\n' "$MFX_MANUAL_LOG_FILE"

if [[ "$open_browser" -eq 1 ]]; then
    mfx_require_cmd open
    open "$MFX_MANUAL_SETTINGS_URL" >/dev/null 2>&1 || mfx_fail "failed to open settings url"
fi

if [[ "$auto_stop_seconds" -gt 0 ]]; then
    mfx_manual_schedule_auto_stop "$MFX_MANUAL_HOST_PID" "$auto_stop_seconds"
    printf 'auto_stop_seconds=%s\n' "$auto_stop_seconds"
fi

mfx_ok "manual websettings runner ready"
