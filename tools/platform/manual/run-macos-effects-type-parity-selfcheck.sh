#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../../.." && pwd)"

source "$repo_root/tools/platform/regression/lib/common.sh"
source "$repo_root/tools/platform/regression/lib/build.sh"
source "$repo_root/tools/platform/manual/lib/macos_core_host.sh"

parse_uint_field() {
    local file="$1"
    local key="$2"
    local value
    value="$(grep -oE "\"$key\":[0-9]+" "$file" | head -n1 | cut -d: -f2 || true)"
    printf '%s' "$value"
}

usage() {
    cat <<'EOF'
Usage:
  tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh [options]

Options:
  --build-dir <path>          Build directory (default: /tmp/mfx-platform-macos-core-build)
  --log-file <path>           Host log path (default: /tmp/mfx-core-effects-type-parity.log)
  --probe-file <path>         Probe file path (default: /tmp/mfx-core-effects-type-parity.probe)
  --jobs <num>                Build jobs (sets MFX_BUILD_JOBS)
  --skip-build                Skip cmake configure/build
  --keep-running              Keep host running after selfcheck
  --auto-stop-seconds <num>   Auto stop host after N seconds (only with --keep-running, default: 120)
  -h, --help                  Show this help
EOF
}

build_dir="/tmp/mfx-platform-macos-core-build"
log_file="/tmp/mfx-core-effects-type-parity.log"
probe_file="/tmp/mfx-core-effects-type-parity.probe"
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

mfx_manual_acquire_entry_host_lock
mfx_manual_prepare_core_host_binary "$repo_root" "$build_dir" "$skip_build"
host_bin="$MFX_MANUAL_HOST_BIN"

declare -a host_env
host_env+=("MFX_ENABLE_EFFECT_OVERLAY_TEST_API=1")
start_status=0
mfx_manual_start_core_host "$host_bin" "$probe_file" "$log_file" "${host_env[@]}" || start_status=$?
if [[ "$start_status" -eq 2 ]]; then
    mfx_ok "macos effects type parity selfcheck skipped: $MFX_MANUAL_STARTUP_SKIP_REASON"
    exit 0
fi
if [[ "$start_status" -ne 0 ]]; then
    exit "$start_status"
fi

printf 'mfx_pid=%s\n' "$MFX_MANUAL_HOST_PID"
printf 'settings_url=%s\n' "$MFX_MANUAL_SETTINGS_URL"
printf 'log_file=%s\n' "$MFX_MANUAL_LOG_FILE"

token="$MFX_MANUAL_SETTINGS_TOKEN"
token_header="x-mfcmouseeffect-token: $token"

profile_probe_file="$tmp_dir/effects-profile-probe.out"
profile_code="$(mfx_http_code "$profile_probe_file" "$MFX_MANUAL_BASE_URL/api/effects/test-render-profiles" \
    -X GET \
    -H "$token_header")"
mfx_assert_eq "$profile_code" "200" "effects profile probe status"
mfx_assert_file_contains "$profile_probe_file" "\"ok\":true" "effects profile probe ok"
mfx_assert_file_contains "$profile_probe_file" "\"alias_matrix\":" "effects profile alias matrix section"
mfx_assert_file_contains "$profile_probe_file" "\"active_normalized\":" "effects profile active normalized sample input"
mfx_assert_file_contains "$profile_probe_file" "\"active_resolved\":" "effects profile active resolved sample input"
mfx_assert_file_contains "$profile_probe_file" "\"hold_follow_mode_normalized\":" "effects profile hold follow mode normalized sample input"
mfx_assert_file_contains "$profile_probe_file" "\"input\":\"scifi\",\"normalized\":\"tubes\"" "effects profile trail scifi alias"
mfx_assert_file_contains "$profile_probe_file" "\"input\":\"stardust\",\"normalized\":\"twinkle\"" "effects profile scroll stardust alias"
mfx_assert_file_contains "$profile_probe_file" "\"input\":\"suspension\",\"normalized\":\"tubes\"" "effects profile hover suspension alias"
mfx_assert_file_contains "$profile_probe_file" "\"catalog_values\":" "effects profile metadata catalog values"
mfx_assert_file_contains "$profile_probe_file" "\"click\":[\"ripple\",\"star\",\"text\",\"none\"]" "effects profile click catalog values"
mfx_assert_file_contains "$profile_probe_file" "\"trail\":[\"meteor\",\"streamer\",\"electric\",\"tubes\",\"particle\",\"line\",\"none\"]" "effects profile trail catalog values"
mfx_assert_file_contains "$profile_probe_file" "\"scroll\":[\"arrow\",\"helix\",\"twinkle\",\"none\"]" "effects profile scroll catalog values"
mfx_assert_file_contains "$profile_probe_file" "\"hover\":[\"glow\",\"tubes\",\"none\"]" "effects profile hover catalog values"
mfx_assert_file_contains "$profile_probe_file" "\"hold\":[\"charge\",\"lightning\",\"hex\",\"tech_ring\",\"hologram\",\"hold_neon3d\"" "effects profile hold catalog values prefix"
mfx_assert_file_contains "$profile_probe_file" "\"hold_quantum_halo_gpu_v2\"" "effects profile hold quantum halo gpu value"
mfx_assert_file_contains "$profile_probe_file" "\"hold_fluxfield_cpu\"" "effects profile hold flux field cpu value"
mfx_assert_file_contains "$profile_probe_file" "\"hold_fluxfield_gpu_v2\",\"none\"]" "effects profile hold catalog values suffix"

