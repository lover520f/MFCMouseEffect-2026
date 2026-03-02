#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_read_json_bool() {
    local file_path="$1"
    local dotted_path="$2"

    python3 - "$file_path" "$dotted_path" <<'PY'
import json
import sys

file_path = sys.argv[1]
dotted_path = sys.argv[2]

with open(file_path, "r", encoding="utf-8") as f:
    root = json.load(f)

value = root
for part in dotted_path.split("."):
    if not isinstance(value, dict) or part not in value:
        print(f"missing:{dotted_path}", file=sys.stderr)
        sys.exit(2)
    value = value[part]

if not isinstance(value, bool):
    print(f"not_bool:{dotted_path}", file=sys.stderr)
    sys.exit(3)

sys.stdout.write("true" if value else "false")
PY
}

_mfx_core_http_read_json_string() {
    local file_path="$1"
    local dotted_path="$2"

    python3 - "$file_path" "$dotted_path" <<'PY'
import json
import sys

file_path = sys.argv[1]
dotted_path = sys.argv[2]

with open(file_path, "r", encoding="utf-8") as f:
    root = json.load(f)

value = root
for part in dotted_path.split("."):
    if not isinstance(value, dict) or part not in value:
        print(f"missing:{dotted_path}", file=sys.stderr)
        sys.exit(2)
    value = value[part]

if not isinstance(value, str):
    print(f"not_string:{dotted_path}", file=sys.stderr)
    sys.exit(3)

sys.stdout.write(value)
PY
}

