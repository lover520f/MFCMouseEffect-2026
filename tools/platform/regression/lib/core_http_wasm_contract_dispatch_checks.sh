#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_wasm_contract_dispatch_checks() {
    local platform="$1"
    local tmp_dir="$2"
    local base_url="$3"
    local token="$4"

    local code_wasm_enable
    code_wasm_enable="$(mfx_http_code "$tmp_dir/wasm-enable.out" "$base_url/api/wasm/enable" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{}')"
    mfx_assert_eq "$code_wasm_enable" "200" "core wasm enable status"
    mfx_assert_file_contains "$tmp_dir/wasm-enable.out" "\"ok\":true" "core wasm enable ok"

    local code_state_before_dispatch
    code_state_before_dispatch="$(mfx_http_code "$tmp_dir/state-before-wasm-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_state_before_dispatch" "200" "core wasm state before dispatch status"

    local require_rendered_any="false"
    if [[ "$platform" == "macos" ]]; then
        require_rendered_any="true"
    fi
    _mfx_core_http_assert_wasm_test_dispatch_ok \
        "$tmp_dir/wasm-test-dispatch.out" \
        "$base_url" \
        "$token" \
        "core wasm test-dispatch" \
        "$require_rendered_any"

    local code_state_after_dispatch
    code_state_after_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_state_after_dispatch" "200" "core wasm state after dispatch status"
    _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
        "$tmp_dir/wasm-test-dispatch.out" \
        "$tmp_dir/state-after-wasm-dispatch.out" \
        "core wasm dispatch diagnostics consistency" \
        "$tmp_dir/state-before-wasm-dispatch.out" \
        "$platform"

    local code_affine_translate
    code_affine_translate="$(_mfx_core_http_wasm_test_resolve_image_affine_http_code \
        "$tmp_dir/wasm-affine-translate.out" \
        "$base_url" \
        "$token" \
        '{"x":100,"y":200,"scale":2.0,"rotation":0.5,"affine_enabled":false,"affine_dx":12,"affine_dy":-7}')"
    mfx_assert_eq "$code_affine_translate" "200" "core wasm affine resolve translate status"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"ok\":true" "core wasm affine resolve translate ok"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"resolved_x_int\":112" "core wasm affine resolve translate x"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"resolved_y_int\":193" "core wasm affine resolve translate y"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"resolved_scale_milli\":2000" "core wasm affine resolve translate scale"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"resolved_rotation_millirad\":500" "core wasm affine resolve translate rotation"

    local code_affine_scale
    code_affine_scale="$(_mfx_core_http_wasm_test_resolve_image_affine_http_code \
        "$tmp_dir/wasm-affine-scale.out" \
        "$base_url" \
        "$token" \
        '{"x":100,"y":200,"scale":2.0,"rotation":0.5,"affine_enabled":true,"affine_dx":12,"affine_dy":-7,"affine_m11":2.0,"affine_m12":0.0,"affine_m21":0.0,"affine_m22":2.0}')"
    mfx_assert_eq "$code_affine_scale" "200" "core wasm affine resolve scale status"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"ok\":true" "core wasm affine resolve scale ok"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"resolved_x_int\":112" "core wasm affine resolve scale x"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"resolved_y_int\":193" "core wasm affine resolve scale y"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"resolved_scale_milli\":4000" "core wasm affine resolve scale scale"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"resolved_rotation_millirad\":500" "core wasm affine resolve scale rotation"

    local code_affine_rotate
    code_affine_rotate="$(_mfx_core_http_wasm_test_resolve_image_affine_http_code \
        "$tmp_dir/wasm-affine-rotate.out" \
        "$base_url" \
        "$token" \
        '{"x":100,"y":200,"scale":1.0,"rotation":0.0,"affine_enabled":true,"affine_dx":0,"affine_dy":0,"affine_m11":0.0,"affine_m12":-1.0,"affine_m21":1.0,"affine_m22":0.0}')"
    mfx_assert_eq "$code_affine_rotate" "200" "core wasm affine resolve rotate status"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-rotate.out" "\"ok\":true" "core wasm affine resolve rotate ok"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-rotate.out" "\"resolved_scale_milli\":1000" "core wasm affine resolve rotate scale"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-rotate.out" "\"resolved_rotation_millirad\":1571" "core wasm affine resolve rotate rotation"

    local code_affine_unsigned_max
    code_affine_unsigned_max="$(_mfx_core_http_wasm_test_resolve_image_affine_http_code \
        "$tmp_dir/wasm-affine-unsigned-max.out" \
        "$base_url" \
        "$token" \
        '{"tint_rgba":4294967295,"delay_ms":4294967295,"life_ms":4294967295,"image_id":4294967295,"affine_anchor_mode":4294967295}')"
    mfx_assert_eq "$code_affine_unsigned_max" "200" "core wasm affine resolve unsigned-max status"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-max.out" "\"ok\":true" "core wasm affine resolve unsigned-max ok"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-max.out" "\"resolved_tint_rgba_hex\":\"0xFFFFFFFF\"" "core wasm affine resolve unsigned-max tint"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-max.out" "\"resolved_delay_ms\":4294967295" "core wasm affine resolve unsigned-max delay"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-max.out" "\"resolved_life_ms\":4294967295" "core wasm affine resolve unsigned-max life"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-max.out" "\"resolved_image_id\":4294967295" "core wasm affine resolve unsigned-max image id"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-max.out" "\"resolved_affine_anchor_mode\":4294967295" "core wasm affine resolve unsigned-max anchor"

    local code_affine_unsigned_negative
    code_affine_unsigned_negative="$(_mfx_core_http_wasm_test_resolve_image_affine_http_code \
        "$tmp_dir/wasm-affine-unsigned-negative.out" \
        "$base_url" \
        "$token" \
        '{"tint_rgba":-1,"delay_ms":-1,"life_ms":-1,"image_id":-1,"affine_anchor_mode":-1}')"
    mfx_assert_eq "$code_affine_unsigned_negative" "200" "core wasm affine resolve unsigned-negative status"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-negative.out" "\"ok\":true" "core wasm affine resolve unsigned-negative ok"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-negative.out" "\"resolved_tint_rgba_hex\":\"0x00000000\"" "core wasm affine resolve unsigned-negative tint"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-negative.out" "\"resolved_delay_ms\":0" "core wasm affine resolve unsigned-negative delay"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-negative.out" "\"resolved_life_ms\":0" "core wasm affine resolve unsigned-negative life"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-negative.out" "\"resolved_image_id\":0" "core wasm affine resolve unsigned-negative image id"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-negative.out" "\"resolved_affine_anchor_mode\":0" "core wasm affine resolve unsigned-negative anchor"

    local code_text_cfg_motion
    code_text_cfg_motion="$(_mfx_core_http_wasm_test_resolve_text_config_http_code \
        "$tmp_dir/wasm-text-config-motion.out" \
        "$base_url" \
        "$token" \
        '{"base_duration_ms":333,"base_float_distance_px":40,"base_font_size_px":20,"life_ms":1200,"vy":-300,"ay":100,"scale":1.0,"color_rgba":4294901760}')"
    mfx_assert_eq "$code_text_cfg_motion" "200" "core wasm text config motion status"
    mfx_assert_file_contains "$tmp_dir/wasm-text-config-motion.out" "\"ok\":true" "core wasm text config motion ok"
    mfx_assert_file_contains "$tmp_dir/wasm-text-config-motion.out" "\"resolved_duration_ms\":1200" "core wasm text config motion duration"
    mfx_assert_file_contains "$tmp_dir/wasm-text-config-motion.out" "\"resolved_float_distance_px\":288" "core wasm text config motion float distance"
    mfx_assert_file_contains "$tmp_dir/wasm-text-config-motion.out" "\"resolved_font_size_px_milli\":20000" "core wasm text config motion font size"
    mfx_assert_file_contains "$tmp_dir/wasm-text-config-motion.out" "\"resolved_color_rgba_hex\":\"0xFFFF0000\"" "core wasm text config motion color"

    local code_text_cfg_clamp
    code_text_cfg_clamp="$(_mfx_core_http_wasm_test_resolve_text_config_http_code \
        "$tmp_dir/wasm-text-config-clamp.out" \
        "$base_url" \
        "$token" \
        '{"base_duration_ms":250,"base_float_distance_px":20,"base_font_size_px":18,"life_ms":1,"vy":0,"ay":0,"scale":100}')"
    mfx_assert_eq "$code_text_cfg_clamp" "200" "core wasm text config clamp status"
    mfx_assert_file_contains "$tmp_dir/wasm-text-config-clamp.out" "\"ok\":true" "core wasm text config clamp ok"
    mfx_assert_file_contains "$tmp_dir/wasm-text-config-clamp.out" "\"resolved_duration_ms\":80" "core wasm text config clamp duration"
    mfx_assert_file_contains "$tmp_dir/wasm-text-config-clamp.out" "\"resolved_float_distance_px\":16" "core wasm text config clamp float distance"
    mfx_assert_file_contains "$tmp_dir/wasm-text-config-clamp.out" "\"resolved_font_size_px_milli\":90000" "core wasm text config clamp font size"

    local code_text_cfg_negative_scale
    code_text_cfg_negative_scale="$(_mfx_core_http_wasm_test_resolve_text_config_http_code \
        "$tmp_dir/wasm-text-config-negative-scale.out" \
        "$base_url" \
        "$token" \
        '{"base_duration_ms":500,"base_float_distance_px":32,"base_font_size_px":24,"life_ms":500,"scale":-1}')"
    mfx_assert_eq "$code_text_cfg_negative_scale" "200" "core wasm text config negative-scale status"
    mfx_assert_file_contains "$tmp_dir/wasm-text-config-negative-scale.out" "\"ok\":true" "core wasm text config negative-scale ok"
    mfx_assert_file_contains "$tmp_dir/wasm-text-config-negative-scale.out" "\"resolved_font_size_px_milli\":24000" "core wasm text config negative-scale font unchanged"
}
