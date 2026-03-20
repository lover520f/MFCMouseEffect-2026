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
  tools/platform/manual/run-macos-mouse-companion-proof.sh [options]

Options:
  --build-dir <path>               Build directory (default: /tmp/mfx-platform-macos-core-build)
  --probe-file <path>              Probe file path (default: /tmp/mfx-core-mouse-companion-proof.probe)
  --log-file <path>                Host log path (default: /tmp/mfx-core-mouse-companion-proof.log)
  --model-path <path>              Companion model path (default: repo Assets/Pet3D/source/pet-main.glb)
  --action-library-path <path>     Action library path (default: repo Assets/Pet3D/source/pet-actions.json)
  --appearance-profile-path <path> Appearance profile path (default: repo Assets/Pet3D/source/pet-appearance.json)
  --state-file <path>              Save full /api/state response to file
  --json-output <path>             Save compact proof json output to file
  --jobs <num>                     Build jobs (sets MFX_BUILD_JOBS)
  --skip-build                     Skip cmake configure/build
  --skip-webui-build               Skip WebUIWorkspace build
  -h, --help                       Show this help
EOF
}

build_dir="/tmp/mfx-platform-macos-core-build"
probe_file="/tmp/mfx-core-mouse-companion-proof.probe"
log_file="/tmp/mfx-core-mouse-companion-proof.log"
model_path="$repo_root/MFCMouseEffect/Assets/Pet3D/source/pet-main.glb"
action_library_path="$repo_root/MFCMouseEffect/Assets/Pet3D/source/pet-actions.json"
appearance_profile_path="$repo_root/MFCMouseEffect/Assets/Pet3D/source/pet-appearance.json"
state_file=""
json_output=""
skip_build=0
skip_webui_build=0
build_jobs=""

while [[ $# -gt 0 ]]; do
    case "$1" in
    --build-dir)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --build-dir"
        build_dir="$2"
        shift 2
        ;;
    --probe-file)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --probe-file"
        probe_file="$2"
        shift 2
        ;;
    --log-file)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --log-file"
        log_file="$2"
        shift 2
        ;;
    --model-path)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --model-path"
        model_path="$2"
        shift 2
        ;;
    --action-library-path)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --action-library-path"
        action_library_path="$2"
        shift 2
        ;;
    --appearance-profile-path)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --appearance-profile-path"
        appearance_profile_path="$2"
        shift 2
        ;;
    --state-file)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --state-file"
        state_file="$2"
        shift 2
        ;;
    --json-output)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --json-output"
        json_output="$2"
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
    --skip-webui-build)
        skip_webui_build=1
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
mfx_require_cmd jq
mfx_require_cmd curl

for required_file in "$model_path" "$action_library_path" "$appearance_profile_path"; do
    [[ -f "$required_file" ]] || mfx_fail "required asset missing: $required_file"
done

mfx_manual_acquire_entry_host_lock
cleanup_lock() {
    mfx_release_lock
}
trap cleanup_lock EXIT

mfx_manual_prepare_core_host_binary "$repo_root" "$build_dir" "$skip_build" "$skip_webui_build"
host_bin="$MFX_MANUAL_HOST_BIN"

stdin_pipe="${probe_file}.stdin"
state_tmp="$(mktemp /tmp/mfx-mouse-companion-proof-state.XXXXXX)"
proof_tmp="$(mktemp /tmp/mfx-mouse-companion-proof-result.XXXXXX)"
dispatch_status_tmp="$(mktemp /tmp/mfx-mouse-companion-proof-dispatch-status.XXXXXX)"
dispatch_move_tmp="$(mktemp /tmp/mfx-mouse-companion-proof-dispatch-move.XXXXXX)"
dispatch_scroll_tmp="$(mktemp /tmp/mfx-mouse-companion-proof-dispatch-scroll.XXXXXX)"
dispatch_down_tmp="$(mktemp /tmp/mfx-mouse-companion-proof-dispatch-down.XXXXXX)"
dispatch_up_tmp="$(mktemp /tmp/mfx-mouse-companion-proof-dispatch-up.XXXXXX)"
dispatch_click_tmp="$(mktemp /tmp/mfx-mouse-companion-proof-dispatch-click.XXXXXX)"
dispatch_hover_start_tmp="$(mktemp /tmp/mfx-mouse-companion-proof-dispatch-hover-start.XXXXXX)"
dispatch_hover_end_tmp="$(mktemp /tmp/mfx-mouse-companion-proof-dispatch-hover-end.XXXXXX)"
dispatch_hold_start_tmp="$(mktemp /tmp/mfx-mouse-companion-proof-dispatch-hold-start.XXXXXX)"
dispatch_hold_update_tmp="$(mktemp /tmp/mfx-mouse-companion-proof-dispatch-hold-update.XXXXXX)"
dispatch_hold_end_tmp="$(mktemp /tmp/mfx-mouse-companion-proof-dispatch-hold-end.XXXXXX)"
rm -f "$probe_file" "${probe_file}.diagnostics" "$stdin_pipe" "$log_file"
mkfifo "$stdin_pipe"

