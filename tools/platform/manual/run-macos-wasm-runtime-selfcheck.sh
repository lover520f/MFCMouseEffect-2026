#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../../.." && pwd)"

source "$repo_root/tools/platform/regression/lib/common.sh"
source "$repo_root/tools/platform/regression/lib/build.sh"
source "$repo_root/tools/platform/manual/lib/macos_core_host.sh"
source "$repo_root/tools/platform/manual/lib/wasm_selfcheck_common.sh"

usage() {
    cat <<'EOF'
Usage:
  tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh [options]

Options:
  --build-dir <path>          Build directory (default: /tmp/mfx-platform-macos-core-build)
  --manifest-path <path>      Plugin manifest path (default: examples/wasm-plugin-template/dist/plugin.json)
  --log-file <path>           Host log path (default: /tmp/mfx-core-wasm-selfcheck.log)
  --probe-file <path>         Probe file path (default: /tmp/mfx-core-wasm-selfcheck.probe)
  --jobs <num>                Build jobs (sets MFX_BUILD_JOBS)
  --skip-build                Skip cmake configure/build
  --open-settings             Open resolved settings URL in browser
  --keep-running              Keep host running after selfcheck
  --auto-stop-seconds <num>   Auto stop host after N seconds (only used with --keep-running, default: 120)
  -h, --help                  Show this help

Env tuning:
  MFX_MANUAL_WASM_DISPATCH_TIMEOUT_SECONDS         max wait for invoke/render ready in test-dispatch assertion (default: 5)
  MFX_MANUAL_WASM_DISPATCH_RETRY_INTERVAL_SECONDS  retry interval for test-dispatch assertion (default: 0.2)
EOF
}

