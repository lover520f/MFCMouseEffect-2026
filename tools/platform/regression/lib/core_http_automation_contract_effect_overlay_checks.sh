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
        -d '{"emit_click":true,"emit_trail":true,"emit_scroll":true,"emit_hold":true,"emit_hover":true,"click_type":"text","trail_type":"electric","scroll_type":"helix","hold_type":"hold_quantum_halo_gpu_v2","hover_type":"tubes","close_persistent":true,"wait_ms":80,"wait_for_clear_ms":1600}')"
    mfx_assert_eq "$code_effect_overlay_probe" "200" "core effect overlay probe status"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"ok\":true" "core effect overlay probe ok"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"before\":" "core effect overlay probe before snapshot"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"after\":" "core effect overlay probe after snapshot"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"before_total_matches_components\":true" "core effect overlay probe before invariant"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"after_total_matches_components\":true" "core effect overlay probe after invariant"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"restored_to_baseline\":true" "core effect overlay probe restore baseline"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"click_type\":\"text\"" "core effect overlay probe click type"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"trail_type\":\"electric\"" "core effect overlay probe trail type"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"scroll_type\":\"helix\"" "core effect overlay probe scroll type"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"hold_type\":\"hold_quantum_halo_gpu_v2\"" "core effect overlay probe hold type"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"hover_type\":\"tubes\"" "core effect overlay probe hover type"

    local before_click_count
    local before_trail_count
    local before_scroll_count
    local before_hold_count
    local before_hover_count
    local before_total_count
    local after_click_count
    local after_trail_count
    local after_scroll_count
    local after_hold_count
    local after_hover_count
    local after_total_count
    before_click_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "before_click_active_overlay_windows")"
    before_trail_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "before_trail_active_overlay_windows")"
    before_scroll_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "before_scroll_active_overlay_windows")"
    before_hold_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "before_hold_active_overlay_windows")"
    before_hover_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "before_hover_active_overlay_windows")"
    before_total_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "before_active_overlay_windows_total")"
    after_click_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "after_click_active_overlay_windows")"
    after_trail_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "after_trail_active_overlay_windows")"
    after_scroll_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "after_scroll_active_overlay_windows")"
    after_hold_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "after_hold_active_overlay_windows")"
    after_hover_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "after_hover_active_overlay_windows")"
    after_total_count="$(_mfx_core_http_automation_parse_uint_field "$tmp_dir/effect-overlay-probe.out" "after_active_overlay_windows_total")"

    if [[ -z "$before_click_count" || -z "$before_trail_count" || -z "$before_scroll_count" || -z "$before_hold_count" || -z "$before_hover_count" || -z "$before_total_count" || -z "$after_click_count" || -z "$after_trail_count" || -z "$after_scroll_count" || -z "$after_hold_count" || -z "$after_hover_count" || -z "$after_total_count" ]]; then
        mfx_fail "core effect overlay probe count parse failed"
    fi

    local before_sum=$((before_click_count + before_trail_count + before_scroll_count + before_hold_count + before_hover_count))
    local after_sum=$((after_click_count + after_trail_count + after_scroll_count + after_hold_count + after_hover_count))
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
