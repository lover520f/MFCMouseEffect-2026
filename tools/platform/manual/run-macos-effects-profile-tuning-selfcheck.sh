#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../../.." && pwd)"

source "$repo_root/tools/platform/regression/lib/common.sh"
source "$repo_root/tools/platform/regression/lib/build.sh"
source "$repo_root/tools/platform/regression/lib/core_http_automation_parse_helpers.sh"
source "$repo_root/tools/platform/manual/lib/macos_core_host.sh"

usage() {
    cat <<'EOF'
Usage:
  tools/platform/manual/run-macos-effects-profile-tuning-selfcheck.sh [options]

Options:
  --build-dir <path>            Build directory (default: /tmp/mfx-platform-macos-core-build)
  --log-file <path>             Host log path (default: /tmp/mfx-core-effects-profile-selfcheck.log)
  --probe-file <path>           Probe file path (default: /tmp/mfx-core-effects-profile-selfcheck.probe)
  --jobs <num>                  Build jobs (sets MFX_BUILD_JOBS)
  --skip-build                  Skip cmake configure/build
  --duration-scale <value>      Override MFX_TEST_EFFECTS_DURATION_SCALE (default: 0.5)
  --size-scale <value>          Override MFX_TEST_EFFECTS_SIZE_SCALE (default: 1.2)
  --opacity-scale <value>       Override MFX_TEST_EFFECTS_OPACITY_SCALE (default: 0.8)
  --trail-throttle-scale <val>  Override MFX_TEST_EFFECTS_TRAIL_THROTTLE_SCALE (default: 0.6)
  --keep-running                Keep host running after selfcheck
  --auto-stop-seconds <num>     Auto stop host after N seconds (only with --keep-running, default: 120)
  -h, --help                    Show this help
EOF
}

build_dir="/tmp/mfx-platform-macos-core-build"
log_file="/tmp/mfx-core-effects-profile-selfcheck.log"
probe_file="/tmp/mfx-core-effects-profile-selfcheck.probe"
skip_build=0
keep_running=0
auto_stop_seconds=120
build_jobs=""
duration_scale="0.5"
size_scale="1.2"
opacity_scale="0.8"
trail_throttle_scale="0.6"

assert_float_within_tolerance() {
    local actual="$1"
    local expected="$2"
    local tolerance="$3"
    local label="$4"
    if [[ -z "$actual" || -z "$expected" || -z "$tolerance" ]]; then
        mfx_fail "$label: missing float comparison input"
    fi
    if ! awk -v a="$actual" -v b="$expected" -v tol="$tolerance" 'BEGIN {
        d = a - b;
        if (d < 0) {
            d = -d;
        }
        exit(d <= tol ? 0 : 1);
    }'; then
        mfx_fail "$label: expected $expected (tol=$tolerance), got $actual"
    fi
}

while [[ $# -gt 0 ]]; do
    case "$1" in
    --build-dir)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --build-dir"
        build_dir="$2"
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
    --duration-scale)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --duration-scale"
        duration_scale="$2"
        shift 2
        ;;
    --size-scale)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --size-scale"
        size_scale="$2"
        shift 2
        ;;
    --opacity-scale)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --opacity-scale"
        opacity_scale="$2"
        shift 2
        ;;
    --trail-throttle-scale)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --trail-throttle-scale"
        trail_throttle_scale="$2"
        shift 2
        ;;
    --keep-running)
        keep_running=1
        shift
        ;;
    --auto-stop-seconds)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --auto-stop-seconds"
        auto_stop_seconds="$2"
        shift 2
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
mfx_manual_validate_non_negative_integer "$auto_stop_seconds" "--auto-stop-seconds"

mfx_require_cmd curl

tmp_dir="$(mktemp -d)"
cleanup() {
    rm -rf "$tmp_dir" >/dev/null 2>&1 || true
    mfx_release_lock
    if [[ "$keep_running" -eq 0 ]]; then
        mfx_manual_stop_core_host "$MFX_MANUAL_HOST_PID"
    fi
}
trap cleanup EXIT

mfx_manual_acquire_entry_host_lock

mfx_manual_prepare_core_host_binary "$repo_root" "$build_dir" "$skip_build"
host_bin="$MFX_MANUAL_HOST_BIN"

declare -a host_env
host_env+=("MFX_ENABLE_EFFECT_OVERLAY_TEST_API=1")
host_env+=("MFX_TEST_EFFECTS_DURATION_SCALE=$duration_scale")
host_env+=("MFX_TEST_EFFECTS_SIZE_SCALE=$size_scale")
host_env+=("MFX_TEST_EFFECTS_OPACITY_SCALE=$opacity_scale")
host_env+=("MFX_TEST_EFFECTS_TRAIL_THROTTLE_SCALE=$trail_throttle_scale")
mfx_manual_start_core_host "$host_bin" "$probe_file" "$log_file" "${host_env[@]}"

