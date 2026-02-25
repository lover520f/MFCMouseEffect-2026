#!/usr/bin/env bash

set -euo pipefail

mfx_wasm_selfcheck_json_escape() {
    local value="$1"
    printf '%s' "$value" | sed 's/\\/\\\\/g; s/"/\\"/g'
}

mfx_wasm_selfcheck_load_manifest_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"

    local manifest_path_escaped
    manifest_path_escaped="$(mfx_wasm_selfcheck_json_escape "$manifest_path")"
    mfx_http_code "$output_file" "$base_url/api/wasm/load-manifest" \
        -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" \
        -d "{\"manifest_path\":\"$manifest_path_escaped\"}"
}

mfx_wasm_selfcheck_assert_load_manifest_ok() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"
    local manifest_path="$5"

    local code
    code="$(mfx_wasm_selfcheck_load_manifest_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "selfcheck $label ok"
}

mfx_wasm_selfcheck_assert_load_manifest_failure() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"
    local manifest_path="$5"
    local expected_stage="$6"
    local expected_code="$7"

    local code
    code="$(mfx_wasm_selfcheck_load_manifest_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":false" "selfcheck $label should fail"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_stage\":\"$expected_stage\"" \
        "selfcheck $label stage"
    mfx_assert_file_contains "$output_file" "\"last_load_failure_code\":\"$expected_code\"" \
        "selfcheck $label code"
}

mfx_wasm_selfcheck_import_selected_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"

    local manifest_path_escaped
    manifest_path_escaped="$(mfx_wasm_selfcheck_json_escape "$manifest_path")"
    mfx_http_code "$output_file" "$base_url/api/wasm/import-selected" \
        -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" \
        -d "{\"manifest_path\":\"$manifest_path_escaped\"}"
}

mfx_wasm_selfcheck_export_all_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"

    mfx_http_code "$output_file" "$base_url/api/wasm/export-all" \
        -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" \
        -d '{}'
}

mfx_wasm_selfcheck_import_dialog_probe_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"

    mfx_http_code "$output_file" "$base_url/api/wasm/import-from-folder-dialog" \
        -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" \
        -d '{"probe_only":true}'
}

mfx_wasm_selfcheck_test_dispatch_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"

    mfx_http_code "$output_file" "$base_url/api/wasm/test-dispatch-click" \
        -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" \
        -d '{"x":640,"y":360,"button":1}'
}

mfx_wasm_selfcheck_parse_uint_field() {
    local input_file="$1"
    local field_name="$2"
    sed -n "s/.*\"${field_name}\":\\([0-9][0-9]*\\).*/\\1/p" "$input_file" | head -n 1
}

mfx_wasm_selfcheck_parse_string_field() {
    local input_file="$1"
    local field_name="$2"
    sed -n "s/.*\"${field_name}\":\"\\([^\"]*\\)\".*/\\1/p" "$input_file" | head -n 1
}

mfx_wasm_selfcheck_assert_dispatch_diagnostics_consistent() {
    local label="$1"
    local dispatch_output_file="$2"
    local state_output_file="$3"

    local dispatch_throttled
    local dispatch_throttled_by_capacity
    local dispatch_throttled_by_interval
    local dispatch_dropped
    local dispatch_render_error
    dispatch_throttled="$(mfx_wasm_selfcheck_parse_uint_field "$dispatch_output_file" "throttled_commands")"
    dispatch_throttled_by_capacity="$(mfx_wasm_selfcheck_parse_uint_field "$dispatch_output_file" "throttled_by_capacity_commands")"
    dispatch_throttled_by_interval="$(mfx_wasm_selfcheck_parse_uint_field "$dispatch_output_file" "throttled_by_interval_commands")"
    dispatch_dropped="$(mfx_wasm_selfcheck_parse_uint_field "$dispatch_output_file" "dropped_commands")"
    dispatch_render_error="$(mfx_wasm_selfcheck_parse_string_field "$dispatch_output_file" "render_error")"

    if [[ -z "$dispatch_throttled" || -z "$dispatch_throttled_by_capacity" || -z "$dispatch_throttled_by_interval" || -z "$dispatch_dropped" ]]; then
        mfx_fail "selfcheck $label dispatch diagnostics parse failed"
    fi

    local state_throttled
    local state_throttled_by_capacity
    local state_throttled_by_interval
    local state_dropped
    local state_render_error
    state_throttled="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "last_throttled_render_commands")"
    state_throttled_by_capacity="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "last_throttled_by_capacity_render_commands")"
    state_throttled_by_interval="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "last_throttled_by_interval_render_commands")"
    state_dropped="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "last_dropped_render_commands")"
    state_render_error="$(mfx_wasm_selfcheck_parse_string_field "$state_output_file" "last_render_error")"

    if [[ -z "$state_throttled" || -z "$state_throttled_by_capacity" || -z "$state_throttled_by_interval" || -z "$state_dropped" ]]; then
        mfx_fail "selfcheck $label state diagnostics parse failed"
    fi

    local dispatch_sum=$((dispatch_throttled_by_capacity + dispatch_throttled_by_interval))
    if (( dispatch_sum != dispatch_throttled )); then
        mfx_fail "selfcheck $label dispatch throttle counters mismatch: total=$dispatch_throttled sum=$dispatch_sum"
    fi

    local state_sum=$((state_throttled_by_capacity + state_throttled_by_interval))
    if (( state_sum != state_throttled )); then
        mfx_fail "selfcheck $label state throttle counters mismatch: total=$state_throttled sum=$state_sum"
    fi

    mfx_assert_eq "$state_throttled" "$dispatch_throttled" "selfcheck $label throttled total"
    mfx_assert_eq "$state_throttled_by_capacity" "$dispatch_throttled_by_capacity" "selfcheck $label throttled by capacity"
    mfx_assert_eq "$state_throttled_by_interval" "$dispatch_throttled_by_interval" "selfcheck $label throttled by interval"
    mfx_assert_eq "$state_dropped" "$dispatch_dropped" "selfcheck $label dropped commands"
    mfx_assert_eq "$state_render_error" "$dispatch_render_error" "selfcheck $label render error"
}

