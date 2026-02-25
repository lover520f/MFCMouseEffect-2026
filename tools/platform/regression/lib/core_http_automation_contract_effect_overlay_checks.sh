#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_contract_effect_overlay_checks() {
    local platform="$1"
    local tmp_dir="$2"
    local base_url="$3"
    local token="$4"

    local code_effect_overlay_probe
    code_effect_overlay_probe="$(mfx_http_code "$tmp_dir/effect-overlay-probe.out" "$base_url/api/effects/test-overlay-windows" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"emit_click":true,"emit_scroll":true,"wait_ms":80,"wait_for_clear_ms":1600}')"
    mfx_assert_eq "$code_effect_overlay_probe" "200" "core effect overlay probe status"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"ok\":true" "core effect overlay probe ok"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"before\":" "core effect overlay probe before snapshot"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"after\":" "core effect overlay probe after snapshot"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"before_total_matches_components\":true" "core effect overlay probe before invariant"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"after_total_matches_components\":true" "core effect overlay probe after invariant"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"restored_to_baseline\":true" "core effect overlay probe restore baseline"

    local before_click_count
    local before_scroll_count
    local before_total_count
    local after_click_count
    local after_scroll_count
    local after_total_count
    before_click_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "before_click_active_overlay_windows")"
    before_scroll_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "before_scroll_active_overlay_windows")"
    before_total_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "before_active_overlay_windows_total")"
    after_click_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "after_click_active_overlay_windows")"
    after_scroll_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "after_scroll_active_overlay_windows")"
    after_total_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "after_active_overlay_windows_total")"

    if [[ -z "$before_click_count" || -z "$before_scroll_count" || -z "$before_total_count" || -z "$after_click_count" || -z "$after_scroll_count" || -z "$after_total_count" ]]; then
        mfx_fail "core effect overlay probe count parse failed"
    fi

    local before_sum=$((before_click_count + before_scroll_count))
    local after_sum=$((after_click_count + after_scroll_count))
    if (( before_sum != before_total_count )); then
        mfx_fail "core effect overlay probe before count mismatch: total=$before_total_count sum=$before_sum"
    fi
    if (( after_sum != after_total_count )); then
        mfx_fail "core effect overlay probe after count mismatch: total=$after_total_count sum=$after_sum"
    fi

    if [[ "$platform" == "macos" ]]; then
        if ! mfx_file_contains_fixed "$tmp_dir/effect-overlay-probe.out" "\"supported\":true"; then
            mfx_fail "core effect overlay probe support on macos: expected supported=true"
        fi
    fi
}