build_dir="/tmp/mfx-platform-macos-core-build"
manifest_path="$repo_root/examples/wasm-plugin-template/dist/plugin.json"
log_file="/tmp/mfx-core-wasm-selfcheck.log"
probe_file="/tmp/mfx-core-wasm-selfcheck.probe"
skip_build=0
open_settings=0
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
    --manifest-path)
        [[ $# -ge 2 ]] || mfx_fail "missing value for --manifest-path"
        manifest_path="$2"
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
    --open-settings)
        open_settings=1
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
mfx_require_cmd sed

if [[ ! -f "$manifest_path" ]]; then
    mfx_fail "manifest file not found: $manifest_path"
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

start_status=0
mfx_manual_start_core_host "$host_bin" "$probe_file" "$log_file" "MFX_ENABLE_WASM_TEST_DISPATCH_API=1" || start_status=$?
if [[ "$start_status" -eq 2 ]]; then
    mfx_ok "macos wasm runtime selfcheck skipped: $MFX_MANUAL_STARTUP_SKIP_REASON"
    exit 0
fi
if [[ "$start_status" -ne 0 ]]; then
    exit "$start_status"
fi

printf 'mfx_pid=%s\n' "$MFX_MANUAL_HOST_PID"
printf 'settings_url=%s\n' "$MFX_MANUAL_SETTINGS_URL"
printf 'manifest_path=%s\n' "$manifest_path"
printf 'log_file=%s\n' "$MFX_MANUAL_LOG_FILE"

if [[ "$open_settings" -eq 1 ]]; then
    mfx_require_cmd open
    open "$MFX_MANUAL_SETTINGS_URL" >/dev/null 2>&1 || mfx_fail "failed to open settings url"
fi

token="$MFX_MANUAL_SETTINGS_TOKEN"
token_header="x-mfcmouseeffect-token: $token"

state_file="$tmp_dir/state.out"
code_state="$(mfx_http_code "$state_file" "$MFX_MANUAL_BASE_URL/api/state" -H "$token_header")"
mfx_assert_eq "$code_state" "200" "selfcheck state status"
mfx_assert_file_contains "$state_file" "\"runtime_backend\":\"wasm3_static\"" "selfcheck runtime backend"
mfx_assert_file_contains "$state_file" "\"render_supported\":true" "selfcheck render capability"

catalog_file="$tmp_dir/wasm-catalog.out"
code_catalog="$(mfx_http_code "$catalog_file" "$MFX_MANUAL_BASE_URL/api/wasm/catalog" \
    -X POST -H "$token_header" -H "Content-Type: application/json" -d '{}')"
mfx_assert_eq "$code_catalog" "200" "selfcheck wasm catalog status"
mfx_assert_file_contains "$catalog_file" "\"ok\":true" "selfcheck wasm catalog ok"
mfx_assert_file_contains "$catalog_file" "\"plugins\":" "selfcheck wasm catalog plugins field"

import_dialog_probe_file="$tmp_dir/wasm-import-dialog-probe.out"
mfx_wasm_selfcheck_assert_import_dialog_probe_supported \
    "wasm import dialog probe" "$import_dialog_probe_file" "$MFX_MANUAL_BASE_URL" "$token"

import_dialog_probe_trimmed_initial_path_file="$tmp_dir/wasm-import-dialog-probe-trimmed-initial-path.out"
mfx_wasm_selfcheck_assert_import_dialog_probe_trimmed_initial_path \
    "wasm import dialog probe trimmed initial path" \
    "$import_dialog_probe_trimmed_initial_path_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    "$repo_root/examples"

import_selected_file="$tmp_dir/wasm-import-selected.out"
mfx_wasm_selfcheck_assert_import_selected_ok \
    "wasm import-selected" "$import_selected_file" "$MFX_MANUAL_BASE_URL" "$token" "$manifest_path"

missing_import_manifest_path="${manifest_path}.missing"
missing_import_file="$tmp_dir/wasm-import-selected-missing.out"
mfx_wasm_selfcheck_assert_import_selected_failure \
    "wasm import-selected missing" "$missing_import_file" "$MFX_MANUAL_BASE_URL" "$token" \
    "$missing_import_manifest_path" "manifest_path_not_found"

not_file_manifest_path="$tmp_dir/manifest-path-not-file"
mkdir -p "$not_file_manifest_path"
not_file_import_file="$tmp_dir/wasm-import-selected-not-file.out"
mfx_wasm_selfcheck_assert_import_selected_failure \
    "wasm import-selected not file" "$not_file_import_file" "$MFX_MANUAL_BASE_URL" "$token" \
    "$not_file_manifest_path" "manifest_path_not_file"

required_import_file="$tmp_dir/wasm-import-selected-required.out"
mfx_wasm_selfcheck_assert_import_selected_failure \
    "wasm import-selected required" "$required_import_file" "$MFX_MANUAL_BASE_URL" "$token" \
    "   " "manifest_path_required"

export_all_file="$tmp_dir/wasm-export-all.out"
mfx_wasm_selfcheck_assert_export_all_ok \
    "wasm export-all" "$export_all_file" "$MFX_MANUAL_BASE_URL" "$token" "1"

load_file="$tmp_dir/wasm-load-manifest.out"
mfx_wasm_selfcheck_assert_load_manifest_ok \
    "wasm load-manifest" "$load_file" "$MFX_MANUAL_BASE_URL" "$token" "$manifest_path"

load_trimmed_file="$tmp_dir/wasm-load-manifest-trimmed.out"
mfx_wasm_selfcheck_assert_load_manifest_trimmed_path_ok \
    "wasm load-manifest trimmed path" "$load_trimmed_file" "$MFX_MANUAL_BASE_URL" "$token" "$manifest_path"

reload_route_file="$tmp_dir/wasm-reload-route.out"
mfx_wasm_selfcheck_assert_reload_ok \
    "wasm reload route" "$reload_route_file" "$MFX_MANUAL_BASE_URL" "$token"

reset_runtime_file="$tmp_dir/wasm-reset-runtime.out"
mfx_wasm_selfcheck_assert_test_reset_runtime_ok \
    "wasm reset runtime" "$reset_runtime_file" "$MFX_MANUAL_BASE_URL" "$token"

reload_without_target_file="$tmp_dir/wasm-reload-without-target.out"
mfx_wasm_selfcheck_reload_failure \
    "wasm reload without target" "$reload_without_target_file" "$MFX_MANUAL_BASE_URL" "$token" \
    "reload_target_missing"

load_after_reset_file="$tmp_dir/wasm-load-manifest-after-reset.out"
mfx_wasm_selfcheck_assert_load_manifest_ok \
    "wasm load-manifest after reset" "$load_after_reset_file" "$MFX_MANUAL_BASE_URL" "$token" "$manifest_path"

reload_failure_fixture_dir="$tmp_dir/wasm-reload-failure-fixture"
reload_failure_manifest_path="$(mfx_wasm_fixture_manifest_copy \
    "$manifest_path" \
    "$reload_failure_fixture_dir" \
    "selfcheck reload failure fixture")"
mfx_wasm_fixture_require_entry_file "$reload_failure_manifest_path" "selfcheck reload failure fixture" >/dev/null

reload_failure_fixture_load_file="$tmp_dir/wasm-load-manifest-reload-failure-fixture.out"
mfx_wasm_selfcheck_assert_load_manifest_ok \
    "wasm reload-failure fixture load" \
    "$reload_failure_fixture_load_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    "$reload_failure_manifest_path"

mfx_wasm_fixture_remove_entry_file "$reload_failure_manifest_path" "selfcheck reload failure fixture" >/dev/null

reload_missing_module_file="$tmp_dir/wasm-reload-missing-module.out"
mfx_wasm_selfcheck_reload_failure \
    "wasm reload missing module" \
    "$reload_missing_module_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    "module_load_failed" \
    "load_module" \
    "module_load_failed"

load_after_reload_failure_restore_file="$tmp_dir/wasm-load-manifest-after-reload-failure.out"
mfx_wasm_selfcheck_assert_load_manifest_ok \
    "wasm load-manifest restore after reload failure" \
    "$load_after_reload_failure_restore_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    "$manifest_path"

reload_api_fixture_dir="$tmp_dir/wasm-reload-api-fixture"
reload_api_manifest_path="$(mfx_wasm_fixture_manifest_copy \
    "$manifest_path" \
    "$reload_api_fixture_dir" \
    "selfcheck reload api fixture")"
reload_api_entry_relative="$(mfx_wasm_fixture_manifest_entry_relative \
    "$reload_api_manifest_path" \
    "selfcheck reload api fixture")"

reload_api_fixture_load_file="$tmp_dir/wasm-load-manifest-reload-api-fixture.out"
mfx_wasm_selfcheck_assert_load_manifest_ok \
    "wasm reload-api fixture load" \
    "$reload_api_fixture_load_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    "$reload_api_manifest_path"

mfx_wasm_fixture_write_manifest_with_api_version \
    "$reload_api_manifest_path" \
    "reload-api-unsupported-plugin" \
    "2" \
    "$reload_api_entry_relative"

reload_api_unsupported_file="$tmp_dir/wasm-reload-api-unsupported.out"
mfx_wasm_selfcheck_reload_failure \
    "wasm reload unsupported api" \
    "$reload_api_unsupported_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    "manifest_api_unsupported" \
    "manifest_api_version" \
    "manifest_api_unsupported"

load_after_api_reload_failure_restore_file="$tmp_dir/wasm-load-manifest-after-api-reload-failure.out"
mfx_wasm_selfcheck_assert_load_manifest_ok \
    "wasm load-manifest restore after api reload failure" \
    "$load_after_api_reload_failure_restore_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    "$manifest_path"

enable_file="$tmp_dir/wasm-enable.out"
code_enable="$(mfx_http_code "$enable_file" "$MFX_MANUAL_BASE_URL/api/wasm/enable" \
    -X POST -H "$token_header" -H "Content-Type: application/json" -d '{}')"
mfx_assert_eq "$code_enable" "200" "selfcheck wasm enable status"
mfx_assert_file_contains "$enable_file" "\"ok\":true" "selfcheck wasm enable ok"

dispatch_file="$tmp_dir/wasm-test-dispatch.out"
state_before_dispatch_file="$tmp_dir/state-before-dispatch.out"
code_state_before_dispatch="$(mfx_http_code "$state_before_dispatch_file" "$MFX_MANUAL_BASE_URL/api/state" -H "$token_header")"
mfx_assert_eq "$code_state_before_dispatch" "200" "selfcheck state before dispatch status"
state_before_invoke_supported="$(mfx_wasm_selfcheck_parse_bool_field "$state_before_dispatch_file" "invoke_supported")"
state_before_render_supported="$(mfx_wasm_selfcheck_parse_bool_field "$state_before_dispatch_file" "render_supported")"
enable_invoke_supported="$(mfx_wasm_selfcheck_parse_bool_field "$enable_file" "invoke_supported")"
enable_render_supported="$(mfx_wasm_selfcheck_parse_bool_field "$enable_file" "render_supported")"
mfx_assert_eq "$enable_invoke_supported" "$state_before_invoke_supported" "selfcheck wasm enable/state invoke_supported parity"
mfx_assert_eq "$enable_render_supported" "$state_before_render_supported" "selfcheck wasm enable/state render_supported parity"
state_before_overlay_max_inflight="$(mfx_wasm_selfcheck_parse_uint_field "$state_before_dispatch_file" "overlay_max_inflight")"
state_before_overlay_min_image_interval_ms="$(mfx_wasm_selfcheck_parse_uint_field "$state_before_dispatch_file" "overlay_min_image_interval_ms")"
state_before_overlay_min_text_interval_ms="$(mfx_wasm_selfcheck_parse_uint_field "$state_before_dispatch_file" "overlay_min_text_interval_ms")"
enable_overlay_max_inflight="$(mfx_wasm_selfcheck_parse_uint_field "$enable_file" "overlay_max_inflight")"
enable_overlay_min_image_interval_ms="$(mfx_wasm_selfcheck_parse_uint_field "$enable_file" "overlay_min_image_interval_ms")"
enable_overlay_min_text_interval_ms="$(mfx_wasm_selfcheck_parse_uint_field "$enable_file" "overlay_min_text_interval_ms")"
mfx_assert_eq "$enable_overlay_max_inflight" "$state_before_overlay_max_inflight" "selfcheck wasm enable/state overlay_max_inflight parity"
mfx_assert_eq "$enable_overlay_min_image_interval_ms" "$state_before_overlay_min_image_interval_ms" "selfcheck wasm enable/state overlay_min_image_interval_ms parity"
mfx_assert_eq "$enable_overlay_min_text_interval_ms" "$state_before_overlay_min_text_interval_ms" "selfcheck wasm enable/state overlay_min_text_interval_ms parity"

mfx_wasm_selfcheck_assert_test_dispatch_ok \
    "wasm test-dispatch" \
    "$dispatch_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    "true"

state_after_dispatch_file="$tmp_dir/state-after-dispatch.out"
code_state_after_dispatch="$(mfx_http_code "$state_after_dispatch_file" "$MFX_MANUAL_BASE_URL/api/state" -H "$token_header")"
mfx_assert_eq "$code_state_after_dispatch" "200" "selfcheck state after dispatch status"
mfx_wasm_selfcheck_assert_dispatch_diagnostics_consistent \
    "wasm dispatch diagnostics consistency" \
    "$dispatch_file" \
    "$state_after_dispatch_file" \
    "$state_before_dispatch_file" \
    "macos"

affine_translate_file="$tmp_dir/wasm-affine-translate.out"
code_affine_translate="$(mfx_wasm_selfcheck_test_resolve_image_affine_http_code \
    "$affine_translate_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    '{"x":100,"y":200,"scale":2.0,"rotation":0.5,"alpha":0.5,"life_ms":500,"delay_ms":17,"tint_rgba":16777215,"affine_enabled":false,"affine_dx":12,"affine_dy":-7}')"