state_apply_file="$tmp_dir/state-apply.out"
state_apply_code="$(mfx_http_code "$state_apply_file" "$MFX_MANUAL_BASE_URL/api/state" \
    -X POST \
    -H "$token_header" \
    -H "Content-Type: application/json" \
    -d '{"active":{"click":"text","trail":"meteor","scroll":"helix","hold":"hologram","hover":"tubes"},"hold_follow_mode":"cursor_priority"}')"
mfx_assert_eq "$state_apply_code" "200" "effects state apply status"
mfx_assert_file_contains "$state_apply_file" "\"ok\":true" "effects state apply ok"

state_after_file="$tmp_dir/state-after.out"
state_after_code="$(mfx_http_code "$state_after_file" "$MFX_MANUAL_BASE_URL/api/state" \
    -X GET \
    -H "$token_header")"
mfx_assert_eq "$state_after_code" "200" "effects state after status"
mfx_assert_file_contains "$state_after_file" "\"click\":\"text\"" "effects state click active"
mfx_assert_file_contains "$state_after_file" "\"trail\":\"meteor\"" "effects state trail active"
mfx_assert_file_contains "$state_after_file" "\"scroll\":\"helix\"" "effects state scroll active"
mfx_assert_file_contains "$state_after_file" "\"hold\":\"hologram\"" "effects state hold active"
mfx_assert_file_contains "$state_after_file" "\"hover\":\"tubes\"" "effects state hover active"
mfx_assert_file_contains "$state_after_file" "\"hold_follow_mode\":\"smooth\"" "effects state hold follow mode alias normalization"
if ! mfx_file_contains_fixed "$state_after_file" "\"trail_move_samples\":"; then
    if [[ "$skip_build" -eq 1 ]]; then
        mfx_fail "effects state trail diagnostics fields missing (likely stale host binary); rerun without --skip-build"
    fi
    mfx_fail "effects state trail diagnostics fields missing in /api/state"
fi

overlay_probe_file="$tmp_dir/effects-overlay-probe.out"
overlay_probe_code="$(mfx_http_code "$overlay_probe_file" "$MFX_MANUAL_BASE_URL/api/effects/test-overlay-windows" \
    -X POST \
    -H "$token_header" \
    -H "Content-Type: application/json" \
    -d '{"emit_click":true,"emit_trail":true,"emit_scroll":true,"emit_hold":true,"emit_hover":true,"click_type":"text","trail_type":"meteor","scroll_type":"helix","hold_type":"hologram","hover_type":"tubes","close_persistent":true,"wait_ms":80,"wait_for_clear_ms":1600}')"
mfx_assert_eq "$overlay_probe_code" "200" "effects overlay probe status"
mfx_assert_file_contains "$overlay_probe_file" "\"ok\":true" "effects overlay probe ok"
mfx_assert_file_contains "$overlay_probe_file" "\"click_type\":\"text\"" "effects overlay click type"
mfx_assert_file_contains "$overlay_probe_file" "\"trail_type\":\"meteor\"" "effects overlay trail type"
mfx_assert_file_contains "$overlay_probe_file" "\"scroll_type\":\"helix\"" "effects overlay scroll type"
mfx_assert_file_contains "$overlay_probe_file" "\"hold_type\":\"hologram\"" "effects overlay hold type"
mfx_assert_file_contains "$overlay_probe_file" "\"hover_type\":\"tubes\"" "effects overlay hover type"
mfx_assert_file_contains "$overlay_probe_file" "\"before_text_effect_fallback_show_count\":" "effects overlay text fallback before count"
mfx_assert_file_contains "$overlay_probe_file" "\"after_text_effect_fallback_show_count\":" "effects overlay text fallback after count"