tail -f /dev/null >"$stdin_pipe" &
feeder_pid="$!"

host_pid=""
base_url=""
token=""

cleanup_runtime() {
    if [[ -n "$host_pid" ]] && kill -0 "$host_pid" 2>/dev/null; then
        if [[ -n "$base_url" && -n "$token" ]]; then
            curl -sS -m 2 -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                "$base_url/api/stop" >/dev/null 2>&1 || true
        fi
        kill -TERM "$host_pid" 2>/dev/null || true
        wait "$host_pid" 2>/dev/null || true
    fi
    kill -TERM "$feeder_pid" 2>/dev/null || true
    wait "$feeder_pid" 2>/dev/null || true
    rm -f "$stdin_pipe"
}
trap cleanup_runtime RETURN

single_key="Global\\MFCMouseEffect_MouseCompanionProof_${RANDOM}_$$"
mfx_info "start host (background mode)"
env MFX_WEBUI_DIR="$repo_root/MFCMouseEffect/WebUI" \
    MFX_SCAFFOLD_WEBUI_DIR="$repo_root/MFCMouseEffect/WebUI" \
    MFX_ENABLE_MOUSE_COMPANION_TEST_API=1 \
    MFX_CORE_WEB_SETTINGS_PROBE_FILE="$probe_file" \
    MFX_CORE_WEB_SETTINGS_PROBE_DIAGNOSTICS_FILE="${probe_file}.diagnostics" \
    "$host_bin" --mode=background --single-instance-key="$single_key" \
    <"$stdin_pipe" >"$log_file" 2>&1 &
host_pid="$!"

for _ in $(seq 1 220); do
    if [[ -s "$probe_file" ]]; then
        break
    fi
    if ! kill -0 "$host_pid" 2>/dev/null; then
        tail -n 120 "$log_file" >&2 || true
        mfx_fail "host exited early (probe missing)"
    fi
    sleep 0.05
done

[[ -s "$probe_file" ]] || mfx_fail "probe file not ready: $probe_file"

base_url="$(sed -n 's/^url=//p' "$probe_file" | head -n1 | sed 's/[?].*$//' | sed 's:/$::')"
token="$(sed -n 's/^token=//p' "$probe_file" | head -n1)"
[[ -n "$base_url" && -n "$token" ]] || mfx_fail "probe missing url/token: $probe_file"

payload="$(jq -n \
    --arg model "$model_path" \
    --arg action "$action_library_path" \
    --arg appearance "$appearance_profile_path" \
    '{mouse_companion:{enabled:true,model_path:$model,action_library_path:$action,appearance_profile_path:$appearance,position_mode:"fixed_bottom_left",release_hold_ms:0,follow_threshold_px:0,use_test_profile:false}}')"

curl -sS -X POST \
    -H "x-mfcmouseeffect-token: $token" \
    -H "content-type: application/json" \
    --data "$payload" \
    "$base_url/api/state" >/dev/null

sleep 0.8
curl -sS \
    -H "x-mfcmouseeffect-token: $token" \
    "$base_url/api/state" >"$state_tmp"