mfx_wasm_selfcheck_assert_import_selected_ok() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"
    local manifest_path="$5"

    local code
    code="$(mfx_wasm_selfcheck_import_selected_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "selfcheck $label ok"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"\"" "selfcheck $label error code clear"
}

mfx_wasm_selfcheck_assert_import_selected_failure() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"
    local manifest_path="$5"
    local expected_error_code="$6"

    local code
    code="$(mfx_wasm_selfcheck_import_selected_http_code "$output_file" "$base_url" "$token" "$manifest_path")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":false" "selfcheck $label should fail"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"$expected_error_code\"" \
        "selfcheck $label error code"
}

mfx_wasm_selfcheck_assert_export_all_ok() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"
    local minimum_count="$5"

    local code
    code="$(mfx_wasm_selfcheck_export_all_http_code "$output_file" "$base_url" "$token")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "selfcheck $label ok"
    mfx_assert_file_contains "$output_file" "\"error_code\":\"\"" "selfcheck $label error code clear"

    local export_path
    export_path="$(sed -n 's/.*"export_path":"\([^"]*\)".*/\1/p' "$output_file" | head -n 1)"
    if [[ -z "$export_path" ]]; then
        mfx_fail "selfcheck $label export path parse failed"
    fi
    if [[ ! -d "$export_path" ]]; then
        mfx_fail "selfcheck $label export path not found: $export_path"
    fi

    local exported_count
    exported_count="$(sed -n 's/.*"count":\([0-9][0-9]*\).*/\1/p' "$output_file" | head -n 1)"
    if [[ -z "$exported_count" ]]; then
        mfx_fail "selfcheck $label exported count parse failed"
    fi
    if (( exported_count < minimum_count )); then
        mfx_fail "selfcheck $label exported count too small: expected >= $minimum_count got $exported_count"
    fi

    local exported_manifest_count
    exported_manifest_count="$(find "$export_path" -mindepth 2 -maxdepth 2 -type f -name 'plugin.json' | wc -l | tr -d ' ')"
    if [[ "$exported_manifest_count" != "$exported_count" ]]; then
        mfx_fail "selfcheck $label exported manifest count mismatch: response=$exported_count manifests=$exported_manifest_count"
    fi
}

mfx_wasm_selfcheck_assert_import_dialog_probe_supported() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"

    local code
    code="$(mfx_wasm_selfcheck_import_dialog_probe_http_code "$output_file" "$base_url" "$token")"
    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "selfcheck $label ok"
    mfx_assert_file_contains "$output_file" "\"probe_only\":true" "selfcheck $label probe flag"
    mfx_assert_file_contains "$output_file" "\"supported\":true" "selfcheck $label supported"
}

mfx_wasm_selfcheck_assert_test_dispatch_ok() {
    local label="$1"
    local output_file="$2"
    local base_url="$3"
    local token="$4"
    local require_rendered_any="${5:-true}"
    local timeout_seconds="${MFX_MANUAL_WASM_DISPATCH_TIMEOUT_SECONDS:-5}"
    local retry_interval_seconds="${MFX_MANUAL_WASM_DISPATCH_RETRY_INTERVAL_SECONDS:-0.2}"
    local deadline=$((SECONDS + timeout_seconds))
    local code=""

    while true; do
        code="$(mfx_wasm_selfcheck_test_dispatch_http_code "$output_file" "$base_url" "$token")"
        if [[ "$code" == "200" ]] && \
           mfx_file_contains_fixed "$output_file" "\"ok\":true" && \
           mfx_file_contains_fixed "$output_file" "\"route_active\":true" && \
           mfx_file_contains_fixed "$output_file" "\"invoke_ok\":true"; then
            if [[ "$require_rendered_any" != "true" ]] || \
               mfx_file_contains_fixed "$output_file" "\"rendered_any\":true"; then
                return 0
            fi
        fi

        if (( SECONDS >= deadline )); then
            break
        fi
        sleep "$retry_interval_seconds"
    done

    mfx_assert_eq "$code" "200" "selfcheck $label status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "selfcheck $label ok"
    mfx_assert_file_contains "$output_file" "\"route_active\":true" "selfcheck $label route active"
    mfx_assert_file_contains "$output_file" "\"invoke_ok\":true" "selfcheck $label invoke ok"
    if [[ "$require_rendered_any" == "true" ]]; then
        mfx_assert_file_contains "$output_file" "\"rendered_any\":true" "selfcheck $label rendered"
    fi
}