before_text_fallback_count="$(parse_uint_field "$overlay_probe_file" "before_text_effect_fallback_show_count")"
after_text_fallback_count="$(parse_uint_field "$overlay_probe_file" "after_text_effect_fallback_show_count")"
if [[ -z "$before_text_fallback_count" || -z "$after_text_fallback_count" ]]; then
    mfx_fail "effects overlay text fallback count parse failed"
fi
if (( after_text_fallback_count < before_text_fallback_count )); then
    mfx_fail "effects overlay text fallback count regressed: before=$before_text_fallback_count after=$after_text_fallback_count"
fi

overlay_text_click_probe_file="$tmp_dir/effects-overlay-text-click-probe.out"
overlay_text_click_probe_code="$(mfx_http_code "$overlay_text_click_probe_file" "$MFX_MANUAL_BASE_URL/api/effects/test-overlay-windows" \
    -X POST \
    -H "$token_header" \
    -H "Content-Type: application/json" \
    -d '{"emit_text_click_effect":true,"text_click_text":"MFX_TEXT_CLICK_PROBE","text_click_font_size_px":112,"close_persistent":true,"wait_ms":140,"wait_for_clear_ms":400}')"
mfx_assert_eq "$overlay_text_click_probe_code" "200" "effects overlay text click probe status"
mfx_assert_file_contains "$overlay_text_click_probe_file" "\"ok\":true" "effects overlay text click probe ok"
mfx_assert_file_contains "$overlay_text_click_probe_file" "\"emit_text_click_effect\":true" "effects overlay text click emit flag"
mfx_assert_file_contains "$overlay_text_click_probe_file" "\"text_click_text\":\"MFX_TEXT_CLICK_PROBE\"" "effects overlay text click text echo"
text_click_before_count="$(parse_uint_field "$overlay_text_click_probe_file" "before_text_effect_fallback_show_count")"
text_click_after_count="$(parse_uint_field "$overlay_text_click_probe_file" "after_text_effect_fallback_show_count")"
if [[ -z "$text_click_before_count" || -z "$text_click_after_count" ]]; then
    mfx_fail "effects overlay text click fallback count parse failed"
fi
if (( text_click_after_count <= text_click_before_count )); then
    mfx_fail "effects overlay text click expected fallback show increase: before=$text_click_before_count after=$text_click_after_count"
fi

overlay_line_trail_probe_file="$tmp_dir/effects-overlay-line-trail-probe.out"
overlay_line_trail_probe_code="$(mfx_http_code "$overlay_line_trail_probe_file" "$MFX_MANUAL_BASE_URL/api/effects/test-overlay-windows" \
    -X POST \
    -H "$token_header" \
    -H "Content-Type: application/json" \
    -d '{"emit_line_trail":true,"reset_line_trail":true,"close_persistent":false,"wait_ms":160,"wait_for_clear_ms":0,"line_trail_steps":10,"line_trail_duration_ms":1200,"line_trail_line_width_px":4}')"
mfx_assert_eq "$overlay_line_trail_probe_code" "200" "effects overlay line trail probe status"
mfx_assert_file_contains "$overlay_line_trail_probe_file" "\"ok\":true" "effects overlay line trail probe ok"
mfx_assert_file_contains "$overlay_line_trail_probe_file" "\"emit_line_trail\":true" "effects overlay line trail emit flag"
mfx_assert_file_contains "$overlay_line_trail_probe_file" "\"after_line_trail_active\":true" "effects overlay line trail active"
line_trail_after_count="$(parse_uint_field "$overlay_line_trail_probe_file" "after_line_trail_point_count")"
if [[ -z "$line_trail_after_count" ]]; then
    mfx_fail "effects overlay line trail point count parse failed"
fi
if (( line_trail_after_count <= 0 )); then
    mfx_fail "effects overlay line trail expected point count > 0, got $line_trail_after_count"
fi

