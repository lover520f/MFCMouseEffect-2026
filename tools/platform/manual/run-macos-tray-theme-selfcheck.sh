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
  tools/platform/manual/run-macos-tray-theme-selfcheck.sh [options]

Options:
  --build-dir <path>          Build directory (default: /tmp/mfx-platform-macos-core-build)
  --log-file <path>           Host log path (default: /tmp/mfx-core-tray-theme-selfcheck.log)
  --probe-file <path>         Probe file path (default: /tmp/mfx-core-tray-theme-selfcheck.probe)
  --jobs <num>                Build jobs (sets MFX_BUILD_JOBS)
  --skip-build                Skip cmake configure/build
  --target-theme <value>      Preferred target theme value (default: neon, auto-fallback if unavailable)
  -h, --help                  Show this help
EOF
}

build_dir="/tmp/mfx-platform-macos-core-build"
log_file="/tmp/mfx-core-tray-theme-selfcheck.log"
probe_file="/tmp/mfx-core-tray-theme-selfcheck.probe"
skip_build=0
build_jobs=""
preferred_theme="neon"

_mfx_tray_selfcheck_read_json_string() {
    local input_file="$1"
    local dotted_key="$2"
    python3 - "$input_file" "$dotted_key" <<'PY'
import json
import sys

path = sys.argv[2].split(".")
with open(sys.argv[1], "r", encoding="utf-8") as fp:
    data = json.load(fp)
value = data
for key in path:
    if isinstance(value, dict) and key in value:
        value = value[key]
    else:
        value = ""
        break
if value is None:
    value = ""
if not isinstance(value, str):
    value = str(value)
print(value)
PY
}

_mfx_tray_selfcheck_select_target_theme() {
    local schema_file="$1"
    local preferred="$2"
    python3 - "$schema_file" "$preferred" <<'PY'
import json
import sys

schema_file, preferred = sys.argv[1], sys.argv[2]
with open(schema_file, "r", encoding="utf-8") as fp:
    schema = json.load(fp)

values = []
for item in schema.get("themes", []):
    if not isinstance(item, dict):
        continue
    value = str(item.get("value", "")).strip()
    if value:
        values.append(value)

if not values:
    print("")
    raise SystemExit(0)

if preferred and preferred in values:
    print(preferred)
    raise SystemExit(0)

print(values[0])
PY
}

_mfx_tray_selfcheck_select_pretrigger_theme() {
    local schema_file="$1"
    local target_theme="$2"
    python3 - "$schema_file" "$target_theme" <<'PY'
import json
import sys

schema_file, target = sys.argv[1], sys.argv[2]
with open(schema_file, "r", encoding="utf-8") as fp:
    schema = json.load(fp)

for item in schema.get("themes", []):
    if not isinstance(item, dict):
        continue
    value = str(item.get("value", "")).strip()
    if value and value != target:
        print(value)
        raise SystemExit(0)
print("")
PY
}

_mfx_tray_selfcheck_write_theme_payload() {
    local output_file="$1"
    local theme_value="$2"
    python3 - "$output_file" "$theme_value" <<'PY'
import json
import sys

with open(sys.argv[1], "w", encoding="utf-8") as fp:
    json.dump({"theme": sys.argv[2]}, fp, ensure_ascii=False)
PY
}

_mfx_tray_selfcheck_fetch_state_theme() {
    local base_url="$1"
    local token_header="$2"
    local output_file="$3"
    local out_var_name="$4"

    local code
    code="$(mfx_http_code "$output_file" "$base_url/api/state" -H "$token_header")"
    mfx_assert_eq "$code" "200" "tray theme selfcheck state status"
    local theme_value
    theme_value="$(_mfx_tray_selfcheck_read_json_string "$output_file" "theme")"
    if [[ -z "$theme_value" ]]; then
        mfx_fail "tray theme selfcheck: empty theme in /api/state"
    fi
    printf -v "$out_var_name" '%s' "$theme_value"
}

_mfx_tray_selfcheck_wait_for_theme() {
    local base_url="$1"
    local token_header="$2"
    local expected_theme="$3"
    local output_file="$4"
    local attempts="${5:-60}"

    local observed_theme=""
    for _ in $(seq 1 "$attempts"); do
        _mfx_tray_selfcheck_fetch_state_theme "$base_url" "$token_header" "$output_file" observed_theme
        if [[ "$observed_theme" == "$expected_theme" ]]; then
            return 0
        fi
        sleep 0.1
    done
    mfx_fail "tray theme selfcheck: expected theme '$expected_theme', got '$observed_theme'"
}

_mfx_tray_selfcheck_refresh_token_header() {
    token="$MFX_MANUAL_SETTINGS_TOKEN"
    token_header="x-mfcmouseeffect-token: $token"
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
    --target-theme)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --target-theme"
        preferred_theme="$2"
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

mfx_require_cmd curl
mfx_require_cmd python3

tmp_dir="$(mktemp -d)"
cleanup() {
    rm -rf "$tmp_dir" >/dev/null 2>&1 || true
    mfx_release_lock
    mfx_manual_stop_core_host "$MFX_MANUAL_HOST_PID"
}
trap cleanup EXIT

mfx_manual_acquire_entry_host_lock

mfx_manual_prepare_core_host_binary "$repo_root" "$build_dir" "$skip_build"
host_bin="$MFX_MANUAL_HOST_BIN"

start_status=0
mfx_manual_start_core_host "$host_bin" "$probe_file" "$log_file" || start_status=$?
if [[ "$start_status" -eq 2 ]]; then
    mfx_ok "macos tray theme selfcheck skipped: $MFX_MANUAL_STARTUP_SKIP_REASON"
    exit 0