_mfx_core_http_run_state_checks() {
    local platform="$1"
    local tmp_dir="$2"
    local settings_url="$3"
    local base_url="$4"
    local token="$5"

    local code_root
    code_root="$(mfx_http_code "$tmp_dir/root.out" "$settings_url")"
    mfx_assert_eq "$code_root" "200" "core root status"

    local code_js
    code_js="$(mfx_http_code "$tmp_dir/settings-js.out" "$base_url/settings-shell.svelte.js?token=$token")"
    mfx_assert_eq "$code_js" "200" "core settings-shell js status"

    local code_state
    code_state="$(mfx_http_code "$tmp_dir/state.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_state" "200" "core state status"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"automation\":" "core state automation section"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"input_capture\":" "core state input_capture section"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"effects_suspended_vm\":" "core state input_capture vm suppression field"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"effects_suspended_vm_check_interval_ms\":" "core state input_capture vm suppression interval field"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"wasm\":" "core state wasm section"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"effects_runtime\":" "core state effects_runtime section"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"effects_profile\":" "core state effects_profile section"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"click_active_overlay_windows\":" "core effects runtime click overlay count"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"trail_active_overlay_windows\":" "core effects runtime trail overlay count"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"scroll_active_overlay_windows\":" "core effects runtime scroll overlay count"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"hold_active_overlay_windows\":" "core effects runtime hold overlay count"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"hover_active_overlay_windows\":" "core effects runtime hover overlay count"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"active_overlay_windows_total\":" "core effects runtime total overlay count"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"active\":{\"click\":" "core effects profile active selections"
    if [[ "$platform" == "macos" ]]; then
        mfx_assert_file_contains "$tmp_dir/state.out" "\"platform\":\"macos\"" "core effects profile platform macos"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"config_basis\":{" "core effects profile config basis section"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"ripple_duration_ms\":" "core effects profile config basis ripple duration"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"trail_profile_max_points\":" "core effects profile config basis trail max points"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"click\":{" "core effects profile click section"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"normal_size_px\":" "core effects profile click size field"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"trail\":{" "core effects profile trail section"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"particle_size_px\":" "core effects profile trail size field"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"trail_throttle\":{" "core effects profile throttle section"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"min_interval_ms\":" "core effects profile throttle interval field"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"scroll\":{" "core effects profile scroll section"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"vertical_size_px\":" "core effects profile scroll size field"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"hold\":{" "core effects profile hold section"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"progress_full_ms\":" "core effects profile hold progress field"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"hover\":{" "core effects profile hover section"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"spin_duration_sec\":" "core effects profile hover spin field"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"overlay_max_inflight\":" "core wasm overlay policy max in-flight"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"overlay_min_image_interval_ms\":" "core wasm overlay policy image interval"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"overlay_min_text_interval_ms\":" "core wasm overlay policy text interval"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"mac_image_overlay_requests\":" "core wasm mac image overlay requests"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"mac_image_overlay_requests_with_asset\":" "core wasm mac image overlay requests with asset"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"mac_image_overlay_apply_tint_requests\":" "core wasm mac image overlay tint requests"
        mfx_assert_file_contains "$tmp_dir/state.out" "\"mac_image_overlay_apply_tint_requests_with_asset\":" "core wasm mac image overlay tint requests with asset"
        if [[ -n "${MFX_EXPECT_MACOS_WASM_OVERLAY_MAX_INFLIGHT:-}" ]]; then
            mfx_assert_file_contains "$tmp_dir/state.out" "\"overlay_max_inflight\":${MFX_EXPECT_MACOS_WASM_OVERLAY_MAX_INFLIGHT}" "core wasm overlay policy max in-flight value"
        fi
        if [[ -n "${MFX_EXPECT_MACOS_WASM_IMAGE_MIN_INTERVAL_MS:-}" ]]; then
            mfx_assert_file_contains "$tmp_dir/state.out" "\"overlay_min_image_interval_ms\":${MFX_EXPECT_MACOS_WASM_IMAGE_MIN_INTERVAL_MS}" "core wasm overlay policy image interval value"
        fi
        if [[ -n "${MFX_EXPECT_MACOS_WASM_TEXT_MIN_INTERVAL_MS:-}" ]]; then
            mfx_assert_file_contains "$tmp_dir/state.out" "\"overlay_min_text_interval_ms\":${MFX_EXPECT_MACOS_WASM_TEXT_MIN_INTERVAL_MS}" "core wasm overlay policy text interval value"
        fi
    else
        mfx_assert_file_contains "$tmp_dir/state.out" "\"platform\":\"non_macos\"" "core effects profile platform non-macos"
    fi
    mfx_assert_file_contains "$tmp_dir/state.out" "\"invoke_supported\":" "core wasm invoke capability"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"render_supported\":" "core wasm render capability"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"last_throttled_render_commands\":" "core wasm throttled render diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"last_throttled_by_capacity_render_commands\":" "core wasm throttled-by-capacity diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"last_throttled_by_interval_render_commands\":" "core wasm throttled-by-interval diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"lifetime_invoke_calls\":" "core wasm lifetime invoke diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"lifetime_invoke_success_calls\":" "core wasm lifetime invoke success diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"lifetime_invoke_failed_calls\":" "core wasm lifetime invoke failed diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"lifetime_render_dispatches\":" "core wasm lifetime render dispatch diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"lifetime_executed_text_commands\":" "core wasm lifetime text diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"lifetime_executed_image_commands\":" "core wasm lifetime image diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"lifetime_throttled_render_commands\":" "core wasm lifetime throttled diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"lifetime_throttled_by_capacity_render_commands\":" "core wasm lifetime throttled-by-capacity diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"lifetime_throttled_by_interval_render_commands\":" "core wasm lifetime throttled-by-interval diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"lifetime_dropped_render_commands\":" "core wasm lifetime dropped diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"last_load_failure_stage\":" "core wasm load-failure stage diagnostics"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"last_load_failure_code\":" "core wasm load-failure code diagnostics"

    local code_state_unauthorized
    code_state_unauthorized="$(mfx_http_code "$tmp_dir/state-unauth.out" "$base_url/api/state")"
    mfx_assert_eq "$code_state_unauthorized" "401" "core state unauthorized status"

    local code_schema
    code_schema="$(mfx_http_code "$tmp_dir/schema.out" "$base_url/api/schema" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_schema" "200" "core schema status"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"capabilities\":" "core schema capabilities section"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"wasm\":" "core schema wasm capabilities section"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"input_capture\":" "core schema input_capture section"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"effects_suspended_vm\"" "core schema input_capture vm suppression key"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"effects_suspended_vm_check_interval_ms\"" "core schema input_capture vm suppression interval key"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"lifetime_invoke_calls\"" "core schema wasm lifetime invoke key"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"lifetime_render_dispatches\"" "core schema wasm lifetime render key"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"lifetime_throttled_render_commands\"" "core schema wasm lifetime throttled key"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"mac_image_overlay_requests\"" "core schema wasm mac image overlay requests key"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"mac_image_overlay_requests_with_asset\"" "core schema wasm mac image overlay requests with asset key"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"mac_image_overlay_apply_tint_requests\"" "core schema wasm mac image overlay tint requests key"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"mac_image_overlay_apply_tint_requests_with_asset\"" "core schema wasm mac image overlay tint requests with asset key"

    local schema_platform
    schema_platform="$(_mfx_core_http_read_json_string "$tmp_dir/schema.out" "capabilities.platform")"
    if [[ "$platform" == "macos" ]]; then
        mfx_assert_eq "$schema_platform" "macos" "core schema capabilities platform value"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.effects.click")" "true" "core schema capabilities effects.click"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.effects.trail")" "true" "core schema capabilities effects.trail"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.effects.scroll")" "true" "core schema capabilities effects.scroll"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.effects.hold")" "true" "core schema capabilities effects.hold"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.effects.hover")" "true" "core schema capabilities effects.hover"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.input.global_hook")" "true" "core schema capabilities input.global_hook"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.input.cursor_position")" "true" "core schema capabilities input.cursor_position"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.input.keyboard_injector")" "true" "core schema capabilities input.keyboard_injector"
    elif [[ "$platform" == "linux" ]]; then
        mfx_assert_eq "$schema_platform" "linux" "core schema capabilities platform value"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.effects.click")" "true" "core schema capabilities effects.click"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.effects.trail")" "false" "core schema capabilities effects.trail"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.effects.scroll")" "false" "core schema capabilities effects.scroll"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.effects.hold")" "false" "core schema capabilities effects.hold"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.effects.hover")" "false" "core schema capabilities effects.hover"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.input.global_hook")" "false" "core schema capabilities input.global_hook"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.input.cursor_position")" "false" "core schema capabilities input.cursor_position"
        mfx_assert_eq "$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.input.keyboard_injector")" "false" "core schema capabilities input.keyboard_injector"
    fi

    local state_wasm_invoke_supported
    local state_wasm_render_supported
    local schema_wasm_invoke
    local schema_wasm_render
    _mfx_core_http_read_json_bool "$tmp_dir/state.out" "input_capture.effects_suspended_vm" >/dev/null
    state_wasm_invoke_supported="$(_mfx_core_http_read_json_bool "$tmp_dir/state.out" "wasm.invoke_supported")"
    state_wasm_render_supported="$(_mfx_core_http_read_json_bool "$tmp_dir/state.out" "wasm.render_supported")"
    schema_wasm_invoke="$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.wasm.invoke")"
    schema_wasm_render="$(_mfx_core_http_read_json_bool "$tmp_dir/schema.out" "capabilities.wasm.render")"
    mfx_assert_eq "$state_wasm_invoke_supported" "$schema_wasm_invoke" "core wasm invoke capability schema/state parity"
    mfx_assert_eq "$state_wasm_render_supported" "$schema_wasm_render" "core wasm render capability schema/state parity"
}