printf 'mfx_pid=%s\n' "$MFX_MANUAL_HOST_PID"
printf 'settings_url=%s\n' "$MFX_MANUAL_SETTINGS_URL"
printf 'log_file=%s\n' "$MFX_MANUAL_LOG_FILE"

token="$MFX_MANUAL_SETTINGS_TOKEN"
token_header="x-mfcmouseeffect-token: $token"

profile_probe_file="$tmp_dir/effects-profile-probe.out"
probe_code="$(mfx_http_code "$profile_probe_file" "$MFX_MANUAL_BASE_URL/api/effects/test-render-profiles" \
    -X GET \
    -H "$token_header")"
mfx_assert_eq "$probe_code" "200" "effects profile test route status"
mfx_assert_file_contains "$profile_probe_file" "\"ok\":true" "effects profile test route ok"
mfx_assert_file_contains "$profile_probe_file" "\"duration_scale\":" "effects profile duration scale field"
mfx_assert_file_contains "$profile_probe_file" "\"size_scale\":" "effects profile size scale field"
mfx_assert_file_contains "$profile_probe_file" "\"opacity_scale\":" "effects profile opacity scale field"
mfx_assert_file_contains "$profile_probe_file" "\"trail_throttle_scale\":" "effects profile trail throttle scale field"
mfx_assert_file_contains "$profile_probe_file" "\"duration_overridden\":true" "effects profile duration override marker"
mfx_assert_file_contains "$profile_probe_file" "\"size_overridden\":true" "effects profile size override marker"
mfx_assert_file_contains "$profile_probe_file" "\"opacity_overridden\":true" "effects profile opacity override marker"
mfx_assert_file_contains "$profile_probe_file" "\"trail_throttle_overridden\":true" "effects profile trail throttle override marker"

probe_duration_scale="$(_mfx_core_http_automation_parse_scalar_field "$profile_probe_file" "duration_scale")"
probe_size_scale="$(_mfx_core_http_automation_parse_scalar_field "$profile_probe_file" "size_scale")"
probe_opacity_scale="$(_mfx_core_http_automation_parse_scalar_field "$profile_probe_file" "opacity_scale")"
probe_trail_throttle_scale="$(_mfx_core_http_automation_parse_scalar_field "$profile_probe_file" "trail_throttle_scale")"
assert_float_within_tolerance "$probe_duration_scale" "$duration_scale" "0.0001" "effects profile duration scale"
assert_float_within_tolerance "$probe_size_scale" "$size_scale" "0.0001" "effects profile size scale"
assert_float_within_tolerance "$probe_opacity_scale" "$opacity_scale" "0.0001" "effects profile opacity scale"
assert_float_within_tolerance "$probe_trail_throttle_scale" "$trail_throttle_scale" "0.0001" "effects profile trail throttle scale"

state_file="$tmp_dir/effects-profile-state.out"
state_code="$(mfx_http_code "$state_file" "$MFX_MANUAL_BASE_URL/api/state" \
    -X GET \
    -H "$token_header")"
mfx_assert_eq "$state_code" "200" "effects profile state status"
mfx_assert_file_contains "$state_file" "\"effects_profile\":" "effects profile state section"
mfx_assert_file_contains "$state_file" "\"duration_scale\":" "effects profile state duration scale field"
mfx_assert_file_contains "$state_file" "\"size_scale\":" "effects profile state size scale field"
mfx_assert_file_contains "$state_file" "\"opacity_scale\":" "effects profile state opacity scale field"
mfx_assert_file_contains "$state_file" "\"trail_throttle_scale\":" "effects profile state trail throttle scale field"

state_duration_scale="$(_mfx_core_http_automation_parse_scalar_field "$state_file" "duration_scale")"
state_size_scale="$(_mfx_core_http_automation_parse_scalar_field "$state_file" "size_scale")"
state_opacity_scale="$(_mfx_core_http_automation_parse_scalar_field "$state_file" "opacity_scale")"
state_trail_throttle_scale="$(_mfx_core_http_automation_parse_scalar_field "$state_file" "trail_throttle_scale")"
assert_float_within_tolerance "$state_duration_scale" "$duration_scale" "0.0001" "effects profile state duration scale"
assert_float_within_tolerance "$state_size_scale" "$size_scale" "0.0001" "effects profile state size scale"
assert_float_within_tolerance "$state_opacity_scale" "$opacity_scale" "0.0001" "effects profile state opacity scale"
assert_float_within_tolerance "$state_trail_throttle_scale" "$trail_throttle_scale" "0.0001" "effects profile state trail throttle scale"

mfx_ok "macos effects profile tuning selfcheck passed"

if [[ "$keep_running" -eq 1 ]]; then
    if [[ "$auto_stop_seconds" -gt 0 ]]; then
        mfx_manual_schedule_auto_stop "$MFX_MANUAL_HOST_PID" "$auto_stop_seconds"
        printf 'auto_stop_seconds=%s\n' "$auto_stop_seconds"
    fi
    mfx_manual_print_stop_command "$MFX_MANUAL_HOST_PID"
fi
