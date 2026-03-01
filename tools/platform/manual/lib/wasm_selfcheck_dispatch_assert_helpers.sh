#!/usr/bin/env bash

set -euo pipefail

mfx_wasm_selfcheck_assert_dispatch_diagnostics_consistent() {
    local label="$1"
    local dispatch_output_file="$2"
    local state_output_file="$3"
    local state_before_output_file="${4:-}"
    local platform="${5:-}"

    local dispatch_throttled
    local dispatch_throttled_by_capacity
    local dispatch_throttled_by_interval
    local dispatch_dropped
    local dispatch_executed_text
    local dispatch_executed_image
    local dispatch_render_error
    dispatch_throttled="$(mfx_wasm_selfcheck_parse_uint_field "$dispatch_output_file" "throttled_commands")"
    dispatch_throttled_by_capacity="$(mfx_wasm_selfcheck_parse_uint_field "$dispatch_output_file" "throttled_by_capacity_commands")"
    dispatch_throttled_by_interval="$(mfx_wasm_selfcheck_parse_uint_field "$dispatch_output_file" "throttled_by_interval_commands")"
    dispatch_dropped="$(mfx_wasm_selfcheck_parse_uint_field "$dispatch_output_file" "dropped_commands")"
    dispatch_executed_text="$(mfx_wasm_selfcheck_parse_uint_field "$dispatch_output_file" "executed_text_commands")"
    dispatch_executed_image="$(mfx_wasm_selfcheck_parse_uint_field "$dispatch_output_file" "executed_image_commands")"
    dispatch_render_error="$(mfx_wasm_selfcheck_parse_string_field "$dispatch_output_file" "render_error")"

    if [[ -z "$dispatch_throttled" || -z "$dispatch_throttled_by_capacity" || -z "$dispatch_throttled_by_interval" || -z "$dispatch_dropped" || -z "$dispatch_executed_text" || -z "$dispatch_executed_image" ]]; then
        mfx_fail "selfcheck $label dispatch diagnostics parse failed"
    fi

    local state_throttled
    local state_throttled_by_capacity
    local state_throttled_by_interval
    local state_dropped
    local state_render_error
    local state_lifetime_invoke_calls
    local state_lifetime_invoke_success_calls
    local state_lifetime_invoke_failed_calls
    local state_lifetime_render_dispatches
    local state_lifetime_rendered_by_wasm_dispatches
    local state_lifetime_executed_text
    local state_lifetime_executed_image
    local state_lifetime_throttled
    local state_lifetime_throttled_by_capacity
    local state_lifetime_throttled_by_interval
    local state_lifetime_dropped
    state_throttled="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "last_throttled_render_commands")"
    state_throttled_by_capacity="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "last_throttled_by_capacity_render_commands")"
    state_throttled_by_interval="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "last_throttled_by_interval_render_commands")"
    state_dropped="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "last_dropped_render_commands")"
    state_render_error="$(mfx_wasm_selfcheck_parse_string_field "$state_output_file" "last_render_error")"
    state_lifetime_invoke_calls="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "lifetime_invoke_calls")"
    state_lifetime_invoke_success_calls="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "lifetime_invoke_success_calls")"
    state_lifetime_invoke_failed_calls="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "lifetime_invoke_failed_calls")"
    state_lifetime_render_dispatches="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "lifetime_render_dispatches")"
    state_lifetime_rendered_by_wasm_dispatches="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "lifetime_rendered_by_wasm_dispatches")"
    state_lifetime_executed_text="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "lifetime_executed_text_commands")"
    state_lifetime_executed_image="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "lifetime_executed_image_commands")"
    state_lifetime_throttled="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "lifetime_throttled_render_commands")"
    state_lifetime_throttled_by_capacity="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "lifetime_throttled_by_capacity_render_commands")"
    state_lifetime_throttled_by_interval="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "lifetime_throttled_by_interval_render_commands")"
    state_lifetime_dropped="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "lifetime_dropped_render_commands")"

    if [[ -z "$state_throttled" || -z "$state_throttled_by_capacity" || -z "$state_throttled_by_interval" || -z "$state_dropped" || -z "$state_lifetime_invoke_calls" || -z "$state_lifetime_invoke_success_calls" || -z "$state_lifetime_invoke_failed_calls" || -z "$state_lifetime_render_dispatches" || -z "$state_lifetime_rendered_by_wasm_dispatches" || -z "$state_lifetime_executed_text" || -z "$state_lifetime_executed_image" || -z "$state_lifetime_throttled" || -z "$state_lifetime_throttled_by_capacity" || -z "$state_lifetime_throttled_by_interval" || -z "$state_lifetime_dropped" ]]; then
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

    local state_lifetime_sum=$((state_lifetime_throttled_by_capacity + state_lifetime_throttled_by_interval))
    if (( state_lifetime_sum != state_lifetime_throttled )); then
        mfx_fail "selfcheck $label state lifetime throttle counters mismatch: total=$state_lifetime_throttled sum=$state_lifetime_sum"
    fi

    local state_lifetime_invoke_total=$((state_lifetime_invoke_success_calls + state_lifetime_invoke_failed_calls))
    if (( state_lifetime_invoke_total != state_lifetime_invoke_calls )); then
        mfx_fail "selfcheck $label state lifetime invoke counters mismatch: calls=$state_lifetime_invoke_calls success+failed=$state_lifetime_invoke_total"
    fi

    if (( state_lifetime_rendered_by_wasm_dispatches > state_lifetime_render_dispatches )); then
        mfx_fail "selfcheck $label state lifetime rendered dispatch overflow: rendered=$state_lifetime_rendered_by_wasm_dispatches total=$state_lifetime_render_dispatches"
    fi

    if (( state_lifetime_invoke_success_calls < 1 || state_lifetime_invoke_calls < 1 )); then
        mfx_fail "selfcheck $label state lifetime invoke counters unexpectedly zero: calls=$state_lifetime_invoke_calls success=$state_lifetime_invoke_success_calls"
    fi

    if (( state_lifetime_executed_text < dispatch_executed_text )); then
        mfx_fail "selfcheck $label state lifetime text commands smaller than dispatch snapshot: lifetime=$state_lifetime_executed_text dispatch=$dispatch_executed_text"
    fi

    if (( state_lifetime_executed_image < dispatch_executed_image )); then
        mfx_fail "selfcheck $label state lifetime image commands smaller than dispatch snapshot: lifetime=$state_lifetime_executed_image dispatch=$dispatch_executed_image"
    fi

    if (( state_lifetime_throttled < dispatch_throttled )); then
        mfx_fail "selfcheck $label state lifetime throttled commands smaller than dispatch snapshot: lifetime=$state_lifetime_throttled dispatch=$dispatch_throttled"
    fi

    if (( state_lifetime_dropped < dispatch_dropped )); then
        mfx_fail "selfcheck $label state lifetime dropped commands smaller than dispatch snapshot: lifetime=$state_lifetime_dropped dispatch=$dispatch_dropped"
    fi

    mfx_assert_eq "$state_throttled" "$dispatch_throttled" "selfcheck $label throttled total"
    mfx_assert_eq "$state_throttled_by_capacity" "$dispatch_throttled_by_capacity" "selfcheck $label throttled by capacity"
    mfx_assert_eq "$state_throttled_by_interval" "$dispatch_throttled_by_interval" "selfcheck $label throttled by interval"
    mfx_assert_eq "$state_dropped" "$dispatch_dropped" "selfcheck $label dropped commands"
    mfx_assert_eq "$state_render_error" "$dispatch_render_error" "selfcheck $label render error"

    if [[ "$platform" == "macos" && -n "$state_before_output_file" && -f "$state_before_output_file" ]]; then
        local before_fallback_show_count
        local after_fallback_show_count
        before_fallback_show_count="$(mfx_wasm_selfcheck_parse_uint_field "$state_before_output_file" "fallback_show_count")"
        after_fallback_show_count="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "fallback_show_count")"
        if [[ -z "$before_fallback_show_count" || -z "$after_fallback_show_count" ]]; then
            mfx_fail "selfcheck $label macos text fallback diagnostics parse failed"
        fi
        if (( dispatch_executed_text > 0 && after_fallback_show_count <= before_fallback_show_count )); then
            mfx_fail "selfcheck $label macos text fallback counter did not increase: before=$before_fallback_show_count after=$after_fallback_show_count dispatched_text=$dispatch_executed_text"
        fi

        local before_image_overlay_requests
        local after_image_overlay_requests
        local before_image_overlay_apply_tint_requests
        local after_image_overlay_apply_tint_requests
        before_image_overlay_requests="$(mfx_wasm_selfcheck_parse_uint_field "$state_before_output_file" "mac_image_overlay_requests")"
        after_image_overlay_requests="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "mac_image_overlay_requests")"
        before_image_overlay_apply_tint_requests="$(mfx_wasm_selfcheck_parse_uint_field "$state_before_output_file" "mac_image_overlay_apply_tint_requests")"
        after_image_overlay_apply_tint_requests="$(mfx_wasm_selfcheck_parse_uint_field "$state_output_file" "mac_image_overlay_apply_tint_requests")"
        if [[ -z "$before_image_overlay_requests" || -z "$after_image_overlay_requests" || -z "$before_image_overlay_apply_tint_requests" || -z "$after_image_overlay_apply_tint_requests" ]]; then
            mfx_fail "selfcheck $label macos image overlay diagnostics parse failed"
        fi
        if (( dispatch_executed_image > 0 && after_image_overlay_requests <= before_image_overlay_requests )); then
            mfx_fail "selfcheck $label macos image overlay request counter did not increase: before=$before_image_overlay_requests after=$after_image_overlay_requests dispatched_image=$dispatch_executed_image"
        fi
        if (( after_image_overlay_apply_tint_requests > after_image_overlay_requests )); then
            mfx_fail "selfcheck $label macos image overlay tint counter invalid: tint=$after_image_overlay_apply_tint_requests total=$after_image_overlay_requests"
        fi
    fi
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