mfx_assert_eq "$code_affine_translate" "200" "selfcheck wasm affine resolve translate status"
mfx_assert_file_contains "$affine_translate_file" "\"ok\":true" "selfcheck wasm affine resolve translate ok"
mfx_assert_file_contains "$affine_translate_file" "\"resolved_x_int\":112" "selfcheck wasm affine resolve translate x"
mfx_assert_file_contains "$affine_translate_file" "\"resolved_y_int\":193" "selfcheck wasm affine resolve translate y"
mfx_assert_file_contains "$affine_translate_file" "\"resolved_scale_milli\":2000" "selfcheck wasm affine resolve translate scale"
mfx_assert_file_contains "$affine_translate_file" "\"resolved_rotation_millirad\":500" "selfcheck wasm affine resolve translate rotation"
mfx_assert_file_contains "$affine_translate_file" "\"runtime_scale_milli\":2000" "selfcheck wasm affine resolve translate runtime scale"
mfx_assert_file_contains "$affine_translate_file" "\"runtime_alpha_milli\":500" "selfcheck wasm affine resolve translate runtime alpha"
mfx_assert_file_contains "$affine_translate_file" "\"runtime_delay_ms\":17" "selfcheck wasm affine resolve translate runtime delay"
mfx_assert_file_contains "$affine_translate_file" "\"runtime_life_ms\":500" "selfcheck wasm affine resolve translate runtime life"
mfx_assert_file_contains "$affine_translate_file" "\"runtime_apply_tint\":false" "selfcheck wasm affine resolve translate runtime tint"

