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

_mfx_core_http_read_json_uint() {
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

if not isinstance(value, int) or value < 0:
    print(f"not_uint:{dotted_path}", file=sys.stderr)
    sys.exit(3)

sys.stdout.write(str(value))
PY
}

_mfx_core_http_read_json_array_size() {
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

if not isinstance(value, list):
    print(f"not_array:{dotted_path}", file=sys.stderr)
    sys.exit(3)

sys.stdout.write(str(len(value)))
PY
}

_mfx_core_http_json_array_contains_string() {
    local file_path="$1"
    local dotted_path="$2"
    local expected="$3"

    python3 - "$file_path" "$dotted_path" "$expected" <<'PY'
import json
import sys

file_path = sys.argv[1]
dotted_path = sys.argv[2]
expected = sys.argv[3]

with open(file_path, "r", encoding="utf-8") as f:
    root = json.load(f)

value = root
for part in dotted_path.split("."):
    if not isinstance(value, dict) or part not in value:
        print(f"missing:{dotted_path}", file=sys.stderr)
        sys.exit(2)
    value = value[part]

if not isinstance(value, list):
    print(f"not_array:{dotted_path}", file=sys.stderr)
    sys.exit(3)

found = False
for item in value:
    if isinstance(item, str) and item == expected:
        found = True
        break
    if isinstance(item, dict) and isinstance(item.get("value"), str) and item["value"] == expected:
        found = True
        break

sys.stdout.write("1" if found else "0")
PY
}

_mfx_core_http_write_theme_catalog_payload() {
    local output_file="$1"
    local root_path="$2"

    python3 - "$output_file" "$root_path" <<'PY'
import json
import sys

output_file = sys.argv[1]
root_path = sys.argv[2]

with open(output_file, "w", encoding="utf-8") as f:
    json.dump({"theme_catalog_root_path": root_path}, f, ensure_ascii=False)
PY
}

_mfx_core_http_write_theme_payload() {
    local output_file="$1"
    local theme_value="$2"

    python3 - "$output_file" "$theme_value" <<'PY'
import json
import sys

output_file = sys.argv[1]
theme_value = sys.argv[2]

with open(output_file, "w", encoding="utf-8") as f:
    json.dump({"theme": theme_value}, f, ensure_ascii=False)
PY
}

