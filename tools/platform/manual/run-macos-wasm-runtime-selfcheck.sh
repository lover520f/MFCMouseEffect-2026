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

if [[ -n "$build_jobs" ]]; then
    if ! [[ "$build_jobs" =~ ^[0-9]+$ ]]; then
        mfx_fail "--jobs must be a positive integer"
    fi
    export MFX_BUILD_JOBS="$build_jobs"
fi

if ! [[ "$auto_stop_seconds" =~ ^[0-9]+$ ]]; then
    mfx_fail "--auto-stop-seconds must be a non-negative integer"
fi

mfx_require_cmd cmake
mfx_require_cmd curl
mfx_require_cmd sed

if [[ "$skip_build" -eq 0 ]]; then
    mfx_configure_platform_build_dir "$repo_root" "$build_dir" "macos" \
        -DMFX_ENABLE_CROSS_HOST_PACKAGES=ON \
        -DMFX_ENABLE_ENTRY_RUNTIME_TARGETS=ON \
        -DMFX_ENABLE_POSIX_CORE_RUNTIME=ON
    mfx_build_targets "$build_dir" "mfx_entry_posix_host"
fi

host_bin="$build_dir/mfx_entry_posix_host"
if [[ ! -x "$host_bin" ]]; then
    mfx_fail "host binary missing or not executable: $host_bin"
fi

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

mfx_manual_start_core_host "$host_bin" "$probe_file" "$log_file" "MFX_ENABLE_WASM_TEST_DISPATCH_API=1"

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
    "$state_after_dispatch_file"

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

reload_file="$tmp_dir/wasm-load-manifest-reload.out"
mfx_wasm_selfcheck_assert_load_manifest_ok \
    "wasm reload-manifest" "$reload_file" "$MFX_MANUAL_BASE_URL" "$token" "$manifest_path"
mfx_assert_file_contains "$reload_file" "\"last_load_failure_stage\":\"\"" "selfcheck wasm reload clears failure stage"
mfx_assert_file_contains "$reload_file" "\"last_load_failure_code\":\"\"" "selfcheck wasm reload clears failure code"

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
