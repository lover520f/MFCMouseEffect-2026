#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../../.." && pwd)"

source "$repo_root/tools/platform/regression/lib/common.sh"
source "$repo_root/tools/platform/regression/lib/build.sh"
source "$repo_root/tools/platform/regression/lib/core_http_automation_parse_helpers.sh"
source "$repo_root/tools/platform/regression/lib/core_http_automation_assert_helpers.sh"
source "$repo_root/tools/platform/manual/lib/macos_core_host.sh"

usage() {
    cat <<'EOF'
Usage:
  tools/platform/manual/run-macos-automation-app-scope-selfcheck.sh [options]

Options:
  --build-dir <path>          Build directory (default: /tmp/mfx-platform-macos-core-build)
  --log-file <path>           Host log path (default: /tmp/mfx-core-automation-app-scope-selfcheck.log)
  --probe-file <path>         Probe file path (default: /tmp/mfx-core-automation-app-scope-selfcheck.probe)
  --jobs <num>                Build jobs (sets MFX_BUILD_JOBS)
  --skip-build                Skip cmake configure/build
  --keep-running              Keep host running after selfcheck
  --auto-stop-seconds <num>   Auto stop host after N seconds (only used with --keep-running, default: 120)
  -h, --help                  Show this help
EOF
}

build_dir="/tmp/mfx-platform-macos-core-build"
log_file="/tmp/mfx-core-automation-app-scope-selfcheck.log"
probe_file="/tmp/mfx-core-automation-app-scope-selfcheck.probe"
skip_build=0
keep_running=0
auto_stop_seconds=120
build_jobs=""

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

parse_first_mapping_scope_count() {
    local input_file="$1"
    python3 - "$input_file" <<'PY'
import json
import sys

with open(sys.argv[1], "r", encoding="utf-8") as f:
    root = json.load(f)

mappings = root.get("automation", {}).get("mouse_mappings", [])
if not mappings:
    print("0")
    sys.exit(0)
scopes = mappings[0].get("app_scopes", [])
if not isinstance(scopes, list):
    print("0")
    sys.exit(0)
print(len(scopes))
PY
}

parse_first_mapping_scope_value() {
    local input_file="$1"
    python3 - "$input_file" <<'PY'
import json
import sys

with open(sys.argv[1], "r", encoding="utf-8") as f:
    root = json.load(f)

mappings = root.get("automation", {}).get("mouse_mappings", [])
if not mappings:
    print("")
    sys.exit(0)
scopes = mappings[0].get("app_scopes", [])
if not isinstance(scopes, list) or not scopes:
    print("")
    sys.exit(0)
value = scopes[0]
print(value if isinstance(value, str) else "")
PY
}

parse_first_mapping_legacy_scope_value() {
    local input_file="$1"
    python3 - "$input_file" <<'PY'
import json
import sys

with open(sys.argv[1], "r", encoding="utf-8") as f:
    root = json.load(f)

mappings = root.get("automation", {}).get("mouse_mappings", [])
if not mappings:
    print("")
    sys.exit(0)
value = mappings[0].get("app_scope", "")
print(value if isinstance(value, str) else "")
PY
}

mfx_manual_acquire_entry_host_lock

mfx_manual_prepare_core_host_binary "$repo_root" "$build_dir" "$skip_build"
host_bin="$MFX_MANUAL_HOST_BIN"

declare -a host_env
host_env+=("MFX_ENABLE_AUTOMATION_SCOPE_TEST_API=1")
start_status=0
mfx_manual_start_core_host "$host_bin" "$probe_file" "$log_file" "${host_env[@]}" || start_status=$?
if [[ "$start_status" -eq 2 ]]; then
    mfx_ok "macos automation app-scope selfcheck skipped: $MFX_MANUAL_STARTUP_SKIP_REASON"
    exit 0
fi
if [[ "$start_status" -ne 0 ]]; then
    exit "$start_status"
fi

printf 'mfx_pid=%s\n' "$MFX_MANUAL_HOST_PID"
printf 'settings_url=%s\n' "$MFX_MANUAL_SETTINGS_URL"
printf 'log_file=%s\n' "$MFX_MANUAL_LOG_FILE"

_mfx_core_http_automation_assert_scope_match "$MFX_MANUAL_BASE_URL" "$MFX_MANUAL_SETTINGS_TOKEN" \
    "code" "process:code.exe" "true" "$tmp_dir/scope-code-vs-exe.out" "app-scope code<->exe"
_mfx_core_http_automation_assert_scope_match "$MFX_MANUAL_BASE_URL" "$MFX_MANUAL_SETTINGS_TOKEN" \
    "code.app" "process:code" "true" "$tmp_dir/scope-app-vs-base.out" "app-scope app<->base"
_mfx_core_http_automation_assert_scope_match "$MFX_MANUAL_BASE_URL" "$MFX_MANUAL_SETTINGS_TOKEN" \
    "code.exe" "process:code.app" "true" "$tmp_dir/scope-exe-vs-app.out" "app-scope exe<->app"
_mfx_core_http_automation_assert_scope_match "$MFX_MANUAL_BASE_URL" "$MFX_MANUAL_SETTINGS_TOKEN" \
    "safari" "process:code" "false" "$tmp_dir/scope-negative.out" "app-scope negative"

code_state_apply_scope_alias_dedupe="$(mfx_http_code "$tmp_dir/state-apply-scope-alias-dedupe.out" "$MFX_MANUAL_BASE_URL/api/state" \
    -X POST \
    -H "x-mfcmouseeffect-token: $MFX_MANUAL_SETTINGS_TOKEN" \
    -H "Content-Type: application/json" \
    -d '{"automation":{"enabled":true,"mouse_mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["process:code.exe","process:code.app","process:code"],"keys":"Cmd+C"}]}}')"
mfx_assert_eq "$code_state_apply_scope_alias_dedupe" "200" "app-scope alias dedupe apply status"

code_state_after_scope_alias_dedupe="$(mfx_http_code "$tmp_dir/state-after-scope-alias-dedupe.out" "$MFX_MANUAL_BASE_URL/api/state" -H "x-mfcmouseeffect-token: $MFX_MANUAL_SETTINGS_TOKEN")"
mfx_assert_eq "$code_state_after_scope_alias_dedupe" "200" "app-scope alias dedupe state status"

dedupe_scope_count="$(parse_first_mapping_scope_count "$tmp_dir/state-after-scope-alias-dedupe.out")"
dedupe_scope_value="$(parse_first_mapping_scope_value "$tmp_dir/state-after-scope-alias-dedupe.out")"
dedupe_legacy_scope_value="$(parse_first_mapping_legacy_scope_value "$tmp_dir/state-after-scope-alias-dedupe.out")"
mfx_assert_eq "$dedupe_scope_count" "1" "app-scope alias dedupe persisted count"
mfx_assert_eq "$dedupe_scope_value" "process:code" "app-scope alias dedupe persisted canonical scope"
mfx_assert_eq "$dedupe_legacy_scope_value" "process:code" "app-scope alias dedupe legacy field parity"

mfx_ok "macos automation app-scope selfcheck passed"

if [[ "$keep_running" -eq 1 ]]; then
    if [[ "$auto_stop_seconds" -gt 0 ]]; then
        mfx_manual_schedule_auto_stop "$MFX_MANUAL_HOST_PID" "$auto_stop_seconds"
        printf 'auto_stop_seconds=%s\n' "$auto_stop_seconds"
    fi
    mfx_manual_print_stop_command "$MFX_MANUAL_HOST_PID"
fi