_mfx_core_http_assert_wasm_runtime_backend_consistency() {
    local platform="$1"
    local state_file="$2"
    local schema_wasm_invoke="$3"
    local schema_wasm_render="$4"

    local state_wasm_runtime_backend
    local state_wasm_fallback_reason
    state_wasm_runtime_backend="$(_mfx_core_http_read_json_string "$state_file" "wasm.runtime_backend")"
    state_wasm_fallback_reason="$(_mfx_core_http_read_json_string "$state_file" "wasm.runtime_fallback_reason")"

    case "$state_wasm_runtime_backend" in
        dynamic_bridge|wasm3_static|null|external)
            ;;
        *)
            mfx_fail "core wasm runtime backend value: unsupported '$state_wasm_runtime_backend'"
            ;;
    esac

    if [[ "$state_wasm_runtime_backend" == "null" && -z "$state_wasm_fallback_reason" ]]; then
        mfx_fail "core wasm runtime backend null requires non-empty runtime_fallback_reason"
    fi

    if [[ "$platform" == "macos" ]]; then
        if [[ "$schema_wasm_invoke" != "true" || "$schema_wasm_render" != "true" ]]; then
            mfx_fail "core schema wasm capability matrix on macos must be invoke=true/render=true"
        fi
    elif [[ "$platform" == "linux" ]]; then
        if [[ "$schema_wasm_invoke" != "false" || "$schema_wasm_render" != "false" ]]; then
            mfx_fail "core schema wasm capability matrix on linux must be invoke=false/render=false"
        fi
    fi
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
    mfx_assert_file_contains "$tmp_dir/state.out" "\"theme_catalog_root_path\":" "core state theme catalog root path"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"theme_catalog_runtime\":" "core state theme catalog runtime section"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"runtime_theme_count\":" "core state theme catalog runtime count"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"external_theme_count\":" "core state theme catalog external count"
    mfx_assert_file_contains "$tmp_dir/state.out" "\"rejected_external_theme_files\":" "core state theme catalog rejected count"
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
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"themes\":" "core schema themes options section"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"theme_catalog\":" "core schema theme catalog section"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"runtime_theme_count\":" "core schema theme catalog runtime count"
    mfx_assert_file_contains "$tmp_dir/schema.out" "\"folder_picker_supported\":" "core schema theme catalog folder picker support"
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
    _mfx_core_http_assert_wasm_runtime_backend_consistency \
        "$platform" \
        "$tmp_dir/state.out" \
        "$schema_wasm_invoke" \
        "$schema_wasm_render"

    local schema_theme_count
    local schema_theme_runtime_count
    local state_theme_runtime_count
    schema_theme_count="$(_mfx_core_http_read_json_array_size "$tmp_dir/schema.out" "themes")"
    schema_theme_runtime_count="$(_mfx_core_http_read_json_uint "$tmp_dir/schema.out" "theme_catalog.runtime_theme_count")"
    state_theme_runtime_count="$(_mfx_core_http_read_json_uint "$tmp_dir/state.out" "theme_catalog_runtime.runtime_theme_count")"
    mfx_assert_eq "$schema_theme_count" "$schema_theme_runtime_count" "core theme catalog schema themes/runtime count parity"
    mfx_assert_eq "$state_theme_runtime_count" "$schema_theme_runtime_count" "core theme catalog schema/state runtime count parity"
    mfx_assert_eq \
        "$(_mfx_core_http_json_array_contains_string "$tmp_dir/schema.out" "themes" "$(_mfx_core_http_read_json_string "$tmp_dir/state.out" "theme")")" \
        "1" \
        "core theme value exists in schema themes options"

    local original_theme_value
    original_theme_value="$(_mfx_core_http_read_json_string "$tmp_dir/state.out" "theme")"

    local apply_chromatic_theme_payload="$tmp_dir/apply-chromatic-theme-contract.json"
    _mfx_core_http_write_theme_payload "$apply_chromatic_theme_payload" "chromatic"
    local code_apply_chromatic_theme
    code_apply_chromatic_theme="$(mfx_http_code "$tmp_dir/state-apply-chromatic-theme-contract.out" "$base_url/api/state" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        --data-binary "@$apply_chromatic_theme_payload")"
    mfx_assert_eq "$code_apply_chromatic_theme" "200" "core chromatic theme apply state status"
    mfx_assert_file_contains "$tmp_dir/state-apply-chromatic-theme-contract.out" "\"ok\":true" "core chromatic theme apply response ok"

    local code_state_after_chromatic_theme
    code_state_after_chromatic_theme="$(mfx_http_code "$tmp_dir/state-after-chromatic-theme-contract.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_state_after_chromatic_theme" "200" "core chromatic theme state-after status"
    mfx_assert_eq "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-chromatic-theme-contract.out" "theme")" "chromatic" "core chromatic theme preserved"
    local code_schema_after_chromatic_theme
    code_schema_after_chromatic_theme="$(mfx_http_code "$tmp_dir/schema-after-chromatic-theme-contract.out" "$base_url/api/schema" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_schema_after_chromatic_theme" "200" "core chromatic theme schema-after status"
    mfx_assert_eq \
        "$(_mfx_core_http_json_array_contains_string "$tmp_dir/schema-after-chromatic-theme-contract.out" "themes" "chromatic")" \
        "1" \
        "core chromatic theme exists in schema themes options"

    local invalid_theme_value="__mfx_contract_invalid_theme__"
    local apply_invalid_theme_payload="$tmp_dir/apply-invalid-theme-contract.json"
    _mfx_core_http_write_theme_payload "$apply_invalid_theme_payload" "$invalid_theme_value"
    local code_apply_invalid_theme
    code_apply_invalid_theme="$(mfx_http_code "$tmp_dir/state-apply-invalid-theme-contract.out" "$base_url/api/state" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        --data-binary "@$apply_invalid_theme_payload")"
    mfx_assert_eq "$code_apply_invalid_theme" "200" "core invalid theme apply state status"
    mfx_assert_file_contains "$tmp_dir/state-apply-invalid-theme-contract.out" "\"ok\":true" "core invalid theme apply response ok"

    local code_state_after_invalid_theme
    code_state_after_invalid_theme="$(mfx_http_code "$tmp_dir/state-after-invalid-theme-contract.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_state_after_invalid_theme" "200" "core invalid theme state-after status"
    local state_theme_after_invalid
    state_theme_after_invalid="$(_mfx_core_http_read_json_string "$tmp_dir/state-after-invalid-theme-contract.out" "theme")"
    if [[ "$state_theme_after_invalid" == "$invalid_theme_value" ]]; then
        mfx_fail "core invalid theme normalization failed: persisted invalid theme value"
    fi
    local code_schema_after_invalid_theme
    code_schema_after_invalid_theme="$(mfx_http_code "$tmp_dir/schema-after-invalid-theme-contract.out" "$base_url/api/schema" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_schema_after_invalid_theme" "200" "core invalid theme schema-after status"
    mfx_assert_eq \
        "$(_mfx_core_http_json_array_contains_string "$tmp_dir/schema-after-invalid-theme-contract.out" "themes" "$state_theme_after_invalid")" \
        "1" \
        "core invalid theme normalized value exists in schema themes options"

    local restore_theme_payload="$tmp_dir/restore-theme-contract.json"
    _mfx_core_http_write_theme_payload "$restore_theme_payload" "$original_theme_value"
    local code_restore_theme
    code_restore_theme="$(mfx_http_code "$tmp_dir/state-restore-theme-contract.out" "$base_url/api/state" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        --data-binary "@$restore_theme_payload")"
    mfx_assert_eq "$code_restore_theme" "200" "core theme restore state status"
    mfx_assert_file_contains "$tmp_dir/state-restore-theme-contract.out" "\"ok\":true" "core theme restore response ok"

    local code_state_after_theme_restore
    code_state_after_theme_restore="$(mfx_http_code "$tmp_dir/state-after-theme-restore-contract.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_state_after_theme_restore" "200" "core theme state-after-restore status"
    mfx_assert_eq "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-theme-restore-contract.out" "theme")" "$original_theme_value" "core theme value restored"

    local code_theme_catalog_picker_probe
    code_theme_catalog_picker_probe="$(mfx_http_code "$tmp_dir/theme-catalog-picker-probe.out" "$base_url/api/theme/catalog-folder-dialog" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"probe_only":true}')"
    mfx_assert_eq "$code_theme_catalog_picker_probe" "200" "core theme catalog picker probe status"
    mfx_assert_file_contains "$tmp_dir/theme-catalog-picker-probe.out" "\"ok\":true" "core theme catalog picker probe ok"
    mfx_assert_file_contains "$tmp_dir/theme-catalog-picker-probe.out" "\"probe_only\":true" "core theme catalog picker probe mode"
    mfx_assert_file_contains "$tmp_dir/theme-catalog-picker-probe.out" "\"supported\":" "core theme catalog picker probe support field"

    local original_theme_catalog_root_path
    original_theme_catalog_root_path="$(_mfx_core_http_read_json_string "$tmp_dir/state.out" "theme_catalog_root_path")"

    local contract_theme_root="$tmp_dir/theme-catalog-contract"
    mkdir -p "$contract_theme_root/pack"
    cat > "$contract_theme_root/pack/contract.theme.json" <<'JSON'
{
  "value": "contract_external_theme",
  "label_zh": "Contract Theme",
  "label_en": "Contract Theme",
  "palette": {
    "click": {
      "duration_ms": 300,
      "window_size": 210,
      "start_radius": 10.0,
      "end_radius": 54.0,
      "stroke_width": 2.5,
      "fill": "#2233AA66",
      "stroke": "#44CCFF",
      "glow": "#66CCFF"
    },
    "icon": {
      "duration_ms": 320,
      "window_size": 220,
      "start_radius": 10.0,
      "end_radius": 52.0,
      "stroke_width": 2.4,
      "fill": "#11224455",
      "stroke": "#66DDEE",
      "glow": "#77DDEE"
    },
    "scroll": {
      "duration_ms": 240,
      "window_size": 190,
      "start_radius": 8.0,
      "end_radius": 46.0,
      "stroke_width": 2.2,
      "fill": "#11335544",
      "stroke": "#77EEFF",
      "glow": "#88EEFF"
    },
    "hold": {
      "duration_ms": 900,
      "window_size": 230,
      "start_radius": 12.0,
      "end_radius": 64.0,
      "stroke_width": 2.8,
      "fill": "#00000000",
      "stroke": "#55DDEE",
      "glow": "#66DDEE"
    },
    "hover": {
      "duration_ms": 2600,
      "window_size": 200,
      "start_radius": 6.0,
      "end_radius": 58.0,
      "stroke_width": 2.0,
      "fill": "#00000000",
      "stroke": "#88EEFF",
      "glow": "#44EEFF"
    }
  }
}
JSON

    cat > "$contract_theme_root/pack/invalid.theme.json" <<'JSON'
{
  "value": "bad theme !",
  "label_zh": "Invalid Theme",
  "label_en": "Invalid Theme",
  "palette": {
    "click": {
      "duration_ms": 200,
      "window_size": 180,
      "start_radius": 8.0,
      "end_radius": 40.0,
      "stroke_width": 2.0,
      "fill": "#22000000",
      "stroke": "#33AAFF",
      "glow": "#33AAFF"
    },
    "icon": {
      "duration_ms": 200,
      "window_size": 180,
      "start_radius": 8.0,
      "end_radius": 40.0,
      "stroke_width": 2.0,
      "fill": "#22000000",
      "stroke": "#33AAFF",
      "glow": "#33AAFF"
    },
    "scroll": {
      "duration_ms": 200,
      "window_size": 180,
      "start_radius": 8.0,
      "end_radius": 40.0,
      "stroke_width": 2.0,
      "fill": "#22000000",
      "stroke": "#33AAFF",
      "glow": "#33AAFF"
    },
    "hold": {
      "duration_ms": 200,
      "window_size": 180,
      "start_radius": 8.0,
      "end_radius": 40.0,
      "stroke_width": 2.0,
      "fill": "#22000000",
      "stroke": "#33AAFF",
      "glow": "#33AAFF"
    },
    "hover": {
      "duration_ms": 200,
      "window_size": 180,
      "start_radius": 8.0,
      "end_radius": 40.0,
      "stroke_width": 2.0,
      "fill": "#22000000",
      "stroke": "#33AAFF",
      "glow": "#33AAFF"
    }
  }
}
JSON

    cat > "$contract_theme_root/pack/neon.theme.json" <<'JSON'
{
  "value": "neon",
  "label_zh": "External Neon Override",
  "label_en": "External Neon Override",
  "palette": {
    "click": {
      "duration_ms": 360,
      "window_size": 230,
      "start_radius": 11.0,
      "end_radius": 66.0,
      "stroke_width": 3.1,
      "fill": "#22AA3344",
      "stroke": "#11AA22",
      "glow": "#33BB44"
    },
    "icon": {
      "duration_ms": 350,
      "window_size": 220,
      "start_radius": 10.0,
      "end_radius": 58.0,
      "stroke_width": 2.9,
      "fill": "#22AA3344",
      "stroke": "#11AA22",
      "glow": "#33BB44"
    },
    "scroll": {
      "duration_ms": 250,
      "window_size": 190,
      "start_radius": 8.0,
      "end_radius": 46.0,
      "stroke_width": 2.4,
      "fill": "#22AA3344",
      "stroke": "#11AA22",
      "glow": "#33BB44"
    },
    "hold": {
      "duration_ms": 920,
      "window_size": 240,
      "start_radius": 12.0,
      "end_radius": 70.0,
      "stroke_width": 3.0,
      "fill": "#00000000",
      "stroke": "#11AA22",
      "glow": "#33BB44"
    },
    "hover": {
      "duration_ms": 2600,
      "window_size": 210,
      "start_radius": 7.0,
      "end_radius": 60.0,
      "stroke_width": 2.2,
      "fill": "#00000000",
      "stroke": "#11AA22",
      "glow": "#33BB44"
    }
  }
}
JSON

    local apply_theme_catalog_payload="$tmp_dir/apply-theme-catalog-contract.json"
    _mfx_core_http_write_theme_catalog_payload "$apply_theme_catalog_payload" "$contract_theme_root"
    local code_apply_theme_catalog
    code_apply_theme_catalog="$(mfx_http_code "$tmp_dir/state-apply-theme-catalog-contract.out" "$base_url/api/state" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        --data-binary "@$apply_theme_catalog_payload")"
    mfx_assert_eq "$code_apply_theme_catalog" "200" "core theme catalog apply state status"
    mfx_assert_file_contains "$tmp_dir/state-apply-theme-catalog-contract.out" "\"ok\":true" "core theme catalog apply response ok"

    local code_schema_after_theme_catalog
    code_schema_after_theme_catalog="$(mfx_http_code "$tmp_dir/schema-after-theme-catalog.out" "$base_url/api/schema" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_schema_after_theme_catalog" "200" "core theme catalog schema-after status"
    mfx_assert_file_contains "$tmp_dir/schema-after-theme-catalog.out" "\"value\":\"contract_external_theme\"" "core theme catalog schema includes external theme"
    mfx_assert_eq "$(_mfx_core_http_read_json_string "$tmp_dir/schema-after-theme-catalog.out" "theme_catalog.configured_root_path")" "$contract_theme_root" "core theme catalog configured_root_path after apply"
    mfx_assert_eq "$(_mfx_core_http_read_json_uint "$tmp_dir/schema-after-theme-catalog.out" "theme_catalog.external_theme_count")" "1" "core theme catalog external count after apply"
    mfx_assert_eq "$(_mfx_core_http_read_json_uint "$tmp_dir/schema-after-theme-catalog.out" "theme_catalog.scanned_external_theme_files")" "3" "core theme catalog scanned file count after apply"
    mfx_assert_eq "$(_mfx_core_http_read_json_uint "$tmp_dir/schema-after-theme-catalog.out" "theme_catalog.rejected_external_theme_files")" "2" "core theme catalog rejected file count after apply"
    if mfx_file_contains_fixed "$tmp_dir/schema-after-theme-catalog.out" "External Neon Override"; then
        mfx_fail "core theme catalog built-in override guard failed: schema exposed external neon override label"
    fi
    mfx_assert_eq "$(_mfx_core_http_read_json_array_size "$tmp_dir/schema-after-theme-catalog.out" "themes")" "$(_mfx_core_http_read_json_uint "$tmp_dir/schema-after-theme-catalog.out" "theme_catalog.runtime_theme_count")" "core theme catalog schema parity after apply"

    local code_state_after_theme_catalog
    code_state_after_theme_catalog="$(mfx_http_code "$tmp_dir/state-after-theme-catalog.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_state_after_theme_catalog" "200" "core theme catalog state-after status"
    mfx_assert_eq "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-theme-catalog.out" "theme_catalog_root_path")" "$contract_theme_root" "core theme catalog root path in state after apply"
    mfx_assert_eq "$(_mfx_core_http_read_json_uint "$tmp_dir/state-after-theme-catalog.out" "theme_catalog_runtime.external_theme_count")" "1" "core theme catalog runtime external count after apply"
    mfx_assert_eq "$(_mfx_core_http_read_json_uint "$tmp_dir/state-after-theme-catalog.out" "theme_catalog_runtime.runtime_theme_count")" "$(_mfx_core_http_read_json_uint "$tmp_dir/schema-after-theme-catalog.out" "theme_catalog.runtime_theme_count")" "core theme catalog schema/state runtime parity after apply"
    mfx_assert_eq \
        "$(_mfx_core_http_json_array_contains_string "$tmp_dir/schema-after-theme-catalog.out" "themes" "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-theme-catalog.out" "theme")")" \
        "1" \
        "core theme value exists in schema themes options after apply"

    local restore_theme_catalog_payload="$tmp_dir/restore-theme-catalog.json"
    _mfx_core_http_write_theme_catalog_payload "$restore_theme_catalog_payload" "$original_theme_catalog_root_path"
    local code_restore_theme_catalog
    code_restore_theme_catalog="$(mfx_http_code "$tmp_dir/state-restore-theme-catalog.out" "$base_url/api/state" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        --data-binary "@$restore_theme_catalog_payload")"
    mfx_assert_eq "$code_restore_theme_catalog" "200" "core theme catalog restore state status"
    mfx_assert_file_contains "$tmp_dir/state-restore-theme-catalog.out" "\"ok\":true" "core theme catalog restore response ok"

    local code_state_after_restore_theme_catalog
    code_state_after_restore_theme_catalog="$(mfx_http_code "$tmp_dir/state-after-restore-theme-catalog.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_state_after_restore_theme_catalog" "200" "core theme catalog state-after-restore status"
    mfx_assert_eq "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-restore-theme-catalog.out" "theme_catalog_root_path")" "$original_theme_catalog_root_path" "core theme catalog root path restored"

    local code_schema_after_restore_theme_catalog
    code_schema_after_restore_theme_catalog="$(mfx_http_code "$tmp_dir/schema-after-restore-theme-catalog.out" "$base_url/api/schema" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_schema_after_restore_theme_catalog" "200" "core theme catalog schema-after-restore status"
    mfx_assert_eq \
        "$(_mfx_core_http_json_array_contains_string "$tmp_dir/schema-after-restore-theme-catalog.out" "themes" "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-restore-theme-catalog.out" "theme")")" \
        "1" \
        "core theme value exists in schema themes options after restore"
}
