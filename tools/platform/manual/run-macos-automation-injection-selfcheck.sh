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
  tools/platform/manual/run-macos-automation-injection-selfcheck.sh [options]

Options:
  --build-dir <path>          Build directory (default: /tmp/mfx-platform-macos-core-build)
  --log-file <path>           Host log path (default: /tmp/mfx-core-automation-inject-selfcheck.log)
  --probe-file <path>         Probe file path (default: /tmp/mfx-core-automation-inject-selfcheck.probe)
  --osascript-timeout-seconds <num> Timeout for TextEdit preparation AppleScript (default: 12)
  --jobs <num>                Build jobs (sets MFX_BUILD_JOBS)
  --skip-build                Skip cmake configure/build
  --dry-run                   Use keyboard injector dry-run mode (no real OS key dispatch)
  --keep-running              Keep host running after selfcheck
  --auto-stop-seconds <num>   Auto stop host after N seconds (only used with --keep-running, default: 120)
  -h, --help                  Show this help
EOF
}

json_escape() {
    local value="$1"
    printf '%s' "$value" | sed 's/\\/\\\\/g; s/"/\\"/g'
}

prepare_textedit_selection() {
    local selected_text="$1"
    osascript - "$selected_text" <<'OSA'
on run argv
  set targetText to item 1 of argv
  tell application "TextEdit"
    activate
    if not (exists document 1) then
      make new document
    end if
    set text of document 1 to targetText
  end tell
  delay 0.2
  tell application "System Events"
    if not (exists process "TextEdit") then
      error "TextEdit process not ready"
    end if
    tell process "TextEdit"
      keystroke "a" using {command down}
    end tell
  end tell
end run
OSA
}

build_dir="/tmp/mfx-platform-macos-core-build"
log_file="/tmp/mfx-core-automation-inject-selfcheck.log"
probe_file="/tmp/mfx-core-automation-inject-selfcheck.probe"
skip_build=0
dry_run=0
keep_running=0
auto_stop_seconds=120
osascript_timeout_seconds=12
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
    --osascript-timeout-seconds)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --osascript-timeout-seconds"
        osascript_timeout_seconds="$2"
        shift 2
        ;;
    --skip-build)
        skip_build=1
        shift
        ;;
    --dry-run)
        dry_run=1
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
if ! [[ "$osascript_timeout_seconds" =~ ^[0-9]+$ ]] || [[ "$osascript_timeout_seconds" -le 0 ]]; then
    mfx_fail "--osascript-timeout-seconds must be a positive integer"
fi

mfx_require_cmd curl
mfx_require_cmd sed

if [[ "$dry_run" -eq 0 ]]; then
    mfx_require_cmd osascript
    mfx_require_cmd pbcopy
    mfx_require_cmd pbpaste
fi

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

run_with_timeout() {
    local timeout_seconds="$1"
    shift

    local timeout_marker="$tmp_dir/.timeout-marker"
    rm -f "$timeout_marker"

    "$@" &
    local cmd_pid="$!"
    (
        sleep "$timeout_seconds"
        if kill -0 "$cmd_pid" 2>/dev/null; then
            touch "$timeout_marker"
            kill -TERM "$cmd_pid" 2>/dev/null || true
        fi
    ) >/dev/null 2>&1 &
    local watchdog_pid="$!"

    local cmd_status=0
    if ! wait "$cmd_pid"; then
        cmd_status=$?
    fi
    kill -TERM "$watchdog_pid" 2>/dev/null || true
    wait "$watchdog_pid" 2>/dev/null || true

    if [[ -f "$timeout_marker" ]]; then
        return 124
    fi
    return "$cmd_status"
}

declare -a host_env
host_env+=("MFX_ENABLE_AUTOMATION_SCOPE_TEST_API=1")
host_env+=("MFX_ENABLE_AUTOMATION_INJECTION_TEST_API=1")
if [[ "$dry_run" -eq 1 ]]; then
    host_env+=("MFX_TEST_KEYBOARD_INJECTOR_DRY_RUN=1")
fi
mfx_manual_start_core_host "$host_bin" "$probe_file" "$log_file" "${host_env[@]}"

printf 'mfx_pid=%s\n' "$MFX_MANUAL_HOST_PID"
printf 'settings_url=%s\n' "$MFX_MANUAL_SETTINGS_URL"
printf 'dry_run=%s\n' "$dry_run"
printf 'log_file=%s\n' "$MFX_MANUAL_LOG_FILE"

token_header=("x-mfcmouseeffect-token: $MFX_MANUAL_SETTINGS_TOKEN")

target_text=""
if [[ "$dry_run" -eq 0 ]]; then
    target_text="MFX_AUTOMATION_INJECT_OK_$(date +%s)"
    baseline_text="MFX_AUTOMATION_INJECT_BASELINE_$(date +%s)"
    printf '%s' "$baseline_text" | pbcopy
    if ! run_with_timeout "$osascript_timeout_seconds" prepare_textedit_selection "$target_text"; then
        mfx_fail "TextEdit preparation timed out or failed. verify Terminal Accessibility permission and retry."
    fi
fi

payload='{"history":["left_click"],"mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["all"],"keys":"Cmd+C"}]}'
inject_file="$tmp_dir/match-and-inject.out"
code_inject="$(mfx_http_code "$inject_file" "$MFX_MANUAL_BASE_URL/api/automation/test-match-and-inject" \
    -X POST -H "${token_header[0]}" -H "Content-Type: application/json" -d "$payload")"
mfx_assert_eq "$code_inject" "200" "automation inject selfcheck status"
mfx_assert_file_contains "$inject_file" "\"ok\":true" "automation inject selfcheck ok"
mfx_assert_file_contains "$inject_file" "\"matched\":true" "automation inject selfcheck matched"
mfx_assert_file_contains "$inject_file" "\"injected\":true" "automation inject selfcheck injected"
mfx_assert_file_contains "$inject_file" "\"selected_keys\":\"Cmd+C\"" "automation inject selfcheck selected keys"

if [[ "$dry_run" -eq 0 ]]; then
    sleep 0.2
    copied_text="$(pbpaste)"
    if [[ "$copied_text" != "$target_text" ]]; then
        mfx_fail "clipboard mismatch after injection (expected=$target_text, actual=$copied_text). verify Accessibility/Input Monitoring permission and TextEdit focus."
    fi
fi

mfx_ok "macos automation injection selfcheck passed"

if [[ "$keep_running" -eq 1 ]]; then
    if [[ "$auto_stop_seconds" -gt 0 ]]; then
        mfx_manual_schedule_auto_stop "$MFX_MANUAL_HOST_PID" "$auto_stop_seconds"
        printf 'auto_stop_seconds=%s\n' "$auto_stop_seconds"
    fi
    mfx_manual_print_stop_command "$MFX_MANUAL_HOST_PID"
fi