affine_scale_file="$tmp_dir/wasm-affine-scale.out"
code_affine_scale="$(mfx_wasm_selfcheck_test_resolve_image_affine_http_code \
    "$affine_scale_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    '{"x":100,"y":200,"scale":2.0,"rotation":0.5,"alpha":0.8,"life_ms":700,"delay_ms":9,"tint_rgba":4281550003,"affine_enabled":true,"affine_dx":12,"affine_dy":-7,"affine_m11":2.0,"affine_m12":0.0,"affine_m21":0.0,"affine_m22":2.0}')"
mfx_assert_eq "$code_affine_scale" "200" "selfcheck wasm affine resolve scale status"
mfx_assert_file_contains "$affine_scale_file" "\"ok\":true" "selfcheck wasm affine resolve scale ok"
mfx_assert_file_contains "$affine_scale_file" "\"resolved_x_int\":112" "selfcheck wasm affine resolve scale x"
mfx_assert_file_contains "$affine_scale_file" "\"resolved_y_int\":193" "selfcheck wasm affine resolve scale y"
mfx_assert_file_contains "$affine_scale_file" "\"resolved_scale_milli\":4000" "selfcheck wasm affine resolve scale scale"
mfx_assert_file_contains "$affine_scale_file" "\"resolved_rotation_millirad\":500" "selfcheck wasm affine resolve scale rotation"
mfx_assert_file_contains "$affine_scale_file" "\"runtime_scale_milli\":4000" "selfcheck wasm affine resolve scale runtime scale"
mfx_assert_file_contains "$affine_scale_file" "\"runtime_alpha_milli\":800" "selfcheck wasm affine resolve scale runtime alpha"
mfx_assert_file_contains "$affine_scale_file" "\"runtime_delay_ms\":9" "selfcheck wasm affine resolve scale runtime delay"
mfx_assert_file_contains "$affine_scale_file" "\"runtime_life_ms\":700" "selfcheck wasm affine resolve scale runtime life"
mfx_assert_file_contains "$affine_scale_file" "\"runtime_apply_tint\":true" "selfcheck wasm affine resolve scale runtime tint"

