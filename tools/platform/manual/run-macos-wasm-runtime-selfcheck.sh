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
EOF
}

json_escape() {
    local value="$1"
    printf '%s' "$value" | sed 's/\\/\\\\/g; s/"/\\"/g'
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
    if [[ "$keep_running" -eq 0 ]]; then
        mfx_manual_stop_core_host "$MFX_MANUAL_HOST_PID"
    fi
}
trap cleanup EXIT

mfx_manual_start_core_host "$host_bin" "$probe_file" "$log_file" "MFX_ENABLE_WASM_TEST_DISPATCH_API=1"

printf 'mfx_pid=%s\n' "$MFX_MANUAL_HOST_PID"
printf 'settings_url=%s\n' "$MFX_MANUAL_SETTINGS_URL"
printf 'manifest_path=%s\n' "$manifest_path"
printf 'log_file=%s\n' "$MFX_MANUAL_LOG_FILE"

if [[ "$open_settings" -eq 1 ]]; then
    mfx_require_cmd open
    open "$MFX_MANUAL_SETTINGS_URL" >/dev/null 2>&1 || mfx_fail "failed to open settings url"
fi

token_header=("x-mfcmouseeffect-token: $MFX_MANUAL_SETTINGS_TOKEN")

state_file="$tmp_dir/state.out"
code_state="$(mfx_http_code "$state_file" "$MFX_MANUAL_BASE_URL/api/state" -H "${token_header[0]}")"
mfx_assert_eq "$code_state" "200" "selfcheck state status"
mfx_assert_file_contains "$state_file" "\"runtime_backend\":\"wasm3_static\"" "selfcheck runtime backend"
mfx_assert_file_contains "$state_file" "\"render_supported\":true" "selfcheck render capability"

manifest_path_escaped="$(json_escape "$manifest_path")"

load_file="$tmp_dir/wasm-load-manifest.out"
code_load="$(mfx_http_code "$load_file" "$MFX_MANUAL_BASE_URL/api/wasm/load-manifest" \
    -X POST -H "${token_header[0]}" -H "Content-Type: application/json" \
    -d "{\"manifest_path\":\"$manifest_path_escaped\"}")"
mfx_assert_eq "$code_load" "200" "selfcheck wasm load-manifest status"
mfx_assert_file_contains "$load_file" "\"ok\":true" "selfcheck wasm load-manifest ok"

enable_file="$tmp_dir/wasm-enable.out"
code_enable="$(mfx_http_code "$enable_file" "$MFX_MANUAL_BASE_URL/api/wasm/enable" \
    -X POST -H "${token_header[0]}" -H "Content-Type: application/json" -d '{}')"
mfx_assert_eq "$code_enable" "200" "selfcheck wasm enable status"
mfx_assert_file_contains "$enable_file" "\"ok\":true" "selfcheck wasm enable ok"

dispatch_file="$tmp_dir/wasm-test-dispatch.out"
code_dispatch="$(mfx_http_code "$dispatch_file" "$MFX_MANUAL_BASE_URL/api/wasm/test-dispatch-click" \
    -X POST -H "${token_header[0]}" -H "Content-Type: application/json" \
    -d '{"x":640,"y":360,"button":1}')"
mfx_assert_eq "$code_dispatch" "200" "selfcheck wasm test-dispatch status"
mfx_assert_file_contains "$dispatch_file" "\"ok\":true" "selfcheck wasm test-dispatch ok"
mfx_assert_file_contains "$dispatch_file" "\"route_active\":true" "selfcheck wasm route active"
mfx_assert_file_contains "$dispatch_file" "\"invoke_ok\":true" "selfcheck wasm invoke ok"
mfx_assert_file_contains "$dispatch_file" "\"rendered_any\":true" "selfcheck wasm rendered"

invalid_manifest_path="${manifest_path}.missing"
invalid_manifest_escaped="$(json_escape "$invalid_manifest_path")"
invalid_file="$tmp_dir/wasm-load-invalid.out"
code_invalid="$(mfx_http_code "$invalid_file" "$MFX_MANUAL_BASE_URL/api/wasm/load-manifest" \
    -X POST -H "${token_header[0]}" -H "Content-Type: application/json" \
    -d "{\"manifest_path\":\"$invalid_manifest_escaped\"}")"
mfx_assert_eq "$code_invalid" "200" "selfcheck invalid manifest status"
mfx_assert_file_contains "$invalid_file" "\"ok\":false" "selfcheck invalid manifest should fail"
mfx_assert_file_contains "$invalid_file" "\"last_load_failure_stage\":\"manifest_load\"" "selfcheck invalid manifest stage"
mfx_assert_file_contains "$invalid_file" "\"last_load_failure_code\":\"manifest_io_error\"" "selfcheck invalid manifest code"

invalid_json_manifest_path="$tmp_dir/manifest-invalid-json.json"
printf '{' > "$invalid_json_manifest_path"
invalid_json_manifest_escaped="$(json_escape "$invalid_json_manifest_path")"
invalid_json_file="$tmp_dir/wasm-load-invalid-json.out"
code_invalid_json="$(mfx_http_code "$invalid_json_file" "$MFX_MANUAL_BASE_URL/api/wasm/load-manifest" \
    -X POST -H "${token_header[0]}" -H "Content-Type: application/json" \
    -d "{\"manifest_path\":\"$invalid_json_manifest_escaped\"}")"