line_trail_state_active_file="$tmp_dir/effects-line-trail-state-active.out"
line_trail_state_active_code="$(mfx_http_code "$line_trail_state_active_file" "$MFX_MANUAL_BASE_URL/api/state" \
    -X GET \
    -H "$token_header")"
mfx_assert_eq "$line_trail_state_active_code" "200" "effects line trail active state status"
mfx_assert_file_contains "$line_trail_state_active_file" "\"line_trail_active\":true" "effects line trail active state flag"
mfx_assert_file_contains "$line_trail_state_active_file" "\"trail_move_samples\":" "effects line trail active state move sample field"
mfx_assert_file_contains "$line_trail_state_active_file" "\"trail_origin_connector_drop_count\":" "effects line trail active state origin connector drop field"
mfx_assert_file_contains "$line_trail_state_active_file" "\"trail_teleport_drop_count\":" "effects line trail active state teleport drop field"
line_trail_state_active_count="$(parse_uint_field "$line_trail_state_active_file" "line_trail_point_count")"
if [[ -z "$line_trail_state_active_count" ]]; then
    mfx_fail "effects line trail active state point count parse failed"
fi
if (( line_trail_state_active_count <= 0 )); then
    mfx_fail "effects line trail active state expected point count > 0, got $line_trail_state_active_count"
fi

overlay_trail_none_probe_file="$tmp_dir/effects-overlay-trail-none-probe.out"
overlay_trail_none_probe_code="$(mfx_http_code "$overlay_trail_none_probe_file" "$MFX_MANUAL_BASE_URL/api/effects/test-overlay-windows" \
    -X POST \
    -H "$token_header" \
    -H "Content-Type: application/json" \
    -d '{"emit_trail":true,"trail_type":"none","close_persistent":true,"reset_line_trail":true,"wait_ms":40,"wait_for_clear_ms":400}')"
mfx_assert_eq "$overlay_trail_none_probe_code" "200" "effects overlay trail none probe status"
mfx_assert_file_contains "$overlay_trail_none_probe_file" "\"ok\":true" "effects overlay trail none probe ok"
mfx_assert_file_contains "$overlay_trail_none_probe_file" "\"trail_type\":\"none\"" "effects overlay trail none probe trail type"
mfx_assert_file_contains "$overlay_trail_none_probe_file" "\"before_line_trail_active\":false" "effects overlay trail none probe before inactive"
mfx_assert_file_contains "$overlay_trail_none_probe_file" "\"after_line_trail_active\":false" "effects overlay trail none probe after inactive"
mfx_assert_file_contains "$overlay_trail_none_probe_file" "\"before_line_trail_point_count\":0" "effects overlay trail none probe before point count"
mfx_assert_file_contains "$overlay_trail_none_probe_file" "\"after_line_trail_point_count\":0" "effects overlay trail none probe after point count"

line_trail_state_cleared_file="$tmp_dir/effects-line-trail-state-cleared.out"
line_trail_state_cleared_code="$(mfx_http_code "$line_trail_state_cleared_file" "$MFX_MANUAL_BASE_URL/api/state" \
    -X GET \
    -H "$token_header")"
mfx_assert_eq "$line_trail_state_cleared_code" "200" "effects line trail cleared state status"
mfx_assert_file_contains "$line_trail_state_cleared_file" "\"line_trail_active\":false" "effects line trail cleared state flag"
mfx_assert_file_contains "$line_trail_state_cleared_file" "\"line_trail_point_count\":0" "effects line trail cleared state point count"
mfx_assert_file_contains "$line_trail_state_cleared_file" "\"trail_move_samples\":" "effects line trail cleared state move sample field"
mfx_assert_file_contains "$line_trail_state_cleared_file" "\"trail_origin_connector_drop_count\":" "effects line trail cleared state origin connector drop field"
mfx_assert_file_contains "$line_trail_state_cleared_file" "\"trail_teleport_drop_count\":" "effects line trail cleared state teleport drop field"

mfx_ok "macos effects type parity selfcheck passed"

if [[ "$keep_running" -eq 1 ]]; then
    if [[ "$auto_stop_seconds" -gt 0 ]]; then
        mfx_manual_schedule_auto_stop "$MFX_MANUAL_HOST_PID" "$auto_stop_seconds"
        printf 'auto_stop_seconds=%s\n' "$auto_stop_seconds"
    fi
    mfx_manual_print_stop_command "$MFX_MANUAL_HOST_PID"
fi