affine_rotate_file="$tmp_dir/wasm-affine-rotate.out"
code_affine_rotate="$(mfx_wasm_selfcheck_test_resolve_image_affine_http_code \
    "$affine_rotate_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    '{"x":100,"y":200,"scale":1.0,"rotation":0.0,"alpha":-1.0,"life_ms":120,"delay_ms":22,"affine_enabled":true,"affine_dx":0,"affine_dy":0,"affine_m11":0.0,"affine_m12":-1.0,"affine_m21":1.0,"affine_m22":0.0}')"
mfx_assert_eq "$code_affine_rotate" "200" "selfcheck wasm affine resolve rotate status"
mfx_assert_file_contains "$affine_rotate_file" "\"ok\":true" "selfcheck wasm affine resolve rotate ok"
mfx_assert_file_contains "$affine_rotate_file" "\"resolved_scale_milli\":1000" "selfcheck wasm affine resolve rotate scale"
mfx_assert_file_contains "$affine_rotate_file" "\"resolved_rotation_millirad\":1571" "selfcheck wasm affine resolve rotate rotation"
mfx_assert_file_contains "$affine_rotate_file" "\"runtime_alpha_milli\":1000" "selfcheck wasm affine resolve rotate runtime alpha fallback"
mfx_assert_file_contains "$affine_rotate_file" "\"runtime_delay_ms\":22" "selfcheck wasm affine resolve rotate runtime delay"
mfx_assert_file_contains "$affine_rotate_file" "\"runtime_life_ms\":120" "selfcheck wasm affine resolve rotate runtime life"

affine_unsigned_max_file="$tmp_dir/wasm-affine-unsigned-max.out"
code_affine_unsigned_max="$(mfx_wasm_selfcheck_test_resolve_image_affine_http_code \
    "$affine_unsigned_max_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    '{"tint_rgba":4294967295,"delay_ms":4294967295,"life_ms":4294967295,"image_id":4294967295,"affine_anchor_mode":4294967295}')"