dispatch_mouse_companion_test_event() {
    local event="$1"
    local x="$2"
    local y="$3"
    local button="$4"
    local delta="$5"
    local hold_ms="$6"
    local out_file="$7"
    local body
    body="$(jq -n \
        --arg event "$event" \
        --argjson x "$x" \
        --argjson y "$y" \
        --argjson button "$button" \
        --argjson delta "$delta" \
        --argjson hold_ms "$hold_ms" \
        '{event:$event,x:$x,y:$y,button:$button,delta:$delta,hold_ms:$hold_ms}')"
    curl -sS -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "content-type: application/json" \
        --data "$body" \
        "$base_url/api/mouse-companion/test-dispatch" >"$out_file"
}

dispatch_mouse_companion_test_event "status" 640 360 1 120 420 "$dispatch_status_tmp"
dispatch_mouse_companion_test_event "move" 640 360 1 120 420 "$dispatch_move_tmp"
dispatch_mouse_companion_test_event "scroll" 640 360 1 120 420 "$dispatch_scroll_tmp"
# Align with click-gate contract: avoid immediate click after scroll suppression window.
sleep 0.20
dispatch_mouse_companion_test_event "button_down" 640 360 1 120 420 "$dispatch_down_tmp"
dispatch_mouse_companion_test_event "button_up" 640 360 1 120 420 "$dispatch_up_tmp"
dispatch_mouse_companion_test_event "click" 640 360 1 120 420 "$dispatch_click_tmp"
dispatch_mouse_companion_test_event "hover_start" 640 360 1 120 420 "$dispatch_hover_start_tmp"
dispatch_mouse_companion_test_event "hover_end" 640 360 1 120 420 "$dispatch_hover_end_tmp"
dispatch_mouse_companion_test_event "hold_start" 640 360 1 120 420 "$dispatch_hold_start_tmp"
dispatch_mouse_companion_test_event "hold_update" 640 360 1 120 640 "$dispatch_hold_update_tmp"
dispatch_mouse_companion_test_event "hold_end" 640 360 1 120 640 "$dispatch_hold_end_tmp"

curl -sS \
    -H "x-mfcmouseeffect-token: $token" \
    "$base_url/api/state" >"$state_tmp"