fi
if [[ "$start_status" -ne 0 ]]; then
    exit "$start_status"
fi

printf 'mfx_pid=%s\n' "$MFX_MANUAL_HOST_PID"
printf 'settings_url=%s\n' "$MFX_MANUAL_SETTINGS_URL"
printf 'log_file=%s\n' "$MFX_MANUAL_LOG_FILE"

_mfx_tray_selfcheck_refresh_token_header

state_before_file="$tmp_dir/state-before.out"
schema_before_file="$tmp_dir/schema-before.out"
original_theme=""
_mfx_tray_selfcheck_fetch_state_theme "$MFX_MANUAL_BASE_URL" "$token_header" "$state_before_file" original_theme
code_schema_before="$(mfx_http_code "$schema_before_file" "$MFX_MANUAL_BASE_URL/api/schema" -H "$token_header")"
mfx_assert_eq "$code_schema_before" "200" "tray theme selfcheck schema-before status"

target_theme="$(_mfx_tray_selfcheck_select_target_theme "$schema_before_file" "$preferred_theme")"
if [[ -z "$target_theme" ]]; then
    mfx_fail "tray theme selfcheck: no theme options from schema"
fi
pretrigger_theme="$(_mfx_tray_selfcheck_select_pretrigger_theme "$schema_before_file" "$target_theme")"

if [[ -n "$pretrigger_theme" ]]; then
    pretrigger_payload="$tmp_dir/apply-pretrigger-theme.json"
    _mfx_tray_selfcheck_write_theme_payload "$pretrigger_payload" "$pretrigger_theme"
    pretrigger_file="$tmp_dir/state-apply-pretrigger-theme.out"
    pretrigger_code="$(mfx_http_code "$pretrigger_file" "$MFX_MANUAL_BASE_URL/api/state" \
        -X POST \
        -H "$token_header" \
        -H "Content-Type: application/json" \
        --data-binary "@$pretrigger_payload")"
    mfx_assert_eq "$pretrigger_code" "200" "tray theme selfcheck apply pretrigger theme status"
    mfx_assert_file_contains "$pretrigger_file" "\"ok\":true" "tray theme selfcheck apply pretrigger theme response ok"

    state_after_pretrigger_file="$tmp_dir/state-after-pretrigger.out"
    observed_pretrigger_theme=""
    _mfx_tray_selfcheck_fetch_state_theme "$MFX_MANUAL_BASE_URL" "$token_header" "$state_after_pretrigger_file" observed_pretrigger_theme
    mfx_assert_eq "$observed_pretrigger_theme" "$pretrigger_theme" "tray theme selfcheck pretrigger theme applied"
fi

mfx_manual_stop_core_host "$MFX_MANUAL_HOST_PID"

start_status=0
mfx_manual_start_core_host \
    "$host_bin" \
    "$probe_file" \
    "$log_file" \
    "MFX_TEST_TRAY_AUTO_TRIGGER_THEME_VALUE=$target_theme" || start_status=$?
if [[ "$start_status" -eq 2 ]]; then
    mfx_ok "macos tray theme selfcheck skipped: $MFX_MANUAL_STARTUP_SKIP_REASON"
    exit 0
fi
if [[ "$start_status" -ne 0 ]]; then
    exit "$start_status"
fi

_mfx_tray_selfcheck_refresh_token_header

state_after_trigger_file="$tmp_dir/state-after-trigger.out"
_mfx_tray_selfcheck_wait_for_theme "$MFX_MANUAL_BASE_URL" "$token_header" "$target_theme" "$state_after_trigger_file"

mfx_manual_stop_core_host "$MFX_MANUAL_HOST_PID"

start_status=0
mfx_manual_start_core_host "$host_bin" "$probe_file" "$log_file" || start_status=$?
if [[ "$start_status" -eq 2 ]]; then
    mfx_ok "macos tray theme selfcheck skipped: $MFX_MANUAL_STARTUP_SKIP_REASON"
    exit 0
fi
if [[ "$start_status" -ne 0 ]]; then
    exit "$start_status"
fi

_mfx_tray_selfcheck_refresh_token_header

state_after_restart_file="$tmp_dir/state-after-restart.out"
persisted_theme=""
_mfx_tray_selfcheck_fetch_state_theme "$MFX_MANUAL_BASE_URL" "$token_header" "$state_after_restart_file" persisted_theme
mfx_assert_eq "$persisted_theme" "$target_theme" "tray theme selfcheck persisted theme after restart"

if [[ "$original_theme" != "$target_theme" ]]; then
    restore_payload="$tmp_dir/restore-theme.json"
    _mfx_tray_selfcheck_write_theme_payload "$restore_payload" "$original_theme"
    restore_file="$tmp_dir/state-restore-theme.out"
    restore_code="$(mfx_http_code "$restore_file" "$MFX_MANUAL_BASE_URL/api/state" \
        -X POST \
        -H "$token_header" \
        -H "Content-Type: application/json" \
        --data-binary "@$restore_payload")"
    mfx_assert_eq "$restore_code" "200" "tray theme selfcheck restore status"
    mfx_assert_file_contains "$restore_file" "\"ok\":true" "tray theme selfcheck restore response ok"

    state_after_restore_file="$tmp_dir/state-after-restore.out"
    restored_theme=""
    _mfx_tray_selfcheck_fetch_state_theme "$MFX_MANUAL_BASE_URL" "$token_header" "$state_after_restore_file" restored_theme
    mfx_assert_eq "$restored_theme" "$original_theme" "tray theme selfcheck restored original theme"
fi

mfx_ok "macos tray theme selfcheck passed"