mfx_assert_eq "$code_affine_unsigned_max" "200" "selfcheck wasm affine resolve unsigned-max status"
mfx_assert_file_contains "$affine_unsigned_max_file" "\"ok\":true" "selfcheck wasm affine resolve unsigned-max ok"
mfx_assert_file_contains "$affine_unsigned_max_file" "\"resolved_tint_rgba_hex\":\"0xFFFFFFFF\"" "selfcheck wasm affine resolve unsigned-max tint"
mfx_assert_file_contains "$affine_unsigned_max_file" "\"resolved_delay_ms\":4294967295" "selfcheck wasm affine resolve unsigned-max delay"
mfx_assert_file_contains "$affine_unsigned_max_file" "\"resolved_life_ms\":4294967295" "selfcheck wasm affine resolve unsigned-max life"
mfx_assert_file_contains "$affine_unsigned_max_file" "\"resolved_image_id\":4294967295" "selfcheck wasm affine resolve unsigned-max image id"
mfx_assert_file_contains "$affine_unsigned_max_file" "\"resolved_affine_anchor_mode\":4294967295" "selfcheck wasm affine resolve unsigned-max anchor"
mfx_assert_file_contains "$affine_unsigned_max_file" "\"runtime_delay_ms\":60000" "selfcheck wasm affine resolve unsigned-max runtime delay clamp"
mfx_assert_file_contains "$affine_unsigned_max_file" "\"runtime_life_ms\":10000" "selfcheck wasm affine resolve unsigned-max runtime life clamp"
mfx_assert_file_contains "$affine_unsigned_max_file" "\"runtime_apply_tint\":true" "selfcheck wasm affine resolve unsigned-max runtime tint"

affine_unsigned_negative_file="$tmp_dir/wasm-affine-unsigned-negative.out"
code_affine_unsigned_negative="$(mfx_wasm_selfcheck_test_resolve_image_affine_http_code \
    "$affine_unsigned_negative_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    '{"tint_rgba":-1,"delay_ms":-1,"life_ms":-1,"image_id":-1,"affine_anchor_mode":-1}')"
mfx_assert_eq "$code_affine_unsigned_negative" "200" "selfcheck wasm affine resolve unsigned-negative status"
mfx_assert_file_contains "$affine_unsigned_negative_file" "\"ok\":true" "selfcheck wasm affine resolve unsigned-negative ok"
mfx_assert_file_contains "$affine_unsigned_negative_file" "\"resolved_tint_rgba_hex\":\"0x00000000\"" "selfcheck wasm affine resolve unsigned-negative tint"
mfx_assert_file_contains "$affine_unsigned_negative_file" "\"resolved_delay_ms\":0" "selfcheck wasm affine resolve unsigned-negative delay"
mfx_assert_file_contains "$affine_unsigned_negative_file" "\"resolved_life_ms\":0" "selfcheck wasm affine resolve unsigned-negative life"
mfx_assert_file_contains "$affine_unsigned_negative_file" "\"resolved_image_id\":0" "selfcheck wasm affine resolve unsigned-negative image id"
mfx_assert_file_contains "$affine_unsigned_negative_file" "\"resolved_affine_anchor_mode\":0" "selfcheck wasm affine resolve unsigned-negative anchor"
mfx_assert_file_contains "$affine_unsigned_negative_file" "\"runtime_delay_ms\":0" "selfcheck wasm affine resolve unsigned-negative runtime delay"
mfx_assert_file_contains "$affine_unsigned_negative_file" "\"runtime_apply_tint\":false" "selfcheck wasm affine resolve unsigned-negative runtime tint"

text_cfg_motion_file="$tmp_dir/wasm-text-config-motion.out"
code_text_cfg_motion="$(mfx_wasm_selfcheck_test_resolve_text_config_http_code \
    "$text_cfg_motion_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    '{"base_duration_ms":333,"base_float_distance_px":40,"base_font_size_px":20,"life_ms":1200,"vy":-300,"ay":100,"scale":1.0,"color_rgba":4294901760}')"
mfx_assert_eq "$code_text_cfg_motion" "200" "selfcheck wasm text config motion status"
mfx_assert_file_contains "$text_cfg_motion_file" "\"ok\":true" "selfcheck wasm text config motion ok"
mfx_assert_file_contains "$text_cfg_motion_file" "\"resolved_duration_ms\":1200" "selfcheck wasm text config motion duration"
mfx_assert_file_contains "$text_cfg_motion_file" "\"resolved_float_distance_px\":288" "selfcheck wasm text config motion float distance"
mfx_assert_file_contains "$text_cfg_motion_file" "\"resolved_font_size_px_milli\":20000" "selfcheck wasm text config motion font size"
mfx_assert_file_contains "$text_cfg_motion_file" "\"resolved_color_rgba_hex\":\"0xFFFF0000\"" "selfcheck wasm text config motion color"