jq -n \
    --arg base_url "$base_url" \
    --arg model_path "$model_path" \
    --arg action_library_path "$action_library_path" \
    --arg appearance_profile_path "$appearance_profile_path" \
    --slurpfile state "$state_tmp" \
    --slurpfile status "$dispatch_status_tmp" '
  ($state[0].mouse_companion_runtime // {}) as $rt
  | {
      base_url: $base_url,
      model_path: $model_path,
      action_library_path: $action_library_path,
      appearance_profile_path: $appearance_profile_path,
      runtime: {
        model_loaded: ($rt.model_loaded // false),
        visual_model_loaded: ($rt.visual_model_loaded // false),
        action_library_loaded: ($rt.action_library_loaded // false),
        effect_profile_loaded: ($rt.effect_profile_loaded // false),
        appearance_profile_loaded: ($rt.appearance_profile_loaded // false),
        pose_binding_configured: ($rt.pose_binding_configured // false),
        skeleton_bone_count: ($rt.skeleton_bone_count // 0),
        last_action_name: ($rt.last_action_name // ""),
        last_action_code: ($rt.last_action_code // -1),
        last_action_intensity: ($rt.last_action_intensity // 0),
        last_action_tick_ms: ($rt.last_action_tick_ms // 0),
        click_streak: ($rt.click_streak // 0),
        click_streak_tint_amount: ($rt.click_streak_tint_amount // 0),
        click_streak_break_ms: ($rt.click_streak_break_ms // 0),
        click_streak_decay_per_second: ($rt.click_streak_decay_per_second // 0),
        loaded_model_path: ($rt.loaded_model_path // ""),
        loaded_effect_profile_path: ($rt.loaded_effect_profile_path // ""),
        visual_model_path: ($rt.visual_model_path // ""),
        model_load_error: ($rt.model_load_error // ""),
        effect_profile_load_error: ($rt.effect_profile_load_error // ""),
        visual_model_load_error: ($rt.visual_model_load_error // "")
      },
      action_coverage: (($rt.action_coverage // {}) | {
        ready: (.ready // false),
        error: (.error // ""),
        expected_action_count: (.expected_action_count // 0),
        covered_action_count: (.covered_action_count // 0),
        missing_action_count: (.missing_action_count // 0),
        skeleton_bone_count: (.skeleton_bone_count // 0),
        total_track_count: (.total_track_count // 0),
        mapped_track_count: (.mapped_track_count // 0),
        overall_coverage_ratio: (.overall_coverage_ratio // 0),
        missing_actions: (.missing_actions // []),
        missing_bone_names: (.missing_bone_names // []),
        actions: (.actions // [])
      })
    }' >"$proof_tmp"

jq --slurpfile status "$dispatch_status_tmp" \
   --slurpfile move "$dispatch_move_tmp" \
   --slurpfile scroll "$dispatch_scroll_tmp" \
   --slurpfile down "$dispatch_down_tmp" \
   --slurpfile up "$dispatch_up_tmp" \
   --slurpfile click "$dispatch_click_tmp" \
   --slurpfile hover_start "$dispatch_hover_start_tmp" \
   --slurpfile hover_end "$dispatch_hover_end_tmp" \
   --slurpfile hold_start "$dispatch_hold_start_tmp" \
   --slurpfile hold_update "$dispatch_hold_update_tmp" \
   --slurpfile hold_end "$dispatch_hold_end_tmp" \
   '.action_sequence = {
      idle: ($status[0].runtime.last_action_name // ""),
      follow: ($move[0].runtime.last_action_name // ""),
      scroll: ($scroll[0].runtime.last_action_name // ""),
      press: ($down[0].runtime.last_action_name // ""),
      release: ($up[0].runtime.last_action_name // ""),
      click: ($click[0].runtime.last_action_name // ""),
      hover_start: ($hover_start[0].runtime.last_action_name // ""),
      hover_end: ($hover_end[0].runtime.last_action_name // ""),
      hold_start: ($hold_start[0].runtime.last_action_name // ""),
      hold_update: ($hold_update[0].runtime.last_action_name // ""),
      hold_end: ($hold_end[0].runtime.last_action_name // "")
    }' "$proof_tmp" >"${proof_tmp}.tmp"
mv "${proof_tmp}.tmp" "$proof_tmp"

if ! jq -e '
    .runtime.model_loaded == true and
    .runtime.visual_model_loaded == true and
    .runtime.action_library_loaded == true and
    .runtime.effect_profile_loaded == true and
    .runtime.appearance_profile_loaded == true and
    .runtime.pose_binding_configured == true and
    (.runtime.skeleton_bone_count | tonumber) > 0 and
    (.runtime.last_action_tick_ms | tonumber) > 0 and
    (.runtime.click_streak | tonumber) >= 1 and
    (.runtime.click_streak_tint_amount | tonumber) > 0 and
    (.runtime.click_streak_break_ms | tonumber) == 650 and
    (.runtime.click_streak_decay_per_second | tonumber) > 0 and
    .action_coverage.ready == true and
    (.action_coverage.expected_action_count | tonumber) == 7 and
    (.action_coverage.covered_action_count | tonumber) >= 7 and
    (.action_coverage.missing_action_count | tonumber) == 0 and
    (.action_coverage.total_track_count | tonumber) > 0 and
    (.action_coverage.mapped_track_count | tonumber) > 0 and
    (.action_coverage.overall_coverage_ratio | tonumber) >= 1.0 and
    (.action_coverage.missing_bone_names | length) == 0 and
    (.action_sequence.idle == "idle" or .action_sequence.idle == "follow" or .action_sequence.idle == "hover_react") and
    .action_sequence.follow == "idle" and
    .action_sequence.scroll == "scroll_react" and
    .action_sequence.press == "drag" and
    .action_sequence.release == "idle" and
    .action_sequence.click == "click_react" and
    .action_sequence.hover_start == "hover_react" and
    .action_sequence.hover_end == "idle" and
    .action_sequence.hold_start == "hold_react" and
    .action_sequence.hold_update == "hold_react" and
    .action_sequence.hold_end == "idle"
' "$proof_tmp" >/dev/null; then
    mfx_info "proof failed; runtime snapshot:"
    cat "$proof_tmp" >&2
    mfx_fail "mouse companion runtime proof failed"
fi

if [[ -n "$state_file" ]]; then
    cp -f "$state_tmp" "$state_file"
fi
if [[ -n "$json_output" ]]; then
    cp -f "$proof_tmp" "$json_output"
fi

cat "$proof_tmp"
mfx_ok "mouse companion runtime proof passed"