mfx_assert_eq "$code_invalid_json" "200" "selfcheck invalid json manifest status"
mfx_assert_file_contains "$invalid_json_file" "\"ok\":false" "selfcheck invalid json manifest should fail"
mfx_assert_file_contains "$invalid_json_file" "\"last_load_failure_stage\":\"manifest_load\"" "selfcheck invalid json manifest stage"
mfx_assert_file_contains "$invalid_json_file" "\"last_load_failure_code\":\"manifest_json_parse_error\"" "selfcheck invalid json manifest code"

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
invalid_schema_manifest_escaped="$(json_escape "$invalid_schema_manifest_path")"
invalid_schema_file="$tmp_dir/wasm-load-invalid-schema.out"
code_invalid_schema="$(mfx_http_code "$invalid_schema_file" "$MFX_MANUAL_BASE_URL/api/wasm/load-manifest" \
    -X POST -H "${token_header[0]}" -H "Content-Type: application/json" \
    -d "{\"manifest_path\":\"$invalid_schema_manifest_escaped\"}")"
mfx_assert_eq "$code_invalid_schema" "200" "selfcheck invalid schema manifest status"
mfx_assert_file_contains "$invalid_schema_file" "\"ok\":false" "selfcheck invalid schema manifest should fail"
mfx_assert_file_contains "$invalid_schema_file" "\"last_load_failure_stage\":\"manifest_load\"" "selfcheck invalid schema manifest stage"
mfx_assert_file_contains "$invalid_schema_file" "\"last_load_failure_code\":\"manifest_invalid\"" "selfcheck invalid schema manifest code"

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
unsupported_api_manifest_escaped="$(json_escape "$unsupported_api_manifest_path")"
unsupported_api_file="$tmp_dir/wasm-load-unsupported-api.out"
code_unsupported_api="$(mfx_http_code "$unsupported_api_file" "$MFX_MANUAL_BASE_URL/api/wasm/load-manifest" \
    -X POST -H "${token_header[0]}" -H "Content-Type: application/json" \
    -d "{\"manifest_path\":\"$unsupported_api_manifest_escaped\"}")"
mfx_assert_eq "$code_unsupported_api" "200" "selfcheck unsupported api manifest status"
mfx_assert_file_contains "$unsupported_api_file" "\"ok\":false" "selfcheck unsupported api manifest should fail"
mfx_assert_file_contains "$unsupported_api_file" "\"last_load_failure_stage\":\"manifest_api_version\"" "selfcheck unsupported api manifest stage"
mfx_assert_file_contains "$unsupported_api_file" "\"last_load_failure_code\":\"manifest_api_unsupported\"" "selfcheck unsupported api manifest code"

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
missing_wasm_manifest_escaped="$(json_escape "$missing_wasm_manifest_path")"
missing_wasm_file="$tmp_dir/wasm-load-missing-wasm.out"
code_missing_wasm="$(mfx_http_code "$missing_wasm_file" "$MFX_MANUAL_BASE_URL/api/wasm/load-manifest" \
    -X POST -H "${token_header[0]}" -H "Content-Type: application/json" \
    -d "{\"manifest_path\":\"$missing_wasm_manifest_escaped\"}")"
mfx_assert_eq "$code_missing_wasm" "200" "selfcheck missing wasm manifest status"
mfx_assert_file_contains "$missing_wasm_file" "\"ok\":false" "selfcheck missing wasm manifest should fail"
mfx_assert_file_contains "$missing_wasm_file" "\"last_load_failure_stage\":\"load_module\"" "selfcheck missing wasm manifest stage"
mfx_assert_file_contains "$missing_wasm_file" "\"last_load_failure_code\":\"module_load_failed\"" "selfcheck missing wasm manifest code"

reload_file="$tmp_dir/wasm-load-manifest-reload.out"
code_reload="$(mfx_http_code "$reload_file" "$MFX_MANUAL_BASE_URL/api/wasm/load-manifest" \
    -X POST -H "${token_header[0]}" -H "Content-Type: application/json" \
    -d "{\"manifest_path\":\"$manifest_path_escaped\"}")"
mfx_assert_eq "$code_reload" "200" "selfcheck wasm reload-manifest status"
mfx_assert_file_contains "$reload_file" "\"ok\":true" "selfcheck wasm reload-manifest ok"
mfx_assert_file_contains "$reload_file" "\"last_load_failure_stage\":\"\"" "selfcheck wasm reload clears failure stage"
mfx_assert_file_contains "$reload_file" "\"last_load_failure_code\":\"\"" "selfcheck wasm reload clears failure code"

if ! kill -0 "$MFX_MANUAL_HOST_PID" 2>/dev/null; then
    tail -n 100 "$MFX_MANUAL_LOG_FILE" >&2 || true
    mfx_fail "host died after fallback-path check"
fi

state_after_file="$tmp_dir/state-after-invalid.out"
code_state_after="$(mfx_http_code "$state_after_file" "$MFX_MANUAL_BASE_URL/api/state" -H "${token_header[0]}")"
mfx_assert_eq "$code_state_after" "200" "selfcheck state after invalid manifest"

mfx_ok "macos wasm runtime selfcheck passed"

if [[ "$keep_running" -eq 1 ]]; then
    if [[ "$auto_stop_seconds" -gt 0 ]]; then
        mfx_manual_schedule_auto_stop "$MFX_MANUAL_HOST_PID" "$auto_stop_seconds"
        printf 'auto_stop_seconds=%s\n' "$auto_stop_seconds"
    fi
    printf 'stop_cmd=pkill -f mfx_entry_posix_host\n'
fi