text_cfg_clamp_file="$tmp_dir/wasm-text-config-clamp.out"
code_text_cfg_clamp="$(mfx_wasm_selfcheck_test_resolve_text_config_http_code \
    "$text_cfg_clamp_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    '{"base_duration_ms":250,"base_float_distance_px":20,"base_font_size_px":18,"life_ms":1,"vy":0,"ay":0,"scale":100}')"
mfx_assert_eq "$code_text_cfg_clamp" "200" "selfcheck wasm text config clamp status"
mfx_assert_file_contains "$text_cfg_clamp_file" "\"ok\":true" "selfcheck wasm text config clamp ok"
mfx_assert_file_contains "$text_cfg_clamp_file" "\"resolved_duration_ms\":80" "selfcheck wasm text config clamp duration"
mfx_assert_file_contains "$text_cfg_clamp_file" "\"resolved_float_distance_px\":16" "selfcheck wasm text config clamp float distance"
mfx_assert_file_contains "$text_cfg_clamp_file" "\"resolved_font_size_px_milli\":90000" "selfcheck wasm text config clamp font size"

text_cfg_negative_scale_file="$tmp_dir/wasm-text-config-negative-scale.out"
code_text_cfg_negative_scale="$(mfx_wasm_selfcheck_test_resolve_text_config_http_code \
    "$text_cfg_negative_scale_file" \
    "$MFX_MANUAL_BASE_URL" \
    "$token" \
    '{"base_duration_ms":500,"base_float_distance_px":32,"base_font_size_px":24,"life_ms":500,"scale":-1}')"
mfx_assert_eq "$code_text_cfg_negative_scale" "200" "selfcheck wasm text config negative-scale status"
mfx_assert_file_contains "$text_cfg_negative_scale_file" "\"ok\":true" "selfcheck wasm text config negative-scale ok"
mfx_assert_file_contains "$text_cfg_negative_scale_file" "\"resolved_font_size_px_milli\":24000" "selfcheck wasm text config negative-scale font unchanged"

invalid_manifest_path="${manifest_path}.missing"
invalid_file="$tmp_dir/wasm-load-invalid.out"
mfx_wasm_selfcheck_assert_load_manifest_failure \
    "invalid manifest" "$invalid_file" "$MFX_MANUAL_BASE_URL" "$token" "$invalid_manifest_path" \
    "manifest_load" "manifest_io_error"

required_manifest_file="$tmp_dir/wasm-load-required.out"
mfx_wasm_selfcheck_assert_load_manifest_required_failure \
    "manifest required" "$required_manifest_file" "$MFX_MANUAL_BASE_URL" "$token"

required_manifest_blank_file="$tmp_dir/wasm-load-required-blank.out"
mfx_wasm_selfcheck_assert_load_manifest_blank_path_required_failure \
    "manifest blank required" "$required_manifest_blank_file" "$MFX_MANUAL_BASE_URL" "$token"

invalid_json_manifest_path="$tmp_dir/manifest-invalid-json.json"
printf '{' > "$invalid_json_manifest_path"
invalid_json_file="$tmp_dir/wasm-load-invalid-json.out"
mfx_wasm_selfcheck_assert_load_manifest_failure \
    "invalid json manifest" "$invalid_json_file" "$MFX_MANUAL_BASE_URL" "$token" "$invalid_json_manifest_path" \
    "manifest_load" "manifest_json_parse_error"

invalid_schema_manifest_path="$tmp_dir/manifest-invalid-schema.json"
cat > "$invalid_schema_manifest_path" <<'EOF'
{
  "id": "",
  "name": "invalid-schema",
  "version": "1.0.0",
  "api_version": 1,
  "entry": "effect.wasm"
}
EOF
invalid_schema_file="$tmp_dir/wasm-load-invalid-schema.out"
mfx_wasm_selfcheck_assert_load_manifest_failure \
    "invalid schema manifest" "$invalid_schema_file" "$MFX_MANUAL_BASE_URL" "$token" "$invalid_schema_manifest_path" \
    "manifest_load" "manifest_invalid"

