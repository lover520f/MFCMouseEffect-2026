#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_assert_wasm_dispatch_diagnostics_consistent() {
    local dispatch_output_file="$1"
    local state_output_file="$2"
    local context="$3"

    local dispatch_throttled
    local dispatch_throttled_by_capacity
    local dispatch_throttled_by_interval
    local dispatch_dropped
    local dispatch_render_error
    dispatch_throttled="$(_mfx_core_http_wasm_parse_uint_field "$dispatch_output_file" "throttled_commands")"
    dispatch_throttled_by_capacity="$(_mfx_core_http_wasm_parse_uint_field "$dispatch_output_file" "throttled_by_capacity_commands")"
    dispatch_throttled_by_interval="$(_mfx_core_http_wasm_parse_uint_field "$dispatch_output_file" "throttled_by_interval_commands")"
    dispatch_dropped="$(_mfx_core_http_wasm_parse_uint_field "$dispatch_output_file" "dropped_commands")"
    dispatch_render_error="$(_mfx_core_http_wasm_parse_string_field "$dispatch_output_file" "render_error")"

    if [[ -z "$dispatch_throttled" || -z "$dispatch_throttled_by_capacity" || -z "$dispatch_throttled_by_interval" || -z "$dispatch_dropped" ]]; then
        mfx_fail "$context dispatch diagnostics parse failed"
    fi

    local state_throttled
    local state_throttled_by_capacity
    local state_throttled_by_interval
    local state_dropped
    local state_render_error
    state_throttled="$(_mfx_core_http_wasm_parse_uint_field "$state_output_file" "last_throttled_render_commands")"
    state_throttled_by_capacity="$(_mfx_core_http_wasm_parse_uint_field "$state_output_file" "last_throttled_by_capacity_render_commands")"
    state_throttled_by_interval="$(_mfx_core_http_wasm_parse_uint_field "$state_output_file" "last_throttled_by_interval_render_commands")"
    state_dropped="$(_mfx_core_http_wasm_parse_uint_field "$state_output_file" "last_dropped_render_commands")"
    state_render_error="$(_mfx_core_http_wasm_parse_string_field "$state_output_file" "last_render_error")"

    if [[ -z "$state_throttled" || -z "$state_throttled_by_capacity" || -z "$state_throttled_by_interval" || -z "$state_dropped" ]]; then
        mfx_fail "$context state diagnostics parse failed"
    fi

    local dispatch_sum=$((dispatch_throttled_by_capacity + dispatch_throttled_by_interval))
    if (( dispatch_sum != dispatch_throttled )); then
        mfx_fail "$context dispatch throttle counters mismatch: total=$dispatch_throttled sum=$dispatch_sum"
    fi

    local state_sum=$((state_throttled_by_capacity + state_throttled_by_interval))
    if (( state_sum != state_throttled )); then
        mfx_fail "$context state throttle counters mismatch: total=$state_throttled sum=$state_sum"
    fi

    mfx_assert_eq "$state_throttled" "$dispatch_throttled" "$context throttled total"
    mfx_assert_eq "$state_throttled_by_capacity" "$dispatch_throttled_by_capacity" "$context throttled by capacity"
    mfx_assert_eq "$state_throttled_by_interval" "$dispatch_throttled_by_interval" "$context throttled by interval"
    mfx_assert_eq "$state_dropped" "$dispatch_dropped" "$context dropped commands"
    mfx_assert_eq "$state_render_error" "$dispatch_render_error" "$context render error"
}

_mfx_core_http_assert_wasm_test_dispatch_ok() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local context="$4"
    local require_rendered_any="${5:-false}"
    local timeout_seconds="${MFX_CORE_HTTP_WASM_DISPATCH_TIMEOUT_SECONDS:-5}"
    local retry_interval_seconds="${MFX_CORE_HTTP_WASM_DISPATCH_RETRY_INTERVAL_SECONDS:-0.2}"
    local deadline=$((SECONDS + timeout_seconds))
    local code=""

    while true; do
        code="$(_mfx_core_http_wasm_test_dispatch_http_code "$output_file" "$base_url" "$token")"
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

    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"route_active\":true" "$context route_active"
    mfx_assert_file_contains "$output_file" "\"invoke_ok\":true" "$context invoke_ok"
    if [[ "$require_rendered_any" == "true" ]]; then
        mfx_assert_file_contains "$output_file" "\"rendered_any\":true" "$context rendered_any"
    fi
}