unsupported_api_manifest_path="$tmp_dir/manifest-unsupported-api.json"
cat > "$unsupported_api_manifest_path" <<'EOF'
{
  "id": "unsupported-api-plugin",
  "name": "unsupported-api-plugin",
  "version": "1.0.0",
  "api_version": 2,
  "entry": "effect.wasm"
}
EOF
unsupported_api_file="$tmp_dir/wasm-load-unsupported-api.out"
mfx_wasm_selfcheck_assert_load_manifest_failure \
    "unsupported api manifest" "$unsupported_api_file" "$MFX_MANUAL_BASE_URL" "$token" "$unsupported_api_manifest_path" \
    "manifest_api_version" "manifest_api_unsupported"

missing_wasm_manifest_path="$tmp_dir/manifest-missing-wasm.json"
cat > "$missing_wasm_manifest_path" <<'EOF'
{
  "id": "missing-wasm-plugin",
  "name": "missing-wasm-plugin",
  "version": "1.0.0",
  "api_version": 1,
  "entry": "missing.wasm"
}
EOF
missing_wasm_file="$tmp_dir/wasm-load-missing-wasm.out"
mfx_wasm_selfcheck_assert_load_manifest_failure \
    "missing wasm manifest" "$missing_wasm_file" "$MFX_MANUAL_BASE_URL" "$token" "$missing_wasm_manifest_path" \
    "load_module" "module_load_failed"

state_after_failure_file="$tmp_dir/state-after-failure-path.out"
code_state_after_failure="$(mfx_http_code "$state_after_failure_file" "$MFX_MANUAL_BASE_URL/api/state" -H "$token_header")"
mfx_assert_eq "$code_state_after_failure" "200" "selfcheck state after wasm fallback path status"
mfx_assert_file_contains "$state_after_failure_file" "\"runtime_backend\":\"wasm3_static\"" "selfcheck wasm backend remains static after fallback path"
failure_stage_after_invalid="$(mfx_wasm_selfcheck_parse_string_field "$state_after_failure_file" "last_load_failure_stage")"
failure_code_after_invalid="$(mfx_wasm_selfcheck_parse_string_field "$state_after_failure_file" "last_load_failure_code")"
if [[ -z "$failure_stage_after_invalid" || -z "$failure_code_after_invalid" ]]; then
    mfx_fail "selfcheck wasm fallback diagnostics missing last_load_failure_stage/code in /api/state"
fi

reload_file="$tmp_dir/wasm-load-manifest-reload.out"
mfx_wasm_selfcheck_assert_load_manifest_ok \
    "wasm reload-manifest" "$reload_file" "$MFX_MANUAL_BASE_URL" "$token" "$manifest_path"
mfx_assert_file_contains "$reload_file" "\"last_load_failure_stage\":\"\"" "selfcheck wasm reload clears failure stage"
mfx_assert_file_contains "$reload_file" "\"last_load_failure_code\":\"\"" "selfcheck wasm reload clears failure code"

state_after_reload_file="$tmp_dir/state-after-reload.out"
code_state_after_reload="$(mfx_http_code "$state_after_reload_file" "$MFX_MANUAL_BASE_URL/api/state" -H "$token_header")"
mfx_assert_eq "$code_state_after_reload" "200" "selfcheck state after wasm reload status"
mfx_assert_eq \
    "$(mfx_wasm_selfcheck_parse_string_field "$state_after_reload_file" "last_load_failure_stage")" \
    "" \
    "selfcheck state wasm reload clears failure stage"
mfx_assert_eq \
    "$(mfx_wasm_selfcheck_parse_string_field "$state_after_reload_file" "last_load_failure_code")" \
    "" \
    "selfcheck state wasm reload clears failure code"

if ! kill -0 "$MFX_MANUAL_HOST_PID" 2>/dev/null; then
    tail -n 100 "$MFX_MANUAL_LOG_FILE" >&2 || true
    mfx_fail "host died after fallback-path check"
fi

state_after_file="$tmp_dir/state-after-invalid.out"
code_state_after="$(mfx_http_code "$state_after_file" "$MFX_MANUAL_BASE_URL/api/state" -H "$token_header")"
mfx_assert_eq "$code_state_after" "200" "selfcheck state after invalid manifest"

mfx_ok "macos wasm runtime selfcheck passed"

if [[ "$keep_running" -eq 1 ]]; then
    if [[ "$auto_stop_seconds" -gt 0 ]]; then
        mfx_manual_schedule_auto_stop "$MFX_MANUAL_HOST_PID" "$auto_stop_seconds"
        printf 'auto_stop_seconds=%s\n' "$auto_stop_seconds"
    fi
    mfx_manual_print_stop_command "$MFX_MANUAL_HOST_PID"
fi
