#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_wasm_contract_dispatch_checks() {
    local platform="$1"
    local tmp_dir="$2"
    local base_url="$3"
    local token="$4"
    local repo_root="${5:-}"
    local default_manifest_path="${6:-}"

    local code_wasm_enable
    code_wasm_enable="$(mfx_http_code "$tmp_dir/wasm-enable.out" "$base_url/api/wasm/enable" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{}')"
    mfx_assert_eq "$code_wasm_enable" "200" "core wasm enable status"
    mfx_assert_file_contains "$tmp_dir/wasm-enable.out" "\"ok\":true" "core wasm enable ok"

    local code_state_before_dispatch
    code_state_before_dispatch="$(mfx_http_code "$tmp_dir/state-before-wasm-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_state_before_dispatch" "200" "core wasm state before dispatch status"
    local state_before_invoke_supported
    local state_before_render_supported
    local enable_invoke_supported
    local enable_render_supported
    state_before_invoke_supported="$(_mfx_core_http_wasm_parse_bool_field "$tmp_dir/state-before-wasm-dispatch.out" "invoke_supported")"
    state_before_render_supported="$(_mfx_core_http_wasm_parse_bool_field "$tmp_dir/state-before-wasm-dispatch.out" "render_supported")"
    enable_invoke_supported="$(_mfx_core_http_wasm_parse_bool_field "$tmp_dir/wasm-enable.out" "invoke_supported")"
    enable_render_supported="$(_mfx_core_http_wasm_parse_bool_field "$tmp_dir/wasm-enable.out" "render_supported")"
    mfx_assert_eq "$enable_invoke_supported" "$state_before_invoke_supported" "core wasm enable/state invoke_supported parity"
    mfx_assert_eq "$enable_render_supported" "$state_before_render_supported" "core wasm enable/state render_supported parity"
    if [[ "$platform" == "macos" ]]; then
        local state_before_overlay_max_inflight
        local state_before_overlay_min_image_interval_ms
        local state_before_overlay_min_text_interval_ms
        local enable_overlay_max_inflight
        local enable_overlay_min_image_interval_ms
        local enable_overlay_min_text_interval_ms
        state_before_overlay_max_inflight="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-before-wasm-dispatch.out" "overlay_max_inflight")"
        state_before_overlay_min_image_interval_ms="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-before-wasm-dispatch.out" "overlay_min_image_interval_ms")"
        state_before_overlay_min_text_interval_ms="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-before-wasm-dispatch.out" "overlay_min_text_interval_ms")"
        enable_overlay_max_inflight="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-enable.out" "overlay_max_inflight")"
        enable_overlay_min_image_interval_ms="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-enable.out" "overlay_min_image_interval_ms")"
        enable_overlay_min_text_interval_ms="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-enable.out" "overlay_min_text_interval_ms")"
        mfx_assert_eq "$enable_overlay_max_inflight" "$state_before_overlay_max_inflight" "core wasm enable/state overlay_max_inflight parity"
        mfx_assert_eq "$enable_overlay_min_image_interval_ms" "$state_before_overlay_min_image_interval_ms" "core wasm enable/state overlay_min_image_interval_ms parity"
        mfx_assert_eq "$enable_overlay_min_text_interval_ms" "$state_before_overlay_min_text_interval_ms" "core wasm enable/state overlay_min_text_interval_ms parity"
    fi

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

    local cursor_decoration_policy_manifest_path="$tmp_dir/cursor-decoration-policy-test/plugin.json"
    local cursor_decoration_policy_manifest_path_escaped
    cursor_decoration_policy_manifest_path_escaped="$(_mfx_core_http_wasm_json_escape "$cursor_decoration_policy_manifest_path")"
    local code_cursor_decoration_policy_set
    code_cursor_decoration_policy_set="$(mfx_http_code \
        "$tmp_dir/wasm-policy-cursor-decoration-set.out" \
        "$base_url/api/wasm/policy" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "{\"manifest_path_cursor_decoration\":\"$cursor_decoration_policy_manifest_path_escaped\"}")"
    mfx_assert_eq "$code_cursor_decoration_policy_set" "200" "core wasm cursor-decoration policy set status"
    mfx_assert_file_contains "$tmp_dir/wasm-policy-cursor-decoration-set.out" "\"ok\":true" "core wasm cursor-decoration policy set ok"
    mfx_assert_eq \
        "$(_mfx_core_http_wasm_parse_string_field "$tmp_dir/wasm-policy-cursor-decoration-set.out" "configured_manifest_path_cursor_decoration")" \
        "$cursor_decoration_policy_manifest_path" \
        "core wasm cursor-decoration policy set response configured manifest path"

    local code_state_after_cursor_decoration_policy_set
    code_state_after_cursor_decoration_policy_set="$(mfx_http_code \
        "$tmp_dir/state-after-wasm-policy-cursor-decoration-set.out" \
        "$base_url/api/state" \
        -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_state_after_cursor_decoration_policy_set" "200" "core wasm cursor-decoration policy set state status"
    mfx_assert_eq \
        "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-wasm-policy-cursor-decoration-set.out" "wasm.configured_manifest_path_cursor_decoration")" \
        "$cursor_decoration_policy_manifest_path" \
        "core wasm cursor-decoration policy set state configured manifest path"

    local code_cursor_decoration_policy_clear
    code_cursor_decoration_policy_clear="$(mfx_http_code \
        "$tmp_dir/wasm-policy-cursor-decoration-clear.out" \
        "$base_url/api/wasm/policy" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"manifest_path_cursor_decoration":""}')"
    mfx_assert_eq "$code_cursor_decoration_policy_clear" "200" "core wasm cursor-decoration policy clear status"
    mfx_assert_file_contains "$tmp_dir/wasm-policy-cursor-decoration-clear.out" "\"ok\":true" "core wasm cursor-decoration policy clear ok"
    mfx_assert_eq \
        "$(_mfx_core_http_wasm_parse_string_field "$tmp_dir/wasm-policy-cursor-decoration-clear.out" "configured_manifest_path_cursor_decoration")" \
        "" \
        "core wasm cursor-decoration policy clear response configured manifest path"

    local code_state_after_cursor_decoration_policy_clear
    code_state_after_cursor_decoration_policy_clear="$(mfx_http_code \
        "$tmp_dir/state-after-wasm-policy-cursor-decoration-clear.out" \
        "$base_url/api/state" \
        -H "x-mfcmouseeffect-token: $token")"
    mfx_assert_eq "$code_state_after_cursor_decoration_policy_clear" "200" "core wasm cursor-decoration policy clear state status"
    mfx_assert_eq \
        "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-wasm-policy-cursor-decoration-clear.out" "wasm.configured_manifest_path_cursor_decoration")" \
        "" \
        "core wasm cursor-decoration policy clear state configured manifest path"

    if [[ -n "$repo_root" ]]; then
        local latest_state_snapshot_file="$tmp_dir/state-after-wasm-dispatch.out"
        local indicator_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/indicator-basic/plugin.json"
        if [[ ! -f "$indicator_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample indicator-basic >/dev/null
        fi
        if [[ -f "$indicator_manifest_path" ]]; then
            local indicator_manifest_path_escaped
            indicator_manifest_path_escaped="$(_mfx_core_http_wasm_json_escape "$indicator_manifest_path")"
            local code_load_indicator_manifest
            code_load_indicator_manifest="$(_mfx_core_http_wasm_load_manifest_payload_http_code \
                "$tmp_dir/wasm-load-manifest-indicator-sample.out" \
                "$base_url" \
                "$token" \
                "{\"manifest_path\":\"$indicator_manifest_path_escaped\",\"surface\":\"indicator\"}")"
            mfx_assert_eq "$code_load_indicator_manifest" "200" "core wasm load-manifest indicator sample status"
            mfx_assert_file_contains "$tmp_dir/wasm-load-manifest-indicator-sample.out" "\"ok\":true" "core wasm load-manifest indicator sample ok"
            mfx_assert_eq \
                "$(_mfx_core_http_wasm_parse_string_field "$tmp_dir/wasm-load-manifest-indicator-sample.out" "indicator_active_manifest_path")" \
                "$indicator_manifest_path" \
                "core wasm load-manifest indicator sample active manifest path"
            local code_state_after_indicator_basic_load
            code_state_after_indicator_basic_load="$(mfx_http_code "$tmp_dir/state-after-indicator-basic-load.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_indicator_basic_load" "200" "core wasm state after indicator basic load status"
            mfx_assert_eq \
                "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-indicator-basic-load.out" "input_indicator.wasm_manifest_path")" \
                "$indicator_manifest_path" \
                "core wasm indicator basic load configured manifest path in state"
            mfx_assert_eq \
                "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-indicator-basic-load.out" "input_indicator.render_mode")" \
                "wasm" \
                "core wasm indicator basic load keeps wasm render mode"
            mfx_assert_eq \
                "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-indicator-basic-load.out" "wasm.indicator_active_manifest_path")" \
                "$indicator_manifest_path" \
                "core wasm indicator basic load active manifest path in state"

            local indicator_stale_manifest_path="${indicator_manifest_path}.missing"
            local indicator_stale_manifest_payload_file="$tmp_dir/state-apply-indicator-stale-manifest.json"
            cat >"$indicator_stale_manifest_payload_file" <<EOF
{
  "input_indicator": {
    "render_mode": "wasm",
    "wasm_manifest_path": "$indicator_stale_manifest_path"
  }
}
EOF
            local code_apply_indicator_stale_manifest
            code_apply_indicator_stale_manifest="$(mfx_http_code "$tmp_dir/state-apply-indicator-stale-manifest.out" "$base_url/api/state" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                --data-binary "@$indicator_stale_manifest_payload_file")"
            mfx_assert_eq "$code_apply_indicator_stale_manifest" "200" "core state apply indicator stale manifest status"
            mfx_assert_file_contains "$tmp_dir/state-apply-indicator-stale-manifest.out" "\"ok\":true" "core state apply indicator stale manifest ok"

            local code_state_after_indicator_stale_manifest_apply
            code_state_after_indicator_stale_manifest_apply="$(mfx_http_code "$tmp_dir/state-after-indicator-stale-manifest-apply.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_indicator_stale_manifest_apply" "200" "core wasm state after indicator stale manifest apply status"
            mfx_assert_eq \
                "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-indicator-stale-manifest-apply.out" "input_indicator.render_mode")" \
                "native" \
                "core stale indicator manifest degrades render mode to native"
            mfx_assert_eq \
                "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-indicator-stale-manifest-apply.out" "input_indicator.wasm_manifest_path")" \
                "" \
                "core stale indicator manifest clears configured wasm manifest path"

            local code_load_indicator_manifest_after_stale_recover
            code_load_indicator_manifest_after_stale_recover="$(_mfx_core_http_wasm_load_manifest_payload_http_code \
                "$tmp_dir/wasm-load-manifest-indicator-basic-after-stale-recover.out" \
                "$base_url" \
                "$token" \
                "{\"manifest_path\":\"$indicator_manifest_path_escaped\",\"surface\":\"indicator\"}")"
            mfx_assert_eq "$code_load_indicator_manifest_after_stale_recover" "200" "core wasm load-manifest indicator basic after stale recover status"
            mfx_assert_file_contains "$tmp_dir/wasm-load-manifest-indicator-basic-after-stale-recover.out" "\"ok\":true" "core wasm load-manifest indicator basic after stale recover ok"
            local code_state_after_indicator_stale_manifest_recover
            code_state_after_indicator_stale_manifest_recover="$(mfx_http_code "$tmp_dir/state-after-indicator-stale-manifest-recover.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_indicator_stale_manifest_recover" "200" "core wasm state after indicator stale manifest recover status"
            mfx_assert_eq \
                "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-indicator-stale-manifest-recover.out" "input_indicator.render_mode")" \
                "wasm" \
                "core stale indicator manifest recover keeps wasm render mode"
            mfx_assert_eq \
                "$(_mfx_core_http_read_json_string "$tmp_dir/state-after-indicator-stale-manifest-recover.out" "input_indicator.wasm_manifest_path")" \
                "$indicator_manifest_path" \
                "core stale indicator manifest recover restores configured wasm manifest path"

            local code_indicator_dispatch
            code_indicator_dispatch="$(_mfx_core_http_wasm_test_dispatch_indicator_key_http_code "$tmp_dir/wasm-test-dispatch-indicator-key.out" "$base_url" "$token")"
            mfx_assert_eq "$code_indicator_dispatch" "200" "core wasm test-dispatch indicator-key status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-indicator-key.out" "\"ok\":true" "core wasm indicator-key response ok"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-indicator-key.out" "\"route_active\":true" "core wasm indicator-key route active"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-indicator-key.out" "\"invoke_ok\":true" "core wasm indicator-key invoke ok"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-indicator-key.out" "\"supports_indicator_key\":true" "core wasm indicator-key route support"

            local code_indicator_app_dispatch_wasm
            code_indicator_app_dispatch_wasm="$(_mfx_core_http_wasm_test_dispatch_app_indicator_key_http_code \
                "$tmp_dir/wasm-test-dispatch-app-indicator-key-wasm.out" \
                "$base_url" \
                "$token" \
                '{"render_mode":"wasm","enabled":true,"keyboard_enabled":true,"wasm_fallback_to_native":false,"key_display_mode":"all","x":640,"y":360,"vk_code":88}')"
            mfx_assert_eq "$code_indicator_app_dispatch_wasm" "200" "core wasm app indicator dispatch wasm status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-app-indicator-key-wasm.out" "\"ok\":true" "core wasm app indicator dispatch wasm ok"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-app-indicator-key-wasm.out" "\"event_kind\":\"key\"" "core wasm app indicator dispatch wasm event kind"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-app-indicator-key-wasm.out" "\"route_attempted\":true" "core wasm app indicator dispatch wasm route attempted"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-app-indicator-key-wasm.out" "\"event_supported\":true" "core wasm app indicator dispatch wasm event supported"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-app-indicator-key-wasm.out" "\"rendered_by_wasm\":true" "core wasm app indicator dispatch wasm rendered"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-app-indicator-key-wasm.out" "\"native_fallback_applied\":false" "core wasm app indicator dispatch wasm no native fallback"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-app-indicator-key-wasm.out" "\"reason\":\"wasm_rendered\"" "core wasm app indicator dispatch wasm reason"

            local indicator_fallback_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-polyline-zigzag/plugin.json"
            if [[ ! -f "$indicator_fallback_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
                npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-polyline-zigzag >/dev/null
            fi
            if [[ -f "$indicator_fallback_manifest_path" ]]; then
                local indicator_fallback_manifest_path_escaped
                indicator_fallback_manifest_path_escaped="$(_mfx_core_http_wasm_json_escape "$indicator_fallback_manifest_path")"
                local code_load_indicator_fallback_manifest
                code_load_indicator_fallback_manifest="$(_mfx_core_http_wasm_load_manifest_payload_http_code \
                    "$tmp_dir/wasm-load-manifest-indicator-fallback-sample.out" \
                    "$base_url" \
                    "$token" \
                    "{\"manifest_path\":\"$indicator_fallback_manifest_path_escaped\",\"surface\":\"indicator\"}")"
                mfx_assert_eq "$code_load_indicator_fallback_manifest" "200" "core wasm load-manifest indicator fallback sample status"
                mfx_assert_file_contains "$tmp_dir/wasm-load-manifest-indicator-fallback-sample.out" "\"ok\":true" "core wasm load-manifest indicator fallback sample ok"
                mfx_assert_eq \
                    "$(_mfx_core_http_wasm_parse_string_field "$tmp_dir/wasm-load-manifest-indicator-fallback-sample.out" "indicator_active_manifest_path")" \
                    "$indicator_fallback_manifest_path" \
                    "core wasm load-manifest indicator fallback sample active manifest path"

                local code_indicator_app_dispatch_fallback
                code_indicator_app_dispatch_fallback="$(_mfx_core_http_wasm_test_dispatch_app_indicator_key_http_code \
                    "$tmp_dir/wasm-test-dispatch-app-indicator-key-fallback.out" \
                    "$base_url" \
                    "$token" \
                    '{"render_mode":"wasm","enabled":true,"keyboard_enabled":true,"wasm_fallback_to_native":true,"key_display_mode":"all","x":640,"y":360,"vk_code":88}')"
                mfx_assert_eq "$code_indicator_app_dispatch_fallback" "200" "core wasm app indicator dispatch fallback status"
                mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-app-indicator-key-fallback.out" "\"ok\":true" "core wasm app indicator dispatch fallback ok"
                mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-app-indicator-key-fallback.out" "\"rendered_by_wasm\":false" "core wasm app indicator dispatch fallback not rendered"
                mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-app-indicator-key-fallback.out" "\"event_supported\":false" "core wasm app indicator dispatch fallback unsupported event"
                mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-app-indicator-key-fallback.out" "\"native_fallback_applied\":true" "core wasm app indicator dispatch fallback applied"
                mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-app-indicator-key-fallback.out" "\"reason\":\"event_not_supported\"" "core wasm app indicator dispatch fallback reason"
                local code_state_after_indicator_fallback
                code_state_after_indicator_fallback="$(mfx_http_code "$tmp_dir/state-after-indicator-fallback.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
                mfx_assert_eq "$code_state_after_indicator_fallback" "200" "core wasm state after indicator fallback status"
                mfx_assert_file_contains "$tmp_dir/state-after-indicator-fallback.out" "\"input_indicator_wasm_route_status\"" "core wasm indicator fallback route status exported"
                mfx_assert_file_contains "$tmp_dir/state-after-indicator-fallback.out" "\"reason\":\"event_not_supported\"" "core wasm indicator fallback route status reason"

                local code_load_indicator_manifest_restore
                code_load_indicator_manifest_restore="$(_mfx_core_http_wasm_load_manifest_payload_http_code \
                    "$tmp_dir/wasm-load-manifest-indicator-sample-restore.out" \
                    "$base_url" \
                    "$token" \
                    "{\"manifest_path\":\"$indicator_manifest_path_escaped\",\"surface\":\"indicator\"}")"
                mfx_assert_eq "$code_load_indicator_manifest_restore" "200" "core wasm load-manifest indicator sample restore status"
                mfx_assert_file_contains "$tmp_dir/wasm-load-manifest-indicator-sample-restore.out" "\"ok\":true" "core wasm load-manifest indicator sample restore ok"
                mfx_assert_eq \
                    "$(_mfx_core_http_wasm_parse_string_field "$tmp_dir/wasm-load-manifest-indicator-sample-restore.out" "indicator_active_manifest_path")" \
                    "$indicator_manifest_path" \
                    "core wasm load-manifest indicator sample restore active manifest path"
            fi

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-indicator-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after indicator sample"
            fi
        fi

        local polyline_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-polyline-zigzag/plugin.json"
        if [[ ! -f "$polyline_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-polyline-zigzag >/dev/null
        fi
        if [[ -f "$polyline_manifest_path" ]]; then
            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-polyline-sample.out" \
                "$base_url" \
                "$token" \
                "$polyline_manifest_path" \
                "core wasm load-manifest polyline sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-polyline.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch polyline sample" \
                "$require_rendered_any"

            local dispatch_polyline_commands
            dispatch_polyline_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-polyline.out" "executed_polyline_commands")"
            if [[ -z "$dispatch_polyline_commands" || "$dispatch_polyline_commands" -lt 1 ]]; then
                mfx_fail "core wasm polyline sample did not execute polyline commands"
            fi

            local code_state_after_polyline_dispatch
            code_state_after_polyline_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-polyline-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_polyline_dispatch" "200" "core wasm state after polyline dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-polyline.out" \
                "$tmp_dir/state-after-wasm-polyline-dispatch.out" \
                "core wasm polyline dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"
            latest_state_snapshot_file="$tmp_dir/state-after-wasm-polyline-dispatch.out"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-polyline-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after polyline sample"
            fi
        fi

        local path_stroke_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-path-stroke-ribbon/plugin.json"
        if [[ ! -f "$path_stroke_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-path-stroke-ribbon >/dev/null
        fi
        if [[ -f "$path_stroke_manifest_path" ]]; then
            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-path-stroke-sample.out" \
                "$base_url" \
                "$token" \
                "$path_stroke_manifest_path" \
                "core wasm load-manifest path-stroke sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-path-stroke.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch path-stroke sample" \
                "$require_rendered_any"

            local dispatch_path_stroke_commands
            dispatch_path_stroke_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-path-stroke.out" "executed_path_stroke_commands")"
            if [[ -z "$dispatch_path_stroke_commands" || "$dispatch_path_stroke_commands" -lt 1 ]]; then
                mfx_fail "core wasm path-stroke sample did not execute path-stroke commands"
            fi

            local code_state_after_path_stroke_dispatch
            code_state_after_path_stroke_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-path-stroke-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_path_stroke_dispatch" "200" "core wasm state after path-stroke dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-path-stroke.out" \
                "$tmp_dir/state-after-wasm-path-stroke-dispatch.out" \
                "core wasm path-stroke dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"
            latest_state_snapshot_file="$tmp_dir/state-after-wasm-path-stroke-dispatch.out"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-path-stroke-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after path-stroke sample"
            fi
        fi

        local path_fill_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-path-fill-badge/plugin.json"
        if [[ ! -f "$path_fill_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-path-fill-badge >/dev/null
        fi
        if [[ -f "$path_fill_manifest_path" ]]; then
            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-path-fill-sample.out" \
                "$base_url" \
                "$token" \
                "$path_fill_manifest_path" \
                "core wasm load-manifest path-fill sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-path-fill.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch path-fill sample" \
                "$require_rendered_any"

            local dispatch_path_fill_commands
            dispatch_path_fill_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-path-fill.out" "executed_path_fill_commands")"
            if [[ -z "$dispatch_path_fill_commands" || "$dispatch_path_fill_commands" -lt 1 ]]; then
                mfx_fail "core wasm path-fill sample did not execute path-fill commands"
            fi

            local code_state_after_path_fill_dispatch
            code_state_after_path_fill_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-path-fill-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_path_fill_dispatch" "200" "core wasm state after path-fill dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-path-fill.out" \
                "$tmp_dir/state-after-wasm-path-fill-dispatch.out" \
                "core wasm path-fill dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"
            latest_state_snapshot_file="$tmp_dir/state-after-wasm-path-fill-dispatch.out"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-path-fill-sample.out" \
                    "$base_url" \
                    "$token" \
                "$default_manifest_path" \
                "core wasm load-manifest restore after path-fill sample"
            fi
        fi

        local path_fill_clip_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-path-fill-clip-window/plugin.json"
        if [[ ! -f "$path_fill_clip_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-path-fill-clip-window >/dev/null
        fi
        if [[ -f "$path_fill_clip_manifest_path" ]]; then
            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-path-fill-clip-sample.out" \
                "$base_url" \
                "$token" \
                "$path_fill_clip_manifest_path" \
                "core wasm load-manifest path-fill clip sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-path-fill-clip.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch path-fill clip sample" \
                "$require_rendered_any"

            local dispatch_path_fill_clip_commands
            dispatch_path_fill_clip_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-path-fill-clip.out" "executed_path_fill_commands")"
            if [[ -z "$dispatch_path_fill_clip_commands" || "$dispatch_path_fill_clip_commands" -lt 1 ]]; then
                mfx_fail "core wasm path-fill clip sample did not execute path-fill commands"
            fi

            local code_state_after_path_fill_clip_dispatch
            code_state_after_path_fill_clip_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-path-fill-clip-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_path_fill_clip_dispatch" "200" "core wasm state after path-fill clip dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-path-fill-clip.out" \
                "$tmp_dir/state-after-wasm-path-fill-clip-dispatch.out" \
                "core wasm path-fill clip dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"
            latest_state_snapshot_file="$tmp_dir/state-after-wasm-path-fill-clip-dispatch.out"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-path-fill-clip-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after path-fill clip sample"
            fi
        fi

        local ribbon_strip_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-ribbon-trace/plugin.json"
        if [[ ! -f "$ribbon_strip_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-ribbon-trace >/dev/null
        fi
        if [[ -f "$ribbon_strip_manifest_path" ]]; then
            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-ribbon-strip-sample.out" \
                "$base_url" \
                "$token" \
                "$ribbon_strip_manifest_path" \
                "core wasm load-manifest ribbon-strip sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-ribbon-strip.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch ribbon-strip sample" \
                "$require_rendered_any"

            local dispatch_ribbon_strip_commands
            dispatch_ribbon_strip_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-ribbon-strip.out" "executed_path_fill_commands")"
            if [[ -z "$dispatch_ribbon_strip_commands" || "$dispatch_ribbon_strip_commands" -lt 1 ]]; then
                mfx_fail "core wasm ribbon-strip sample did not execute filled path lane commands"
            fi

            local code_state_after_ribbon_strip_dispatch
            code_state_after_ribbon_strip_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-ribbon-strip-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_ribbon_strip_dispatch" "200" "core wasm state after ribbon-strip dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-ribbon-strip.out" \
                "$tmp_dir/state-after-wasm-ribbon-strip-dispatch.out" \
                "core wasm ribbon-strip dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"
            latest_state_snapshot_file="$tmp_dir/state-after-wasm-ribbon-strip-dispatch.out"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-ribbon-strip-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after ribbon-strip sample"
            fi
        fi

        local glow_batch_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-glow-burst/plugin.json"
        if [[ ! -f "$glow_batch_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-glow-burst >/dev/null
        fi
        if [[ -f "$glow_batch_manifest_path" ]]; then
            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-glow-batch-sample.out" \
                "$base_url" \
                "$token" \
                "$glow_batch_manifest_path" \
                "core wasm load-manifest glow-batch sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-glow-batch.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch glow-batch sample" \
                "$require_rendered_any"

            local dispatch_glow_batch_commands
            dispatch_glow_batch_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-glow-batch.out" "executed_glow_batch_commands")"
            if [[ -z "$dispatch_glow_batch_commands" || "$dispatch_glow_batch_commands" -lt 1 ]]; then
                mfx_fail "core wasm glow-batch sample did not execute glow-batch commands"
            fi

            local code_state_after_glow_batch_dispatch
            code_state_after_glow_batch_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-glow-batch-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_glow_batch_dispatch" "200" "core wasm state after glow-batch dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-glow-batch.out" \
                "$tmp_dir/state-after-wasm-glow-batch-dispatch.out" \
                "core wasm glow-batch dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-glow-batch-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after glow-batch sample"
            fi
            latest_state_snapshot_file="$tmp_dir/state-after-wasm-glow-batch-dispatch.out"
        fi

        local sprite_batch_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-sprite-burst/plugin.json"
        if [[ ! -f "$sprite_batch_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-sprite-burst >/dev/null
        fi
        if [[ -f "$sprite_batch_manifest_path" ]]; then
            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-sprite-batch-sample.out" \
                "$base_url" \
                "$token" \
                "$sprite_batch_manifest_path" \
                "core wasm load-manifest sprite-batch sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-sprite-batch.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch sprite-batch sample" \
                "$require_rendered_any"

            local dispatch_sprite_batch_commands
            dispatch_sprite_batch_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-sprite-batch.out" "executed_sprite_batch_commands")"
            if [[ -z "$dispatch_sprite_batch_commands" || "$dispatch_sprite_batch_commands" -lt 1 ]]; then
                mfx_fail "core wasm sprite-batch sample did not execute sprite-batch commands"
            fi

            local code_state_after_sprite_batch_dispatch
            code_state_after_sprite_batch_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-sprite-batch-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_sprite_batch_dispatch" "200" "core wasm state after sprite-batch dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-sprite-batch.out" \
                "$tmp_dir/state-after-wasm-sprite-batch-dispatch.out" \
                "core wasm sprite-batch dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-sprite-batch-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after sprite-batch sample"
            fi
            latest_state_snapshot_file="$tmp_dir/state-after-wasm-sprite-batch-dispatch.out"
        fi

        local quad_batch_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-quad-atlas-burst/plugin.json"
        if [[ ! -f "$quad_batch_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-quad-atlas-burst >/dev/null
        fi
        if [[ -f "$quad_batch_manifest_path" ]]; then
            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-quad-batch-sample.out" \
                "$base_url" \
                "$token" \
                "$quad_batch_manifest_path" \
                "core wasm load-manifest quad-batch sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-quad-batch.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch quad-batch sample" \
                "$require_rendered_any"

            local dispatch_quad_batch_commands
            dispatch_quad_batch_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-quad-batch.out" "executed_sprite_batch_commands")"
            if [[ -z "$dispatch_quad_batch_commands" || "$dispatch_quad_batch_commands" -lt 1 ]]; then
                mfx_fail "core wasm quad-batch sample did not execute sprite-batch lane commands"
            fi

            local code_state_after_quad_batch_dispatch
            code_state_after_quad_batch_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-quad-batch-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_quad_batch_dispatch" "200" "core wasm state after quad-batch dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-quad-batch.out" \
                "$tmp_dir/state-after-wasm-quad-batch-dispatch.out" \
                "core wasm quad-batch dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-quad-batch-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after quad-batch sample"
            fi
            latest_state_snapshot_file="$tmp_dir/state-after-wasm-quad-batch-dispatch.out"
        fi

        local retained_glow_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-retained-glow-field/plugin.json"
        if [[ ! -f "$retained_glow_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-retained-glow-field >/dev/null
        fi
        if [[ -f "$retained_glow_manifest_path" ]]; then
            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-retained-glow-sample.out" \
                "$base_url" \
                "$token" \
                "$retained_glow_manifest_path" \
                "core wasm load-manifest retained-glow sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-retained-glow.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch retained-glow sample" \
                "$require_rendered_any"

            local dispatch_glow_emitter_commands
            dispatch_glow_emitter_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-glow.out" "executed_glow_emitter_commands")"
            if [[ -z "$dispatch_glow_emitter_commands" || "$dispatch_glow_emitter_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-glow sample did not execute glow-emitter commands"
            fi

            local code_state_after_retained_glow_dispatch
            code_state_after_retained_glow_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-glow-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_glow_dispatch" "200" "core wasm state after retained-glow dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-glow.out" \
                "$tmp_dir/state-after-wasm-retained-glow-dispatch.out" \
                "core wasm retained-glow dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            local retained_glow_active_count
            retained_glow_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-glow-dispatch.out" "retained_glow_emitter_active_count")"
            if [[ -z "$retained_glow_active_count" || "$retained_glow_active_count" -lt 1 ]]; then
                mfx_fail "core wasm retained-glow sample did not keep an active retained emitter"
            fi

            local code_retained_glow_remove_dispatch
            code_retained_glow_remove_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-glow-remove.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":640,"y":360,"button":2}')"
            mfx_assert_eq "$code_retained_glow_remove_dispatch" "200" "core wasm retained-glow remove dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-glow-remove.out" "\"ok\":true" "core wasm retained-glow remove dispatch ok"

            local dispatch_glow_emitter_remove_commands
            dispatch_glow_emitter_remove_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-glow-remove.out" "executed_glow_emitter_remove_commands")"
            if [[ -z "$dispatch_glow_emitter_remove_commands" || "$dispatch_glow_emitter_remove_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-glow sample did not execute glow-emitter remove commands"
            fi

            local code_state_after_retained_glow_remove
            code_state_after_retained_glow_remove="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-glow-remove.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_glow_remove" "200" "core wasm state after retained-glow remove status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-glow-remove.out" \
                "$tmp_dir/state-after-wasm-retained-glow-remove.out" \
                "core wasm retained-glow remove diagnostics consistency" \
                "$tmp_dir/state-after-wasm-retained-glow-dispatch.out" \
                "$platform"

            local retained_glow_active_count_after_remove
            retained_glow_active_count_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-glow-remove.out" "retained_glow_emitter_active_count")"
            if [[ -z "$retained_glow_active_count_after_remove" || "$retained_glow_active_count_after_remove" -ne 0 ]]; then
                mfx_fail "core wasm retained-glow sample did not clear the retained emitter after remove dispatch"
            fi

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-retained-glow-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after retained-glow sample"
            fi
        fi

        local retained_sprite_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-retained-sprite-fountain/plugin.json"
        if [[ ! -f "$retained_sprite_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-retained-sprite-fountain >/dev/null
        fi
        if [[ -f "$retained_sprite_manifest_path" ]]; then
            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-retained-sprite-sample.out" \
                "$base_url" \
                "$token" \
                "$retained_sprite_manifest_path" \
                "core wasm load-manifest retained-sprite sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-retained-sprite.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch retained-sprite sample" \
                "$require_rendered_any"

            local dispatch_sprite_emitter_commands
            dispatch_sprite_emitter_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-sprite.out" "executed_sprite_emitter_commands")"
            if [[ -z "$dispatch_sprite_emitter_commands" || "$dispatch_sprite_emitter_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-sprite sample did not execute sprite-emitter commands"
            fi

            local code_state_after_retained_sprite_dispatch
            code_state_after_retained_sprite_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-sprite-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_sprite_dispatch" "200" "core wasm state after retained-sprite dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-sprite.out" \
                "$tmp_dir/state-after-wasm-retained-sprite-dispatch.out" \
                "core wasm retained-sprite dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            local retained_sprite_active_count
            retained_sprite_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-sprite-dispatch.out" "retained_sprite_emitter_active_count")"
            if [[ -z "$retained_sprite_active_count" || "$retained_sprite_active_count" -lt 1 ]]; then
                mfx_fail "core wasm retained-sprite sample did not keep an active retained sprite emitter"
            fi

            local code_retained_sprite_remove_dispatch
            code_retained_sprite_remove_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-sprite-remove.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":640,"y":360,"button":2}')"
            mfx_assert_eq "$code_retained_sprite_remove_dispatch" "200" "core wasm retained-sprite remove dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-sprite-remove.out" "\"ok\":true" "core wasm retained-sprite remove dispatch ok"

            local dispatch_sprite_emitter_remove_commands
            dispatch_sprite_emitter_remove_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-sprite-remove.out" "executed_sprite_emitter_remove_commands")"
            if [[ -z "$dispatch_sprite_emitter_remove_commands" || "$dispatch_sprite_emitter_remove_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-sprite sample did not execute sprite-emitter remove commands"
            fi

            local code_state_after_retained_sprite_remove
            code_state_after_retained_sprite_remove="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-sprite-remove.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_sprite_remove" "200" "core wasm state after retained-sprite remove status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-sprite-remove.out" \
                "$tmp_dir/state-after-wasm-retained-sprite-remove.out" \
                "core wasm retained-sprite remove diagnostics consistency" \
                "$tmp_dir/state-after-wasm-retained-sprite-dispatch.out" \
                "$platform"

            local retained_sprite_active_count_after_remove
            retained_sprite_active_count_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-sprite-remove.out" "retained_sprite_emitter_active_count")"
            if [[ -z "$retained_sprite_active_count_after_remove" || "$retained_sprite_active_count_after_remove" -ne 0 ]]; then
                mfx_fail "core wasm retained-sprite sample did not clear the retained sprite emitter after remove dispatch"
            fi

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-retained-sprite-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after retained-sprite sample"
            fi
        fi

        local retained_particle_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-retained-particle-field/plugin.json"
        if [[ ! -f "$retained_particle_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-retained-particle-field >/dev/null
        fi
        if [[ -f "$retained_particle_manifest_path" ]]; then
            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-retained-particle-sample.out" \
                "$base_url" \
                "$token" \
                "$retained_particle_manifest_path" \
                "core wasm load-manifest retained-particle sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-retained-particle.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch retained-particle sample" \
                "$require_rendered_any"

            local dispatch_particle_emitter_commands
            dispatch_particle_emitter_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-particle.out" "executed_particle_emitter_commands")"
            if [[ -z "$dispatch_particle_emitter_commands" || "$dispatch_particle_emitter_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-particle sample did not execute particle-emitter commands"
            fi

            local code_state_after_retained_particle_dispatch
            code_state_after_retained_particle_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-particle-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_particle_dispatch" "200" "core wasm state after retained-particle dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-particle.out" \
                "$tmp_dir/state-after-wasm-retained-particle-dispatch.out" \
                "core wasm retained-particle dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            local retained_particle_active_count
            retained_particle_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-particle-dispatch.out" "retained_particle_emitter_active_count")"
            if [[ -z "$retained_particle_active_count" || "$retained_particle_active_count" -lt 1 ]]; then
                mfx_fail "core wasm retained-particle sample did not keep an active retained particle emitter"
            fi

            local code_retained_particle_remove_dispatch
            code_retained_particle_remove_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-particle-remove.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":640,"y":360,"button":2}')"
            mfx_assert_eq "$code_retained_particle_remove_dispatch" "200" "core wasm retained-particle remove dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-particle-remove.out" "\"ok\":true" "core wasm retained-particle remove dispatch ok"

            local dispatch_particle_emitter_remove_commands
            dispatch_particle_emitter_remove_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-particle-remove.out" "executed_particle_emitter_remove_commands")"
            if [[ -z "$dispatch_particle_emitter_remove_commands" || "$dispatch_particle_emitter_remove_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-particle sample did not execute particle-emitter remove commands"
            fi

            local code_state_after_retained_particle_remove
            code_state_after_retained_particle_remove="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-particle-remove.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_particle_remove" "200" "core wasm state after retained-particle remove status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-particle-remove.out" \
                "$tmp_dir/state-after-wasm-retained-particle-remove.out" \
                "core wasm retained-particle remove diagnostics consistency" \
                "$tmp_dir/state-after-wasm-retained-particle-dispatch.out" \
                "$platform"

            local retained_particle_active_count_after_remove
            retained_particle_active_count_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-particle-remove.out" "retained_particle_emitter_active_count")"
            if [[ -z "$retained_particle_active_count_after_remove" || "$retained_particle_active_count_after_remove" -ne 0 ]]; then
                mfx_fail "core wasm retained-particle sample did not clear the retained particle emitter after remove dispatch"
            fi

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-retained-particle-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after retained-particle sample"
            fi
        fi

        local retained_ribbon_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-retained-ribbon-trail/plugin.json"
        if [[ ! -f "$retained_ribbon_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-retained-ribbon-trail >/dev/null
        fi
        if [[ -f "$retained_ribbon_manifest_path" ]]; then
            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-retained-ribbon-sample.out" \
                "$base_url" \
                "$token" \
                "$retained_ribbon_manifest_path" \
                "core wasm load-manifest retained-ribbon sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-retained-ribbon.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch retained-ribbon sample" \
                "$require_rendered_any"

            local dispatch_ribbon_trail_commands
            dispatch_ribbon_trail_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-ribbon.out" "executed_ribbon_trail_commands")"
            if [[ -z "$dispatch_ribbon_trail_commands" || "$dispatch_ribbon_trail_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-ribbon sample did not execute ribbon-trail commands"
            fi

            local code_state_after_retained_ribbon_dispatch
            code_state_after_retained_ribbon_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-ribbon-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_ribbon_dispatch" "200" "core wasm state after retained-ribbon dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-ribbon.out" \
                "$tmp_dir/state-after-wasm-retained-ribbon-dispatch.out" \
                "core wasm retained-ribbon dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            local retained_ribbon_active_count
            retained_ribbon_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-ribbon-dispatch.out" "retained_ribbon_trail_active_count")"
            if [[ -z "$retained_ribbon_active_count" || "$retained_ribbon_active_count" -lt 1 ]]; then
                mfx_fail "core wasm retained-ribbon sample did not keep an active retained ribbon trail"
            fi

            local code_retained_ribbon_remove_dispatch
            code_retained_ribbon_remove_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-ribbon-remove.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":640,"y":360,"button":2}')"
            mfx_assert_eq "$code_retained_ribbon_remove_dispatch" "200" "core wasm retained-ribbon remove dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-ribbon-remove.out" "\"ok\":true" "core wasm retained-ribbon remove dispatch ok"

            local dispatch_ribbon_trail_remove_commands
            dispatch_ribbon_trail_remove_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-ribbon-remove.out" "executed_ribbon_trail_remove_commands")"
            if [[ -z "$dispatch_ribbon_trail_remove_commands" || "$dispatch_ribbon_trail_remove_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-ribbon sample did not execute ribbon-trail remove commands"
            fi

            local code_state_after_retained_ribbon_remove
            code_state_after_retained_ribbon_remove="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-ribbon-remove.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_ribbon_remove" "200" "core wasm state after retained-ribbon remove status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-ribbon-remove.out" \
                "$tmp_dir/state-after-wasm-retained-ribbon-remove.out" \
                "core wasm retained-ribbon remove diagnostics consistency" \
                "$tmp_dir/state-after-wasm-retained-ribbon-dispatch.out" \
                "$platform"

            local retained_ribbon_active_count_after_remove
            retained_ribbon_active_count_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-ribbon-remove.out" "retained_ribbon_trail_active_count")"
            if [[ -z "$retained_ribbon_active_count_after_remove" || "$retained_ribbon_active_count_after_remove" -ne 0 ]]; then
                mfx_fail "core wasm retained-ribbon sample did not clear the retained ribbon trail after remove dispatch"
            fi

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-retained-ribbon-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after retained-ribbon sample"
            fi
        fi

        local retained_quad_field_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-retained-quad-field/plugin.json"
        if [[ ! -f "$retained_quad_field_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-retained-quad-field >/dev/null
        fi
        if [[ -f "$retained_quad_field_manifest_path" ]]; then
            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-retained-quad-field-sample.out" \
                "$base_url" \
                "$token" \
                "$retained_quad_field_manifest_path" \
                "core wasm load-manifest retained-quad-field sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-retained-quad-field.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch retained-quad-field sample" \
                "$require_rendered_any"

            local dispatch_quad_field_commands
            dispatch_quad_field_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-quad-field.out" "executed_quad_field_commands")"
            if [[ -z "$dispatch_quad_field_commands" || "$dispatch_quad_field_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-quad-field sample did not execute quad-field commands"
            fi

            local code_state_after_retained_quad_field_dispatch
            code_state_after_retained_quad_field_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-quad-field-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_quad_field_dispatch" "200" "core wasm state after retained-quad-field dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-quad-field.out" \
                "$tmp_dir/state-after-wasm-retained-quad-field-dispatch.out" \
                "core wasm retained-quad-field dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            local retained_quad_field_active_count
            retained_quad_field_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-quad-field-dispatch.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_quad_field_active_count" || "$retained_quad_field_active_count" -lt 1 ]]; then
                mfx_fail "core wasm retained-quad-field sample did not keep an active retained quad field"
            fi

            local code_retained_quad_field_remove_dispatch
            code_retained_quad_field_remove_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-quad-field-remove.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":640,"y":360,"button":2}')"
            mfx_assert_eq "$code_retained_quad_field_remove_dispatch" "200" "core wasm retained-quad-field remove dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-quad-field-remove.out" "\"ok\":true" "core wasm retained-quad-field remove dispatch ok"

            local dispatch_quad_field_remove_commands
            dispatch_quad_field_remove_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-quad-field-remove.out" "executed_quad_field_remove_commands")"
            if [[ -z "$dispatch_quad_field_remove_commands" || "$dispatch_quad_field_remove_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-quad-field sample did not execute quad-field remove commands"
            fi

            local code_state_after_retained_quad_field_remove
            code_state_after_retained_quad_field_remove="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-quad-field-remove.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_quad_field_remove" "200" "core wasm state after retained-quad-field remove status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-quad-field-remove.out" \
                "$tmp_dir/state-after-wasm-retained-quad-field-remove.out" \
                "core wasm retained-quad-field remove diagnostics consistency" \
                "$tmp_dir/state-after-wasm-retained-quad-field-dispatch.out" \
                "$platform"

            local retained_quad_field_active_count_after_remove
            retained_quad_field_active_count_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-quad-field-remove.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_quad_field_active_count_after_remove" || "$retained_quad_field_active_count_after_remove" -ne 0 ]]; then
                mfx_fail "core wasm retained-quad-field sample did not clear the retained quad field after remove dispatch"
            fi

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-retained-quad-field-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after retained-quad-field sample"
            fi
        fi

        local retained_group_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-retained-group-clear/plugin.json"
        if [[ ! -f "$retained_group_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-retained-group-clear >/dev/null
        fi
        if [[ -f "$retained_group_manifest_path" ]]; then
            local retained_group_baseline_max_commands
            retained_group_baseline_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$latest_state_snapshot_file" "configured_max_commands")"
            if [[ -z "$retained_group_baseline_max_commands" || "$retained_group_baseline_max_commands" -lt 3 ]]; then
                local code_retained_group_policy
                code_retained_group_policy="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d '{"max_commands":8}')"
                mfx_assert_eq "$code_retained_group_policy" "200" "core wasm retained-group policy status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group.out" "\"ok\":true" "core wasm retained-group policy ok"
                local retained_group_policy_configured_max_commands
                local retained_group_policy_runtime_max_commands
                retained_group_policy_configured_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-policy-retained-group.out" "configured_max_commands")"
                retained_group_policy_runtime_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-policy-retained-group.out" "runtime_max_commands")"
                if [[ -z "$retained_group_policy_configured_max_commands" || "$retained_group_policy_configured_max_commands" -lt 8 ]]; then
                    mfx_fail "core wasm retained-group policy did not raise configured_max_commands"
                fi
                if [[ -z "$retained_group_policy_runtime_max_commands" || "$retained_group_policy_runtime_max_commands" -lt 8 ]]; then
                    mfx_fail "core wasm retained-group policy did not raise runtime_max_commands"
                fi
            fi

            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-retained-group-sample.out" \
                "$base_url" \
                "$token" \
                "$retained_group_manifest_path" \
                "core wasm load-manifest retained-group sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-retained-group.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch retained-group sample" \
                "$require_rendered_any"

            local dispatch_group_glow_commands
            local dispatch_group_ribbon_commands
            local dispatch_group_quad_commands
            dispatch_group_glow_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group.out" "executed_glow_emitter_commands")"
            dispatch_group_ribbon_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group.out" "executed_ribbon_trail_commands")"
            dispatch_group_quad_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group.out" "executed_quad_field_commands")"
            if [[ -z "$dispatch_group_glow_commands" || "$dispatch_group_glow_commands" -lt 1 || -z "$dispatch_group_ribbon_commands" || "$dispatch_group_ribbon_commands" -lt 1 || -z "$dispatch_group_quad_commands" || "$dispatch_group_quad_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group sample did not execute grouped retained commands"
            fi

            local code_state_after_retained_group_dispatch
            code_state_after_retained_group_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_dispatch" "200" "core wasm state after retained-group dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group.out" \
                "$tmp_dir/state-after-wasm-retained-group-dispatch.out" \
                "core wasm retained-group dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            local retained_group_glow_active_count
            local retained_group_ribbon_active_count
            local retained_group_quad_active_count
            retained_group_glow_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-dispatch.out" "retained_glow_emitter_active_count")"
            retained_group_ribbon_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-dispatch.out" "retained_ribbon_trail_active_count")"
            retained_group_quad_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-dispatch.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_glow_active_count" || "$retained_group_glow_active_count" -lt 1 || -z "$retained_group_ribbon_active_count" || "$retained_group_ribbon_active_count" -lt 1 || -z "$retained_group_quad_active_count" || "$retained_group_quad_active_count" -lt 1 ]]; then
                mfx_fail "core wasm retained-group sample did not keep grouped retained instances alive"
            fi

            local code_retained_group_remove_dispatch
            code_retained_group_remove_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-remove.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":640,"y":360,"button":2}')"
            mfx_assert_eq "$code_retained_group_remove_dispatch" "200" "core wasm retained-group remove dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-remove.out" "\"ok\":true" "core wasm retained-group remove dispatch ok"

            local dispatch_group_remove_commands
            dispatch_group_remove_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-remove.out" "executed_group_remove_commands")"
            if [[ -z "$dispatch_group_remove_commands" || "$dispatch_group_remove_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group sample did not execute group-remove commands"
            fi

            local code_state_after_retained_group_remove
            code_state_after_retained_group_remove="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-remove.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_remove" "200" "core wasm state after retained-group remove status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-remove.out" \
                "$tmp_dir/state-after-wasm-retained-group-remove.out" \
                "core wasm retained-group remove diagnostics consistency" \
                "$tmp_dir/state-after-wasm-retained-group-dispatch.out" \
                "$platform"

            local retained_group_glow_active_count_after_remove
            local retained_group_ribbon_active_count_after_remove
            local retained_group_quad_active_count_after_remove
            retained_group_glow_active_count_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-remove.out" "retained_glow_emitter_active_count")"
            retained_group_ribbon_active_count_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-remove.out" "retained_ribbon_trail_active_count")"
            retained_group_quad_active_count_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-remove.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_glow_active_count_after_remove" || "$retained_group_glow_active_count_after_remove" -ne 0 || -z "$retained_group_ribbon_active_count_after_remove" || "$retained_group_ribbon_active_count_after_remove" -ne 0 || -z "$retained_group_quad_active_count_after_remove" || "$retained_group_quad_active_count_after_remove" -ne 0 ]]; then
                mfx_fail "core wasm retained-group sample did not clear grouped retained instances after remove dispatch"
            fi

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-retained-group-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after retained-group sample"
            fi

            if [[ -n "$retained_group_baseline_max_commands" && "$retained_group_baseline_max_commands" -lt 3 ]]; then
                local code_retained_group_policy_restore
                code_retained_group_policy_restore="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-restore.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d "{\"max_commands\":$retained_group_baseline_max_commands}")"
                mfx_assert_eq "$code_retained_group_policy_restore" "200" "core wasm retained-group policy restore status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-restore.out" "\"ok\":true" "core wasm retained-group policy restore ok"
                local code_state_after_retained_group_policy_restore
                code_state_after_retained_group_policy_restore="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-policy-restore.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
                mfx_assert_eq "$code_state_after_retained_group_policy_restore" "200" "core wasm retained-group policy restore state status"
                latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-policy-restore.out"
            fi
        fi

        local retained_group_alpha_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-retained-group-alpha/plugin.json"
        if [[ ! -f "$retained_group_alpha_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-retained-group-alpha >/dev/null
        fi
        if [[ -f "$retained_group_alpha_manifest_path" ]]; then
            local retained_group_alpha_baseline_max_commands
            retained_group_alpha_baseline_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$latest_state_snapshot_file" "configured_max_commands")"
            if [[ -z "$retained_group_alpha_baseline_max_commands" || "$retained_group_alpha_baseline_max_commands" -lt 4 ]]; then
                local code_retained_group_alpha_policy
                code_retained_group_alpha_policy="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-alpha.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d '{"max_commands":8}')"
                mfx_assert_eq "$code_retained_group_alpha_policy" "200" "core wasm retained-group-alpha policy status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-alpha.out" "\"ok\":true" "core wasm retained-group-alpha policy ok"
                local retained_group_alpha_policy_configured_max_commands
                local retained_group_alpha_policy_runtime_max_commands
                retained_group_alpha_policy_configured_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-policy-retained-group-alpha.out" "configured_max_commands")"
                retained_group_alpha_policy_runtime_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-policy-retained-group-alpha.out" "runtime_max_commands")"
                if [[ -z "$retained_group_alpha_policy_configured_max_commands" || "$retained_group_alpha_policy_configured_max_commands" -lt 8 ]]; then
                    mfx_fail "core wasm retained-group-alpha policy did not raise configured_max_commands"
                fi
                if [[ -z "$retained_group_alpha_policy_runtime_max_commands" || "$retained_group_alpha_policy_runtime_max_commands" -lt 8 ]]; then
                    mfx_fail "core wasm retained-group-alpha policy did not raise runtime_max_commands"
                fi
            fi

            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-retained-group-alpha-sample.out" \
                "$base_url" \
                "$token" \
                "$retained_group_alpha_manifest_path" \
                "core wasm load-manifest retained-group-alpha sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-retained-group-alpha.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch retained-group-alpha sample" \
                "$require_rendered_any"

            local dispatch_group_alpha_glow_commands
            local dispatch_group_alpha_ribbon_commands
            local dispatch_group_alpha_quad_commands
            local dispatch_group_alpha_presentation_commands
            dispatch_group_alpha_glow_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-alpha.out" "executed_glow_emitter_commands")"
            dispatch_group_alpha_ribbon_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-alpha.out" "executed_ribbon_trail_commands")"
            dispatch_group_alpha_quad_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-alpha.out" "executed_quad_field_commands")"
            dispatch_group_alpha_presentation_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-alpha.out" "executed_group_presentation_commands")"
            if [[ -z "$dispatch_group_alpha_glow_commands" || "$dispatch_group_alpha_glow_commands" -lt 1 || -z "$dispatch_group_alpha_ribbon_commands" || "$dispatch_group_alpha_ribbon_commands" -lt 1 || -z "$dispatch_group_alpha_quad_commands" || "$dispatch_group_alpha_quad_commands" -lt 1 || -z "$dispatch_group_alpha_presentation_commands" || "$dispatch_group_alpha_presentation_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-alpha sample did not execute grouped retained commands plus group presentation"
            fi

            local code_state_after_retained_group_alpha_dispatch
            code_state_after_retained_group_alpha_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-alpha-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_alpha_dispatch" "200" "core wasm state after retained-group-alpha dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-alpha.out" \
                "$tmp_dir/state-after-wasm-retained-group-alpha-dispatch.out" \
                "core wasm retained-group-alpha dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            local retained_group_alpha_glow_active_count
            local retained_group_alpha_ribbon_active_count
            local retained_group_alpha_quad_active_count
            retained_group_alpha_glow_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-alpha-dispatch.out" "retained_glow_emitter_active_count")"
            retained_group_alpha_ribbon_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-alpha-dispatch.out" "retained_ribbon_trail_active_count")"
            retained_group_alpha_quad_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-alpha-dispatch.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_alpha_glow_active_count" || "$retained_group_alpha_glow_active_count" -lt 1 || -z "$retained_group_alpha_ribbon_active_count" || "$retained_group_alpha_ribbon_active_count" -lt 1 || -z "$retained_group_alpha_quad_active_count" || "$retained_group_alpha_quad_active_count" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-alpha sample did not keep grouped retained instances alive"
            fi

            local code_retained_group_alpha_dim_dispatch
            code_retained_group_alpha_dim_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-alpha-dim.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":640,"y":360,"button":3}')"
            mfx_assert_eq "$code_retained_group_alpha_dim_dispatch" "200" "core wasm retained-group-alpha dim dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-alpha-dim.out" "\"ok\":true" "core wasm retained-group-alpha dim dispatch ok"

            local dispatch_group_alpha_dim_commands
            dispatch_group_alpha_dim_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-alpha-dim.out" "executed_group_presentation_commands")"
            if [[ -z "$dispatch_group_alpha_dim_commands" || "$dispatch_group_alpha_dim_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-alpha sample did not execute middle-click group-presentation commands"
            fi

            local code_state_after_retained_group_alpha_dim
            code_state_after_retained_group_alpha_dim="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-alpha-dim.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_alpha_dim" "200" "core wasm state after retained-group-alpha dim status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-alpha-dim.out" \
                "$tmp_dir/state-after-wasm-retained-group-alpha-dim.out" \
                "core wasm retained-group-alpha dim diagnostics consistency" \
                "$tmp_dir/state-after-wasm-retained-group-alpha-dispatch.out" \
                "$platform"

            local retained_group_alpha_glow_after_dim
            local retained_group_alpha_ribbon_after_dim
            local retained_group_alpha_quad_after_dim
            retained_group_alpha_glow_after_dim="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-alpha-dim.out" "retained_glow_emitter_active_count")"
            retained_group_alpha_ribbon_after_dim="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-alpha-dim.out" "retained_ribbon_trail_active_count")"
            retained_group_alpha_quad_after_dim="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-alpha-dim.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_alpha_glow_after_dim" || "$retained_group_alpha_glow_after_dim" -lt 1 || -z "$retained_group_alpha_ribbon_after_dim" || "$retained_group_alpha_ribbon_after_dim" -lt 1 || -z "$retained_group_alpha_quad_after_dim" || "$retained_group_alpha_quad_after_dim" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-alpha dim dispatch unexpectedly cleared grouped retained instances"
            fi

            local code_retained_group_alpha_remove_dispatch
            code_retained_group_alpha_remove_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-alpha-remove.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":640,"y":360,"button":2}')"
            mfx_assert_eq "$code_retained_group_alpha_remove_dispatch" "200" "core wasm retained-group-alpha remove dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-alpha-remove.out" "\"ok\":true" "core wasm retained-group-alpha remove dispatch ok"

            local dispatch_group_alpha_remove_commands
            dispatch_group_alpha_remove_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-alpha-remove.out" "executed_group_remove_commands")"
            if [[ -z "$dispatch_group_alpha_remove_commands" || "$dispatch_group_alpha_remove_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-alpha sample did not execute group-remove commands"
            fi

            local code_state_after_retained_group_alpha_remove
            code_state_after_retained_group_alpha_remove="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-alpha-remove.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_alpha_remove" "200" "core wasm state after retained-group-alpha remove status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-alpha-remove.out" \
                "$tmp_dir/state-after-wasm-retained-group-alpha-remove.out" \
                "core wasm retained-group-alpha remove diagnostics consistency" \
                "$tmp_dir/state-after-wasm-retained-group-alpha-dim.out" \
                "$platform"

            local retained_group_alpha_glow_after_remove
            local retained_group_alpha_ribbon_after_remove
            local retained_group_alpha_quad_after_remove
            retained_group_alpha_glow_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-alpha-remove.out" "retained_glow_emitter_active_count")"
            retained_group_alpha_ribbon_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-alpha-remove.out" "retained_ribbon_trail_active_count")"
            retained_group_alpha_quad_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-alpha-remove.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_alpha_glow_after_remove" || "$retained_group_alpha_glow_after_remove" -ne 0 || -z "$retained_group_alpha_ribbon_after_remove" || "$retained_group_alpha_ribbon_after_remove" -ne 0 || -z "$retained_group_alpha_quad_after_remove" || "$retained_group_alpha_quad_after_remove" -ne 0 ]]; then
                mfx_fail "core wasm retained-group-alpha sample did not clear grouped retained instances after remove_group"
            fi

            latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-alpha-remove.out"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-retained-group-alpha-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after retained-group-alpha sample"
            fi

            if [[ -n "$retained_group_alpha_baseline_max_commands" && "$retained_group_alpha_baseline_max_commands" -lt 4 ]]; then
                local code_retained_group_alpha_policy_restore
                code_retained_group_alpha_policy_restore="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-alpha-restore.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d "{\"max_commands\":$retained_group_alpha_baseline_max_commands}")"
                mfx_assert_eq "$code_retained_group_alpha_policy_restore" "200" "core wasm retained-group-alpha policy restore status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-alpha-restore.out" "\"ok\":true" "core wasm retained-group-alpha policy restore ok"
                local code_state_after_retained_group_alpha_policy_restore
                code_state_after_retained_group_alpha_policy_restore="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-alpha-policy-restore.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
                mfx_assert_eq "$code_state_after_retained_group_alpha_policy_restore" "200" "core wasm retained-group-alpha policy restore state status"
                latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-alpha-policy-restore.out"
            fi
        fi

        local retained_group_clip_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-retained-group-clip/plugin.json"
        if [[ ! -f "$retained_group_clip_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-retained-group-clip >/dev/null
        fi
        if [[ -f "$retained_group_clip_manifest_path" ]]; then
            local retained_group_clip_baseline_max_commands
            retained_group_clip_baseline_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$latest_state_snapshot_file" "configured_max_commands")"
            if [[ -z "$retained_group_clip_baseline_max_commands" || "$retained_group_clip_baseline_max_commands" -lt 4 ]]; then
                local code_retained_group_clip_policy
                code_retained_group_clip_policy="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-clip.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d '{"max_commands":8}')"
                mfx_assert_eq "$code_retained_group_clip_policy" "200" "core wasm retained-group-clip policy status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-clip.out" "\"ok\":true" "core wasm retained-group-clip policy ok"
                local retained_group_clip_policy_configured_max_commands
                local retained_group_clip_policy_runtime_max_commands
                retained_group_clip_policy_configured_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-policy-retained-group-clip.out" "configured_max_commands")"
                retained_group_clip_policy_runtime_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-policy-retained-group-clip.out" "runtime_max_commands")"
                if [[ -z "$retained_group_clip_policy_configured_max_commands" || "$retained_group_clip_policy_configured_max_commands" -lt 8 ]]; then
                    mfx_fail "core wasm retained-group-clip policy did not raise configured_max_commands"
                fi
                if [[ -z "$retained_group_clip_policy_runtime_max_commands" || "$retained_group_clip_policy_runtime_max_commands" -lt 8 ]]; then
                    mfx_fail "core wasm retained-group-clip policy did not raise runtime_max_commands"
                fi
            fi

            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-retained-group-clip-sample.out" \
                "$base_url" \
                "$token" \
                "$retained_group_clip_manifest_path" \
                "core wasm load-manifest retained-group-clip sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-retained-group-clip.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch retained-group-clip sample" \
                "$require_rendered_any"

            local dispatch_group_clip_glow_commands
            local dispatch_group_clip_ribbon_commands
            local dispatch_group_clip_quad_commands
            local dispatch_group_clip_clip_commands
            dispatch_group_clip_glow_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-clip.out" "executed_glow_emitter_commands")"
            dispatch_group_clip_ribbon_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-clip.out" "executed_ribbon_trail_commands")"
            dispatch_group_clip_quad_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-clip.out" "executed_quad_field_commands")"
            dispatch_group_clip_clip_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-clip.out" "executed_group_clip_rect_commands")"
            if [[ -z "$dispatch_group_clip_glow_commands" || "$dispatch_group_clip_glow_commands" -lt 1 || -z "$dispatch_group_clip_ribbon_commands" || "$dispatch_group_clip_ribbon_commands" -lt 1 || -z "$dispatch_group_clip_quad_commands" || "$dispatch_group_clip_quad_commands" -lt 1 || -z "$dispatch_group_clip_clip_commands" || "$dispatch_group_clip_clip_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-clip sample did not execute grouped retained commands plus group clip"
            fi

            local code_state_after_retained_group_clip_dispatch
            code_state_after_retained_group_clip_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-clip-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_clip_dispatch" "200" "core wasm state after retained-group-clip dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-clip.out" \
                "$tmp_dir/state-after-wasm-retained-group-clip-dispatch.out" \
                "core wasm retained-group-clip dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            local retained_group_clip_glow_active_count
            local retained_group_clip_ribbon_active_count
            local retained_group_clip_quad_active_count
            retained_group_clip_glow_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-clip-dispatch.out" "retained_glow_emitter_active_count")"
            retained_group_clip_ribbon_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-clip-dispatch.out" "retained_ribbon_trail_active_count")"
            retained_group_clip_quad_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-clip-dispatch.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_clip_glow_active_count" || "$retained_group_clip_glow_active_count" -lt 1 || -z "$retained_group_clip_ribbon_active_count" || "$retained_group_clip_ribbon_active_count" -lt 1 || -z "$retained_group_clip_quad_active_count" || "$retained_group_clip_quad_active_count" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-clip sample did not keep grouped retained instances alive"
            fi

            local code_retained_group_clip_update_dispatch
            code_retained_group_clip_update_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-clip-update.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":640,"y":360,"button":3}')"
            mfx_assert_eq "$code_retained_group_clip_update_dispatch" "200" "core wasm retained-group-clip update dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-clip-update.out" "\"ok\":true" "core wasm retained-group-clip update dispatch ok"

            local dispatch_group_clip_update_commands
            dispatch_group_clip_update_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-clip-update.out" "executed_group_clip_rect_commands")"
            if [[ -z "$dispatch_group_clip_update_commands" || "$dispatch_group_clip_update_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-clip sample did not execute middle-click group-clip commands"
            fi

            local code_state_after_retained_group_clip_update
            code_state_after_retained_group_clip_update="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-clip-update.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_clip_update" "200" "core wasm state after retained-group-clip update status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-clip-update.out" \
                "$tmp_dir/state-after-wasm-retained-group-clip-update.out" \
                "core wasm retained-group-clip update diagnostics consistency" \
                "$tmp_dir/state-after-wasm-retained-group-clip-dispatch.out" \
                "$platform"

            local retained_group_clip_glow_after_update
            local retained_group_clip_ribbon_after_update
            local retained_group_clip_quad_after_update
            retained_group_clip_glow_after_update="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-clip-update.out" "retained_glow_emitter_active_count")"
            retained_group_clip_ribbon_after_update="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-clip-update.out" "retained_ribbon_trail_active_count")"
            retained_group_clip_quad_after_update="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-clip-update.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_clip_glow_after_update" || "$retained_group_clip_glow_after_update" -lt 1 || -z "$retained_group_clip_ribbon_after_update" || "$retained_group_clip_ribbon_after_update" -lt 1 || -z "$retained_group_clip_quad_after_update" || "$retained_group_clip_quad_after_update" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-clip update dispatch unexpectedly cleared grouped retained instances"
            fi

            local code_retained_group_clip_remove_dispatch
            code_retained_group_clip_remove_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-clip-remove.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":640,"y":360,"button":2}')"
            mfx_assert_eq "$code_retained_group_clip_remove_dispatch" "200" "core wasm retained-group-clip remove dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-clip-remove.out" "\"ok\":true" "core wasm retained-group-clip remove dispatch ok"

            local dispatch_group_clip_remove_commands
            dispatch_group_clip_remove_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-clip-remove.out" "executed_group_remove_commands")"
            if [[ -z "$dispatch_group_clip_remove_commands" || "$dispatch_group_clip_remove_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-clip sample did not execute group-remove commands"
            fi

            local code_state_after_retained_group_clip_remove
            code_state_after_retained_group_clip_remove="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-clip-remove.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_clip_remove" "200" "core wasm state after retained-group-clip remove status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-clip-remove.out" \
                "$tmp_dir/state-after-wasm-retained-group-clip-remove.out" \
                "core wasm retained-group-clip remove diagnostics consistency" \
                "$tmp_dir/state-after-wasm-retained-group-clip-update.out" \
                "$platform"

            local retained_group_clip_glow_after_remove
            local retained_group_clip_ribbon_after_remove
            local retained_group_clip_quad_after_remove
            retained_group_clip_glow_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-clip-remove.out" "retained_glow_emitter_active_count")"
            retained_group_clip_ribbon_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-clip-remove.out" "retained_ribbon_trail_active_count")"
            retained_group_clip_quad_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-clip-remove.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_clip_glow_after_remove" || "$retained_group_clip_glow_after_remove" -ne 0 || -z "$retained_group_clip_ribbon_after_remove" || "$retained_group_clip_ribbon_after_remove" -ne 0 || -z "$retained_group_clip_quad_after_remove" || "$retained_group_clip_quad_after_remove" -ne 0 ]]; then
                mfx_fail "core wasm retained-group-clip sample did not clear grouped retained instances after remove_group"
            fi

            latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-clip-remove.out"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-retained-group-clip-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after retained-group-clip sample"
            fi

            if [[ -n "$retained_group_clip_baseline_max_commands" && "$retained_group_clip_baseline_max_commands" -lt 4 ]]; then
                local code_retained_group_clip_policy_restore
                code_retained_group_clip_policy_restore="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-clip-restore.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d "{\"max_commands\":$retained_group_clip_baseline_max_commands}")"
                mfx_assert_eq "$code_retained_group_clip_policy_restore" "200" "core wasm retained-group-clip policy restore status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-clip-restore.out" "\"ok\":true" "core wasm retained-group-clip policy restore ok"
                local code_state_after_retained_group_clip_policy_restore
                code_state_after_retained_group_clip_policy_restore="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-clip-policy-restore.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
                mfx_assert_eq "$code_state_after_retained_group_clip_policy_restore" "200" "core wasm retained-group-clip policy restore state status"
                latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-clip-policy-restore.out"
            fi
        fi

        local retained_group_layer_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-retained-group-layer/plugin.json"
        if [[ ! -f "$retained_group_layer_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-retained-group-layer >/dev/null
        fi
        if [[ -f "$retained_group_layer_manifest_path" ]]; then
            local retained_group_layer_baseline_max_commands
            retained_group_layer_baseline_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$latest_state_snapshot_file" "configured_max_commands")"
            if [[ -z "$retained_group_layer_baseline_max_commands" || "$retained_group_layer_baseline_max_commands" -lt 4 ]]; then
                local code_retained_group_layer_policy
                code_retained_group_layer_policy="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-layer.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d '{"max_commands":8}')"
                mfx_assert_eq "$code_retained_group_layer_policy" "200" "core wasm retained-group-layer policy status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-layer.out" "\"ok\":true" "core wasm retained-group-layer policy ok"
                local retained_group_layer_policy_configured_max_commands
                local retained_group_layer_policy_runtime_max_commands
                retained_group_layer_policy_configured_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-policy-retained-group-layer.out" "configured_max_commands")"
                retained_group_layer_policy_runtime_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-policy-retained-group-layer.out" "runtime_max_commands")"
                if [[ -z "$retained_group_layer_policy_configured_max_commands" || "$retained_group_layer_policy_configured_max_commands" -lt 8 ]]; then
                    mfx_fail "core wasm retained-group-layer policy did not raise configured_max_commands"
                fi
                if [[ -z "$retained_group_layer_policy_runtime_max_commands" || "$retained_group_layer_policy_runtime_max_commands" -lt 8 ]]; then
                    mfx_fail "core wasm retained-group-layer policy did not raise runtime_max_commands"
                fi
            fi

            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-retained-group-layer-sample.out" \
                "$base_url" \
                "$token" \
                "$retained_group_layer_manifest_path" \
                "core wasm load-manifest retained-group-layer sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-retained-group-layer.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch retained-group-layer sample" \
                "$require_rendered_any"

            local dispatch_group_layer_glow_commands
            local dispatch_group_layer_ribbon_commands
            local dispatch_group_layer_quad_commands
            local dispatch_group_layer_layer_commands
            dispatch_group_layer_glow_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-layer.out" "executed_glow_emitter_commands")"
            dispatch_group_layer_ribbon_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-layer.out" "executed_ribbon_trail_commands")"
            dispatch_group_layer_quad_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-layer.out" "executed_quad_field_commands")"
            dispatch_group_layer_layer_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-layer.out" "executed_group_layer_commands")"
            if [[ -z "$dispatch_group_layer_glow_commands" || "$dispatch_group_layer_glow_commands" -lt 1 || -z "$dispatch_group_layer_ribbon_commands" || "$dispatch_group_layer_ribbon_commands" -lt 1 || -z "$dispatch_group_layer_quad_commands" || "$dispatch_group_layer_quad_commands" -lt 1 || -z "$dispatch_group_layer_layer_commands" || "$dispatch_group_layer_layer_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-layer sample did not execute grouped retained commands plus group layer"
            fi

            local code_state_after_retained_group_layer_dispatch
            code_state_after_retained_group_layer_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-layer-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_layer_dispatch" "200" "core wasm state after retained-group-layer dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-layer.out" \
                "$tmp_dir/state-after-wasm-retained-group-layer-dispatch.out" \
                "core wasm retained-group-layer dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            local retained_group_layer_glow_active_count
            local retained_group_layer_ribbon_active_count
            local retained_group_layer_quad_active_count
            retained_group_layer_glow_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-layer-dispatch.out" "retained_glow_emitter_active_count")"
            retained_group_layer_ribbon_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-layer-dispatch.out" "retained_ribbon_trail_active_count")"
            retained_group_layer_quad_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-layer-dispatch.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_layer_glow_active_count" || "$retained_group_layer_glow_active_count" -lt 1 || -z "$retained_group_layer_ribbon_active_count" || "$retained_group_layer_ribbon_active_count" -lt 1 || -z "$retained_group_layer_quad_active_count" || "$retained_group_layer_quad_active_count" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-layer sample did not keep grouped retained instances alive"
            fi

            local code_retained_group_layer_update_dispatch
            code_retained_group_layer_update_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-layer-update.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":640,"y":360,"button":3}')"
            mfx_assert_eq "$code_retained_group_layer_update_dispatch" "200" "core wasm retained-group-layer update dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-layer-update.out" "\"ok\":true" "core wasm retained-group-layer update dispatch ok"

            local dispatch_group_layer_update_commands
            dispatch_group_layer_update_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-layer-update.out" "executed_group_layer_commands")"
            if [[ -z "$dispatch_group_layer_update_commands" || "$dispatch_group_layer_update_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-layer sample did not execute middle-click group-layer commands"
            fi

            local code_state_after_retained_group_layer_update
            code_state_after_retained_group_layer_update="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-layer-update.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_layer_update" "200" "core wasm state after retained-group-layer update status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-layer-update.out" \
                "$tmp_dir/state-after-wasm-retained-group-layer-update.out" \
                "core wasm retained-group-layer update diagnostics consistency" \
                "$tmp_dir/state-after-wasm-retained-group-layer-dispatch.out" \
                "$platform"

            local retained_group_layer_glow_after_update
            local retained_group_layer_ribbon_after_update
            local retained_group_layer_quad_after_update
            retained_group_layer_glow_after_update="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-layer-update.out" "retained_glow_emitter_active_count")"
            retained_group_layer_ribbon_after_update="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-layer-update.out" "retained_ribbon_trail_active_count")"
            retained_group_layer_quad_after_update="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-layer-update.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_layer_glow_after_update" || "$retained_group_layer_glow_after_update" -lt 1 || -z "$retained_group_layer_ribbon_after_update" || "$retained_group_layer_ribbon_after_update" -lt 1 || -z "$retained_group_layer_quad_after_update" || "$retained_group_layer_quad_after_update" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-layer update dispatch unexpectedly cleared grouped retained instances"
            fi

            local code_retained_group_layer_remove_dispatch
            code_retained_group_layer_remove_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-layer-remove.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":640,"y":360,"button":2}')"
            mfx_assert_eq "$code_retained_group_layer_remove_dispatch" "200" "core wasm retained-group-layer remove dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-layer-remove.out" "\"ok\":true" "core wasm retained-group-layer remove dispatch ok"

            local dispatch_group_layer_remove_commands
            dispatch_group_layer_remove_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-layer-remove.out" "executed_group_remove_commands")"
            if [[ -z "$dispatch_group_layer_remove_commands" || "$dispatch_group_layer_remove_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-layer sample did not execute group-remove commands"
            fi

            local code_state_after_retained_group_layer_remove
            code_state_after_retained_group_layer_remove="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-layer-remove.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_layer_remove" "200" "core wasm state after retained-group-layer remove status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-layer-remove.out" \
                "$tmp_dir/state-after-wasm-retained-group-layer-remove.out" \
                "core wasm retained-group-layer remove diagnostics consistency" \
                "$tmp_dir/state-after-wasm-retained-group-layer-update.out" \
                "$platform"

            local retained_group_layer_glow_after_remove
            local retained_group_layer_ribbon_after_remove
            local retained_group_layer_quad_after_remove
            retained_group_layer_glow_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-layer-remove.out" "retained_glow_emitter_active_count")"
            retained_group_layer_ribbon_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-layer-remove.out" "retained_ribbon_trail_active_count")"
            retained_group_layer_quad_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-layer-remove.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_layer_glow_after_remove" || "$retained_group_layer_glow_after_remove" -ne 0 || -z "$retained_group_layer_ribbon_after_remove" || "$retained_group_layer_ribbon_after_remove" -ne 0 || -z "$retained_group_layer_quad_after_remove" || "$retained_group_layer_quad_after_remove" -ne 0 ]]; then
                mfx_fail "core wasm retained-group-layer sample did not clear grouped retained instances after remove_group"
            fi

            latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-layer-remove.out"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-retained-group-layer-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after retained-group-layer sample"
            fi

            if [[ -n "$retained_group_layer_baseline_max_commands" && "$retained_group_layer_baseline_max_commands" -lt 4 ]]; then
                local code_retained_group_layer_policy_restore
                code_retained_group_layer_policy_restore="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-layer-restore.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d "{\"max_commands\":$retained_group_layer_baseline_max_commands}")"
                mfx_assert_eq "$code_retained_group_layer_policy_restore" "200" "core wasm retained-group-layer policy restore status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-layer-restore.out" "\"ok\":true" "core wasm retained-group-layer policy restore ok"
                local code_state_after_retained_group_layer_policy_restore
                code_state_after_retained_group_layer_policy_restore="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-layer-policy-restore.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
                mfx_assert_eq "$code_state_after_retained_group_layer_policy_restore" "200" "core wasm retained-group-layer policy restore state status"
                latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-layer-policy-restore.out"
            fi
        fi

        local retained_group_mask_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-retained-group-mask/plugin.json"
        if [[ ! -f "$retained_group_mask_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-retained-group-mask >/dev/null
        fi
        if [[ -f "$retained_group_mask_manifest_path" ]]; then
            local retained_group_mask_baseline_max_commands
            retained_group_mask_baseline_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$latest_state_snapshot_file" "configured_max_commands")"
            if [[ -z "$retained_group_mask_baseline_max_commands" || "$retained_group_mask_baseline_max_commands" -lt 4 ]]; then
                local code_retained_group_mask_policy
                code_retained_group_mask_policy="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-mask.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d '{"max_commands":8}')"
                mfx_assert_eq "$code_retained_group_mask_policy" "200" "core wasm retained-group-mask policy status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-mask.out" "\"ok\":true" "core wasm retained-group-mask policy ok"
                local retained_group_mask_policy_configured_max_commands
                local retained_group_mask_policy_runtime_max_commands
                retained_group_mask_policy_configured_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-policy-retained-group-mask.out" "configured_max_commands")"
                retained_group_mask_policy_runtime_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-policy-retained-group-mask.out" "runtime_max_commands")"
                if [[ -z "$retained_group_mask_policy_configured_max_commands" || "$retained_group_mask_policy_configured_max_commands" -lt 8 ]]; then
                    mfx_fail "core wasm retained-group-mask policy did not raise configured_max_commands"
                fi
                if [[ -z "$retained_group_mask_policy_runtime_max_commands" || "$retained_group_mask_policy_runtime_max_commands" -lt 8 ]]; then
                    mfx_fail "core wasm retained-group-mask policy did not raise runtime_max_commands"
                fi
            fi

            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-retained-group-mask-sample.out" \
                "$base_url" \
                "$token" \
                "$retained_group_mask_manifest_path" \
                "core wasm load-manifest retained-group-mask sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-retained-group-mask.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch retained-group-mask sample" \
                "$require_rendered_any"

            local dispatch_group_mask_glow_commands
            local dispatch_group_mask_ribbon_commands
            local dispatch_group_mask_quad_commands
            local dispatch_group_mask_clip_commands
            dispatch_group_mask_glow_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-mask.out" "executed_glow_emitter_commands")"
            dispatch_group_mask_ribbon_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-mask.out" "executed_ribbon_trail_commands")"
            dispatch_group_mask_quad_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-mask.out" "executed_quad_field_commands")"
            dispatch_group_mask_clip_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-mask.out" "executed_group_clip_rect_commands")"
            if [[ -z "$dispatch_group_mask_glow_commands" || "$dispatch_group_mask_glow_commands" -lt 1 || -z "$dispatch_group_mask_ribbon_commands" || "$dispatch_group_mask_ribbon_commands" -lt 1 || -z "$dispatch_group_mask_quad_commands" || "$dispatch_group_mask_quad_commands" -lt 1 || -z "$dispatch_group_mask_clip_commands" || "$dispatch_group_mask_clip_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-mask sample did not execute grouped retained commands plus group clip mask"
            fi

            local code_state_after_retained_group_mask_dispatch
            code_state_after_retained_group_mask_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-mask-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_mask_dispatch" "200" "core wasm state after retained-group-mask dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-mask.out" \
                "$tmp_dir/state-after-wasm-retained-group-mask-dispatch.out" \
                "core wasm retained-group-mask dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            local retained_group_mask_glow_active_count
            local retained_group_mask_ribbon_active_count
            local retained_group_mask_quad_active_count
            retained_group_mask_glow_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-mask-dispatch.out" "retained_glow_emitter_active_count")"
            retained_group_mask_ribbon_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-mask-dispatch.out" "retained_ribbon_trail_active_count")"
            retained_group_mask_quad_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-mask-dispatch.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_mask_glow_active_count" || "$retained_group_mask_glow_active_count" -lt 1 || -z "$retained_group_mask_ribbon_active_count" || "$retained_group_mask_ribbon_active_count" -lt 1 || -z "$retained_group_mask_quad_active_count" || "$retained_group_mask_quad_active_count" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-mask sample did not keep grouped retained instances alive"
            fi

            local code_retained_group_mask_update_dispatch
            code_retained_group_mask_update_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-mask-update.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":640,"y":360,"button":3}')"
            mfx_assert_eq "$code_retained_group_mask_update_dispatch" "200" "core wasm retained-group-mask update dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-mask-update.out" "\"ok\":true" "core wasm retained-group-mask update dispatch ok"

            local dispatch_group_mask_update_commands
            dispatch_group_mask_update_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-mask-update.out" "executed_group_clip_rect_commands")"
            if [[ -z "$dispatch_group_mask_update_commands" || "$dispatch_group_mask_update_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-mask sample did not execute middle-click group-clip commands"
            fi

            local code_state_after_retained_group_mask_update
            code_state_after_retained_group_mask_update="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-mask-update.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_mask_update" "200" "core wasm state after retained-group-mask update status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-mask-update.out" \
                "$tmp_dir/state-after-wasm-retained-group-mask-update.out" \
                "core wasm retained-group-mask update diagnostics consistency" \
                "$tmp_dir/state-after-wasm-retained-group-mask-dispatch.out" \
                "$platform"

            local retained_group_mask_glow_after_update
            local retained_group_mask_ribbon_after_update
            local retained_group_mask_quad_after_update
            retained_group_mask_glow_after_update="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-mask-update.out" "retained_glow_emitter_active_count")"
            retained_group_mask_ribbon_after_update="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-mask-update.out" "retained_ribbon_trail_active_count")"
            retained_group_mask_quad_after_update="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-mask-update.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_mask_glow_after_update" || "$retained_group_mask_glow_after_update" -lt 1 || -z "$retained_group_mask_ribbon_after_update" || "$retained_group_mask_ribbon_after_update" -lt 1 || -z "$retained_group_mask_quad_after_update" || "$retained_group_mask_quad_after_update" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-mask update dispatch unexpectedly cleared grouped retained instances"
            fi

            local code_retained_group_mask_remove_dispatch
            code_retained_group_mask_remove_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-mask-remove.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":640,"y":360,"button":2}')"
            mfx_assert_eq "$code_retained_group_mask_remove_dispatch" "200" "core wasm retained-group-mask remove dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-mask-remove.out" "\"ok\":true" "core wasm retained-group-mask remove dispatch ok"

            local dispatch_group_mask_remove_commands
            dispatch_group_mask_remove_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-mask-remove.out" "executed_group_remove_commands")"
            if [[ -z "$dispatch_group_mask_remove_commands" || "$dispatch_group_mask_remove_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-mask sample did not execute group-remove commands"
            fi

            local code_state_after_retained_group_mask_remove
            code_state_after_retained_group_mask_remove="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-mask-remove.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_mask_remove" "200" "core wasm state after retained-group-mask remove status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-mask-remove.out" \
                "$tmp_dir/state-after-wasm-retained-group-mask-remove.out" \
                "core wasm retained-group-mask remove diagnostics consistency" \
                "$tmp_dir/state-after-wasm-retained-group-mask-update.out" \
                "$platform"

            local retained_group_mask_glow_after_remove
            local retained_group_mask_ribbon_after_remove
            local retained_group_mask_quad_after_remove
            retained_group_mask_glow_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-mask-remove.out" "retained_glow_emitter_active_count")"
            retained_group_mask_ribbon_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-mask-remove.out" "retained_ribbon_trail_active_count")"
            retained_group_mask_quad_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-mask-remove.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_mask_glow_after_remove" || "$retained_group_mask_glow_after_remove" -ne 0 || -z "$retained_group_mask_ribbon_after_remove" || "$retained_group_mask_ribbon_after_remove" -ne 0 || -z "$retained_group_mask_quad_after_remove" || "$retained_group_mask_quad_after_remove" -ne 0 ]]; then
                mfx_fail "core wasm retained-group-mask sample did not clear grouped retained instances after remove_group"
            fi

            latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-mask-remove.out"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-retained-group-mask-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after retained-group-mask sample"
            fi

            if [[ -n "$retained_group_mask_baseline_max_commands" && "$retained_group_mask_baseline_max_commands" -lt 4 ]]; then
                local code_retained_group_mask_policy_restore
                code_retained_group_mask_policy_restore="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-mask-restore.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d "{\"max_commands\":$retained_group_mask_baseline_max_commands}")"
                mfx_assert_eq "$code_retained_group_mask_policy_restore" "200" "core wasm retained-group-mask policy restore status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-mask-restore.out" "\"ok\":true" "core wasm retained-group-mask policy restore ok"
                local code_state_after_retained_group_mask_policy_restore
                code_state_after_retained_group_mask_policy_restore="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-mask-policy-restore.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
                mfx_assert_eq "$code_state_after_retained_group_mask_policy_restore" "200" "core wasm retained-group-mask policy restore state status"
                latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-mask-policy-restore.out"
            fi
        fi

        local retained_group_local_origin_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-retained-group-local-origin/plugin.json"
        if [[ ! -f "$retained_group_local_origin_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-retained-group-local-origin >/dev/null
        fi
        if [[ -f "$retained_group_local_origin_manifest_path" ]]; then
            local retained_group_local_origin_baseline_max_commands
            retained_group_local_origin_baseline_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$latest_state_snapshot_file" "configured_max_commands")"
            if [[ -z "$retained_group_local_origin_baseline_max_commands" || "$retained_group_local_origin_baseline_max_commands" -lt 7 ]]; then
                local code_retained_group_local_origin_policy
                code_retained_group_local_origin_policy="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-local-origin.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d '{"max_commands":10}')"
                mfx_assert_eq "$code_retained_group_local_origin_policy" "200" "core wasm retained-group-local-origin policy status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-local-origin.out" "\"ok\":true" "core wasm retained-group-local-origin policy ok"
            fi

            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-retained-group-local-origin-sample.out" \
                "$base_url" \
                "$token" \
                "$retained_group_local_origin_manifest_path" \
                "core wasm load-manifest retained-group-local-origin sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-retained-group-local-origin.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch retained-group-local-origin sample" \
                "$require_rendered_any"

            local dispatch_group_local_origin_glow_commands
            local dispatch_group_local_origin_sprite_commands
            local dispatch_group_local_origin_particle_commands
            local dispatch_group_local_origin_ribbon_commands
            local dispatch_group_local_origin_quad_commands
            local dispatch_group_local_origin_origin_commands
            local dispatch_group_local_origin_transform_commands
            dispatch_group_local_origin_glow_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-local-origin.out" "executed_glow_emitter_commands")"
            dispatch_group_local_origin_sprite_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-local-origin.out" "executed_sprite_emitter_commands")"
            dispatch_group_local_origin_particle_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-local-origin.out" "executed_particle_emitter_commands")"
            dispatch_group_local_origin_ribbon_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-local-origin.out" "executed_ribbon_trail_commands")"
            dispatch_group_local_origin_quad_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-local-origin.out" "executed_quad_field_commands")"
            dispatch_group_local_origin_origin_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-local-origin.out" "executed_group_local_origin_commands")"
            dispatch_group_local_origin_transform_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-local-origin.out" "executed_group_transform_commands")"
            if [[ -z "$dispatch_group_local_origin_glow_commands" || "$dispatch_group_local_origin_glow_commands" -lt 1 || -z "$dispatch_group_local_origin_sprite_commands" || "$dispatch_group_local_origin_sprite_commands" -lt 1 || -z "$dispatch_group_local_origin_particle_commands" || "$dispatch_group_local_origin_particle_commands" -lt 1 || -z "$dispatch_group_local_origin_ribbon_commands" || "$dispatch_group_local_origin_ribbon_commands" -lt 1 || -z "$dispatch_group_local_origin_quad_commands" || "$dispatch_group_local_origin_quad_commands" -lt 1 || -z "$dispatch_group_local_origin_origin_commands" || "$dispatch_group_local_origin_origin_commands" -lt 1 || -z "$dispatch_group_local_origin_transform_commands" || "$dispatch_group_local_origin_transform_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-local-origin sample did not execute grouped retained commands plus local-origin setup"
            fi

            local code_state_after_retained_group_local_origin_dispatch
            code_state_after_retained_group_local_origin_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-local-origin-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_local_origin_dispatch" "200" "core wasm state after retained-group-local-origin dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-local-origin.out" \
                "$tmp_dir/state-after-wasm-retained-group-local-origin-dispatch.out" \
                "core wasm retained-group-local-origin dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            local retained_group_local_origin_glow_active_count
            local retained_group_local_origin_sprite_active_count
            local retained_group_local_origin_particle_active_count
            local retained_group_local_origin_ribbon_active_count
            local retained_group_local_origin_quad_active_count
            retained_group_local_origin_glow_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-local-origin-dispatch.out" "retained_glow_emitter_active_count")"
            retained_group_local_origin_sprite_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-local-origin-dispatch.out" "retained_sprite_emitter_active_count")"
            retained_group_local_origin_particle_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-local-origin-dispatch.out" "retained_particle_emitter_active_count")"
            retained_group_local_origin_ribbon_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-local-origin-dispatch.out" "retained_ribbon_trail_active_count")"
            retained_group_local_origin_quad_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-local-origin-dispatch.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_local_origin_glow_active_count" || "$retained_group_local_origin_glow_active_count" -lt 1 || -z "$retained_group_local_origin_sprite_active_count" || "$retained_group_local_origin_sprite_active_count" -lt 1 || -z "$retained_group_local_origin_particle_active_count" || "$retained_group_local_origin_particle_active_count" -lt 1 || -z "$retained_group_local_origin_ribbon_active_count" || "$retained_group_local_origin_ribbon_active_count" -lt 1 || -z "$retained_group_local_origin_quad_active_count" || "$retained_group_local_origin_quad_active_count" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-local-origin sample did not keep grouped retained instances alive"
            fi

            local code_retained_group_local_origin_update_dispatch
            code_retained_group_local_origin_update_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-local-origin-update.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":712,"y":388,"button":3}')"
            mfx_assert_eq "$code_retained_group_local_origin_update_dispatch" "200" "core wasm retained-group-local-origin update dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-local-origin-update.out" "\"ok\":true" "core wasm retained-group-local-origin update dispatch ok"

            local dispatch_group_local_origin_update_commands
            dispatch_group_local_origin_update_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-local-origin-update.out" "executed_group_local_origin_commands")"
            if [[ -z "$dispatch_group_local_origin_update_commands" || "$dispatch_group_local_origin_update_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-local-origin sample did not execute middle-click group-local-origin commands"
            fi

            local code_retained_group_local_origin_remove_dispatch
            code_retained_group_local_origin_remove_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-local-origin-remove.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":712,"y":388,"button":2}')"
            mfx_assert_eq "$code_retained_group_local_origin_remove_dispatch" "200" "core wasm retained-group-local-origin remove dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-local-origin-remove.out" "\"ok\":true" "core wasm retained-group-local-origin remove dispatch ok"

            local dispatch_group_local_origin_remove_commands
            dispatch_group_local_origin_remove_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-local-origin-remove.out" "executed_group_remove_commands")"
            if [[ -z "$dispatch_group_local_origin_remove_commands" || "$dispatch_group_local_origin_remove_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-local-origin sample did not execute group-remove commands"
            fi

            local code_state_after_retained_group_local_origin_remove
            code_state_after_retained_group_local_origin_remove="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-local-origin-remove.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_local_origin_remove" "200" "core wasm state after retained-group-local-origin remove status"
            local retained_group_local_origin_glow_after_remove
            local retained_group_local_origin_sprite_after_remove
            local retained_group_local_origin_particle_after_remove
            local retained_group_local_origin_ribbon_after_remove
            local retained_group_local_origin_quad_after_remove
            retained_group_local_origin_glow_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-local-origin-remove.out" "retained_glow_emitter_active_count")"
            retained_group_local_origin_sprite_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-local-origin-remove.out" "retained_sprite_emitter_active_count")"
            retained_group_local_origin_particle_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-local-origin-remove.out" "retained_particle_emitter_active_count")"
            retained_group_local_origin_ribbon_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-local-origin-remove.out" "retained_ribbon_trail_active_count")"
            retained_group_local_origin_quad_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-local-origin-remove.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_local_origin_glow_after_remove" || "$retained_group_local_origin_glow_after_remove" -ne 0 || -z "$retained_group_local_origin_sprite_after_remove" || "$retained_group_local_origin_sprite_after_remove" -ne 0 || -z "$retained_group_local_origin_particle_after_remove" || "$retained_group_local_origin_particle_after_remove" -ne 0 || -z "$retained_group_local_origin_ribbon_after_remove" || "$retained_group_local_origin_ribbon_after_remove" -ne 0 || -z "$retained_group_local_origin_quad_after_remove" || "$retained_group_local_origin_quad_after_remove" -ne 0 ]]; then
                mfx_fail "core wasm retained-group-local-origin sample did not clear grouped retained instances after remove_group"
            fi

            latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-local-origin-remove.out"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-retained-group-local-origin-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after retained-group-local-origin sample"
            fi

            if [[ -n "$retained_group_local_origin_baseline_max_commands" && "$retained_group_local_origin_baseline_max_commands" -lt 7 ]]; then
                local code_retained_group_local_origin_policy_restore
                code_retained_group_local_origin_policy_restore="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-local-origin-restore.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d "{\"max_commands\":$retained_group_local_origin_baseline_max_commands}")"
                mfx_assert_eq "$code_retained_group_local_origin_policy_restore" "200" "core wasm retained-group-local-origin policy restore status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-local-origin-restore.out" "\"ok\":true" "core wasm retained-group-local-origin policy restore ok"
                local code_state_after_retained_group_local_origin_policy_restore
                code_state_after_retained_group_local_origin_policy_restore="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-local-origin-policy-restore.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
                mfx_assert_eq "$code_state_after_retained_group_local_origin_policy_restore" "200" "core wasm retained-group-local-origin policy restore state status"
                latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-local-origin-policy-restore.out"
            fi
        fi

        local retained_group_material_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-retained-group-material/plugin.json"
        if [[ ! -f "$retained_group_material_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-retained-group-material >/dev/null
        fi
        if [[ -f "$retained_group_material_manifest_path" ]]; then
            local retained_group_material_baseline_max_commands
            retained_group_material_baseline_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$latest_state_snapshot_file" "configured_max_commands")"
            if [[ -z "$retained_group_material_baseline_max_commands" || "$retained_group_material_baseline_max_commands" -lt 5 ]]; then
                local code_retained_group_material_policy
                code_retained_group_material_policy="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-material.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d '{"max_commands":8}')"
                mfx_assert_eq "$code_retained_group_material_policy" "200" "core wasm retained-group-material policy status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-material.out" "\"ok\":true" "core wasm retained-group-material policy ok"
            fi

            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-retained-group-material-sample.out" \
                "$base_url" \
                "$token" \
                "$retained_group_material_manifest_path" \
                "core wasm load-manifest retained-group-material sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-retained-group-material.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch retained-group-material sample" \
                "$require_rendered_any"

            local dispatch_group_material_glow_commands
            local dispatch_group_material_sprite_commands
            local dispatch_group_material_particle_commands
            local dispatch_group_material_ribbon_commands
            local dispatch_group_material_quad_commands
            dispatch_group_material_glow_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-material.out" "executed_glow_emitter_commands")"
            dispatch_group_material_sprite_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-material.out" "executed_sprite_emitter_commands")"
            dispatch_group_material_particle_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-material.out" "executed_particle_emitter_commands")"
            dispatch_group_material_ribbon_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-material.out" "executed_ribbon_trail_commands")"
            dispatch_group_material_quad_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-material.out" "executed_quad_field_commands")"
            if [[ -z "$dispatch_group_material_glow_commands" || "$dispatch_group_material_glow_commands" -lt 1 || -z "$dispatch_group_material_sprite_commands" || "$dispatch_group_material_sprite_commands" -lt 1 || -z "$dispatch_group_material_particle_commands" || "$dispatch_group_material_particle_commands" -lt 1 || -z "$dispatch_group_material_ribbon_commands" || "$dispatch_group_material_ribbon_commands" -lt 1 || -z "$dispatch_group_material_quad_commands" || "$dispatch_group_material_quad_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-material sample did not execute grouped retained commands"
            fi

            local code_state_after_retained_group_material_dispatch
            code_state_after_retained_group_material_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-material-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_material_dispatch" "200" "core wasm state after retained-group-material dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-material.out" \
                "$tmp_dir/state-after-wasm-retained-group-material-dispatch.out" \
                "core wasm retained-group-material dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            local retained_group_material_glow_active_count
            local retained_group_material_sprite_active_count
            local retained_group_material_particle_active_count
            local retained_group_material_ribbon_active_count
            local retained_group_material_quad_active_count
            retained_group_material_glow_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-material-dispatch.out" "retained_glow_emitter_active_count")"
            retained_group_material_sprite_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-material-dispatch.out" "retained_sprite_emitter_active_count")"
            retained_group_material_particle_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-material-dispatch.out" "retained_particle_emitter_active_count")"
            retained_group_material_ribbon_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-material-dispatch.out" "retained_ribbon_trail_active_count")"
            retained_group_material_quad_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-material-dispatch.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_material_glow_active_count" || "$retained_group_material_glow_active_count" -lt 1 || -z "$retained_group_material_sprite_active_count" || "$retained_group_material_sprite_active_count" -lt 1 || -z "$retained_group_material_particle_active_count" || "$retained_group_material_particle_active_count" -lt 1 || -z "$retained_group_material_ribbon_active_count" || "$retained_group_material_ribbon_active_count" -lt 1 || -z "$retained_group_material_quad_active_count" || "$retained_group_material_quad_active_count" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-material sample did not keep grouped retained instances alive"
            fi

            local code_retained_group_material_update_dispatch
            code_retained_group_material_update_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-material-update.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":712,"y":388,"button":3}')"
            mfx_assert_eq "$code_retained_group_material_update_dispatch" "200" "core wasm retained-group-material update dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-material-update.out" "\"ok\":true" "core wasm retained-group-material update dispatch ok"

            local dispatch_group_material_update_commands
            dispatch_group_material_update_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-material-update.out" "executed_group_material_commands")"
            if [[ -z "$dispatch_group_material_update_commands" || "$dispatch_group_material_update_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-material sample did not execute middle-click group-material commands"
            fi

            local code_retained_group_material_remove_dispatch
            code_retained_group_material_remove_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-material-remove.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":712,"y":388,"button":2}')"
            mfx_assert_eq "$code_retained_group_material_remove_dispatch" "200" "core wasm retained-group-material remove dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-material-remove.out" "\"ok\":true" "core wasm retained-group-material remove dispatch ok"

            local dispatch_group_material_remove_commands
            dispatch_group_material_remove_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-material-remove.out" "executed_group_remove_commands")"
            if [[ -z "$dispatch_group_material_remove_commands" || "$dispatch_group_material_remove_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-material sample did not execute group-remove commands"
            fi

            local code_state_after_retained_group_material_remove
            code_state_after_retained_group_material_remove="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-material-remove.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_material_remove" "200" "core wasm state after retained-group-material remove status"
            local retained_group_material_glow_after_remove
            local retained_group_material_sprite_after_remove
            local retained_group_material_particle_after_remove
            local retained_group_material_ribbon_after_remove
            local retained_group_material_quad_after_remove
            retained_group_material_glow_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-material-remove.out" "retained_glow_emitter_active_count")"
            retained_group_material_sprite_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-material-remove.out" "retained_sprite_emitter_active_count")"
            retained_group_material_particle_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-material-remove.out" "retained_particle_emitter_active_count")"
            retained_group_material_ribbon_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-material-remove.out" "retained_ribbon_trail_active_count")"
            retained_group_material_quad_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-material-remove.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_material_glow_after_remove" || "$retained_group_material_glow_after_remove" -ne 0 || -z "$retained_group_material_sprite_after_remove" || "$retained_group_material_sprite_after_remove" -ne 0 || -z "$retained_group_material_particle_after_remove" || "$retained_group_material_particle_after_remove" -ne 0 || -z "$retained_group_material_ribbon_after_remove" || "$retained_group_material_ribbon_after_remove" -ne 0 || -z "$retained_group_material_quad_after_remove" || "$retained_group_material_quad_after_remove" -ne 0 ]]; then
                mfx_fail "core wasm retained-group-material sample did not clear grouped retained instances after remove_group"
            fi

            latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-material-remove.out"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-retained-group-material-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after retained-group-material sample"
            fi

            if [[ -n "$retained_group_material_baseline_max_commands" && "$retained_group_material_baseline_max_commands" -lt 5 ]]; then
                local code_retained_group_material_policy_restore
                code_retained_group_material_policy_restore="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-material-restore.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d "{\"max_commands\":$retained_group_material_baseline_max_commands}")"
                mfx_assert_eq "$code_retained_group_material_policy_restore" "200" "core wasm retained-group-material policy restore status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-material-restore.out" "\"ok\":true" "core wasm retained-group-material policy restore ok"
                local code_state_after_retained_group_material_policy_restore
                code_state_after_retained_group_material_policy_restore="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-material-policy-restore.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
                mfx_assert_eq "$code_state_after_retained_group_material_policy_restore" "200" "core wasm retained-group-material policy restore state status"
                latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-material-policy-restore.out"
            fi
        fi

        local retained_group_pass_manifest_path="$repo_root/examples/wasm-plugin-template/dist/samples/click-retained-group-pass/plugin.json"
        if [[ ! -f "$retained_group_pass_manifest_path" ]] && command -v npm >/dev/null 2>&1; then
            npm --prefix "$repo_root/examples/wasm-plugin-template" run build:sample -- --sample click-retained-group-pass >/dev/null
        fi
        if [[ -f "$retained_group_pass_manifest_path" ]]; then
            local retained_group_pass_baseline_max_commands
            retained_group_pass_baseline_max_commands="$(_mfx_core_http_wasm_parse_uint_field "$latest_state_snapshot_file" "configured_max_commands")"
            if [[ -z "$retained_group_pass_baseline_max_commands" || "$retained_group_pass_baseline_max_commands" -lt 5 ]]; then
                local code_retained_group_pass_policy
                code_retained_group_pass_policy="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-pass.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d '{"max_commands":8}')"
                mfx_assert_eq "$code_retained_group_pass_policy" "200" "core wasm retained-group-pass policy status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-pass.out" "\"ok\":true" "core wasm retained-group-pass policy ok"
            fi

            _mfx_core_http_assert_wasm_load_manifest_ok \
                "$tmp_dir/wasm-load-manifest-retained-group-pass-sample.out" \
                "$base_url" \
                "$token" \
                "$retained_group_pass_manifest_path" \
                "core wasm load-manifest retained-group-pass sample"

            _mfx_core_http_assert_wasm_test_dispatch_ok \
                "$tmp_dir/wasm-test-dispatch-retained-group-pass.out" \
                "$base_url" \
                "$token" \
                "core wasm test-dispatch retained-group-pass sample" \
                "$require_rendered_any"

            local dispatch_group_pass_glow_commands
            local dispatch_group_pass_sprite_commands
            local dispatch_group_pass_particle_commands
            local dispatch_group_pass_ribbon_commands
            local dispatch_group_pass_quad_commands
            dispatch_group_pass_glow_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-pass.out" "executed_glow_emitter_commands")"
            dispatch_group_pass_sprite_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-pass.out" "executed_sprite_emitter_commands")"
            dispatch_group_pass_particle_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-pass.out" "executed_particle_emitter_commands")"
            dispatch_group_pass_ribbon_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-pass.out" "executed_ribbon_trail_commands")"
            dispatch_group_pass_quad_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-pass.out" "executed_quad_field_commands")"
            if [[ -z "$dispatch_group_pass_glow_commands" || "$dispatch_group_pass_glow_commands" -lt 1 || -z "$dispatch_group_pass_sprite_commands" || "$dispatch_group_pass_sprite_commands" -lt 1 || -z "$dispatch_group_pass_particle_commands" || "$dispatch_group_pass_particle_commands" -lt 1 || -z "$dispatch_group_pass_ribbon_commands" || "$dispatch_group_pass_ribbon_commands" -lt 1 || -z "$dispatch_group_pass_quad_commands" || "$dispatch_group_pass_quad_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-pass sample did not execute grouped retained commands"
            fi

            local code_state_after_retained_group_pass_dispatch
            code_state_after_retained_group_pass_dispatch="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-pass-dispatch.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_pass_dispatch" "200" "core wasm state after retained-group-pass dispatch status"
            _mfx_core_http_assert_wasm_dispatch_diagnostics_consistent \
                "$tmp_dir/wasm-test-dispatch-retained-group-pass.out" \
                "$tmp_dir/state-after-wasm-retained-group-pass-dispatch.out" \
                "core wasm retained-group-pass dispatch diagnostics consistency" \
                "$latest_state_snapshot_file" \
                "$platform"

            local retained_group_pass_glow_active_count
            local retained_group_pass_sprite_active_count
            local retained_group_pass_particle_active_count
            local retained_group_pass_ribbon_active_count
            local retained_group_pass_quad_active_count
            retained_group_pass_glow_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-pass-dispatch.out" "retained_glow_emitter_active_count")"
            retained_group_pass_sprite_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-pass-dispatch.out" "retained_sprite_emitter_active_count")"
            retained_group_pass_particle_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-pass-dispatch.out" "retained_particle_emitter_active_count")"
            retained_group_pass_ribbon_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-pass-dispatch.out" "retained_ribbon_trail_active_count")"
            retained_group_pass_quad_active_count="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-pass-dispatch.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_pass_glow_active_count" || "$retained_group_pass_glow_active_count" -lt 1 || -z "$retained_group_pass_sprite_active_count" || "$retained_group_pass_sprite_active_count" -lt 1 || -z "$retained_group_pass_particle_active_count" || "$retained_group_pass_particle_active_count" -lt 1 || -z "$retained_group_pass_ribbon_active_count" || "$retained_group_pass_ribbon_active_count" -lt 1 || -z "$retained_group_pass_quad_active_count" || "$retained_group_pass_quad_active_count" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-pass sample did not keep grouped retained instances alive"
            fi

            local code_retained_group_pass_update_dispatch
            code_retained_group_pass_update_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-pass-update.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":712,"y":388,"button":3}')"
            mfx_assert_eq "$code_retained_group_pass_update_dispatch" "200" "core wasm retained-group-pass update dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-pass-update.out" "\"ok\":true" "core wasm retained-group-pass update dispatch ok"

            local dispatch_group_pass_update_commands
            dispatch_group_pass_update_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-pass-update.out" "executed_group_pass_commands")"
            if [[ -z "$dispatch_group_pass_update_commands" || "$dispatch_group_pass_update_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-pass sample did not execute middle-click group-pass commands"
            fi

            local code_retained_group_pass_remove_dispatch
            code_retained_group_pass_remove_dispatch="$(mfx_http_code \
                "$tmp_dir/wasm-test-dispatch-retained-group-pass-remove.out" \
                "$base_url/api/wasm/test-dispatch-click" \
                -X POST \
                -H "x-mfcmouseeffect-token: $token" \
                -H "Content-Type: application/json" \
                -d '{"x":712,"y":388,"button":2}')"
            mfx_assert_eq "$code_retained_group_pass_remove_dispatch" "200" "core wasm retained-group-pass remove dispatch status"
            mfx_assert_file_contains "$tmp_dir/wasm-test-dispatch-retained-group-pass-remove.out" "\"ok\":true" "core wasm retained-group-pass remove dispatch ok"

            local dispatch_group_pass_remove_commands
            dispatch_group_pass_remove_commands="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/wasm-test-dispatch-retained-group-pass-remove.out" "executed_group_remove_commands")"
            if [[ -z "$dispatch_group_pass_remove_commands" || "$dispatch_group_pass_remove_commands" -lt 1 ]]; then
                mfx_fail "core wasm retained-group-pass sample did not execute group-remove commands"
            fi

            local code_state_after_retained_group_pass_remove
            code_state_after_retained_group_pass_remove="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-pass-remove.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
            mfx_assert_eq "$code_state_after_retained_group_pass_remove" "200" "core wasm state after retained-group-pass remove status"
            local retained_group_pass_glow_after_remove
            local retained_group_pass_sprite_after_remove
            local retained_group_pass_particle_after_remove
            local retained_group_pass_ribbon_after_remove
            local retained_group_pass_quad_after_remove
            retained_group_pass_glow_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-pass-remove.out" "retained_glow_emitter_active_count")"
            retained_group_pass_sprite_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-pass-remove.out" "retained_sprite_emitter_active_count")"
            retained_group_pass_particle_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-pass-remove.out" "retained_particle_emitter_active_count")"
            retained_group_pass_ribbon_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-pass-remove.out" "retained_ribbon_trail_active_count")"
            retained_group_pass_quad_after_remove="$(_mfx_core_http_wasm_parse_uint_field "$tmp_dir/state-after-wasm-retained-group-pass-remove.out" "retained_quad_field_active_count")"
            if [[ -z "$retained_group_pass_glow_after_remove" || "$retained_group_pass_glow_after_remove" -ne 0 || -z "$retained_group_pass_sprite_after_remove" || "$retained_group_pass_sprite_after_remove" -ne 0 || -z "$retained_group_pass_particle_after_remove" || "$retained_group_pass_particle_after_remove" -ne 0 || -z "$retained_group_pass_ribbon_after_remove" || "$retained_group_pass_ribbon_after_remove" -ne 0 || -z "$retained_group_pass_quad_after_remove" || "$retained_group_pass_quad_after_remove" -ne 0 ]]; then
                mfx_fail "core wasm retained-group-pass sample did not clear grouped retained instances after remove_group"
            fi

            latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-pass-remove.out"

            if [[ -n "$default_manifest_path" && -f "$default_manifest_path" ]]; then
                _mfx_core_http_assert_wasm_load_manifest_ok \
                    "$tmp_dir/wasm-load-manifest-after-retained-group-pass-sample.out" \
                    "$base_url" \
                    "$token" \
                    "$default_manifest_path" \
                    "core wasm load-manifest restore after retained-group-pass sample"
            fi

            if [[ -n "$retained_group_pass_baseline_max_commands" && "$retained_group_pass_baseline_max_commands" -lt 5 ]]; then
                local code_retained_group_pass_policy_restore
                code_retained_group_pass_policy_restore="$(mfx_http_code \
                    "$tmp_dir/wasm-policy-retained-group-pass-restore.out" \
                    "$base_url/api/wasm/policy" \
                    -X POST \
                    -H "x-mfcmouseeffect-token: $token" \
                    -H "Content-Type: application/json" \
                    -d "{\"max_commands\":$retained_group_pass_baseline_max_commands}")"
                mfx_assert_eq "$code_retained_group_pass_policy_restore" "200" "core wasm retained-group-pass policy restore status"
                mfx_assert_file_contains "$tmp_dir/wasm-policy-retained-group-pass-restore.out" "\"ok\":true" "core wasm retained-group-pass policy restore ok"
                local code_state_after_retained_group_pass_policy_restore
                code_state_after_retained_group_pass_policy_restore="$(mfx_http_code "$tmp_dir/state-after-wasm-retained-group-pass-policy-restore.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
                mfx_assert_eq "$code_state_after_retained_group_pass_policy_restore" "200" "core wasm retained-group-pass policy restore state status"
                latest_state_snapshot_file="$tmp_dir/state-after-wasm-retained-group-pass-policy-restore.out"
            fi
        fi
    fi

    local code_affine_translate
    code_affine_translate="$(_mfx_core_http_wasm_test_resolve_image_affine_http_code \
        "$tmp_dir/wasm-affine-translate.out" \
        "$base_url" \
        "$token" \
        '{"x":100,"y":200,"scale":2.0,"rotation":0.5,"alpha":0.5,"life_ms":500,"delay_ms":17,"tint_rgba":16777215,"affine_enabled":false,"affine_dx":12,"affine_dy":-7}')"
    mfx_assert_eq "$code_affine_translate" "200" "core wasm affine resolve translate status"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"ok\":true" "core wasm affine resolve translate ok"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"resolved_x_int\":112" "core wasm affine resolve translate x"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"resolved_y_int\":193" "core wasm affine resolve translate y"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"resolved_scale_milli\":2000" "core wasm affine resolve translate scale"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"resolved_rotation_millirad\":500" "core wasm affine resolve translate rotation"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"runtime_scale_milli\":2000" "core wasm affine resolve translate runtime scale"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"runtime_alpha_milli\":500" "core wasm affine resolve translate runtime alpha"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"runtime_delay_ms\":17" "core wasm affine resolve translate runtime delay"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"runtime_life_ms\":500" "core wasm affine resolve translate runtime life"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-translate.out" "\"runtime_apply_tint\":false" "core wasm affine resolve translate runtime tint"

    local code_affine_scale
    code_affine_scale="$(_mfx_core_http_wasm_test_resolve_image_affine_http_code \
        "$tmp_dir/wasm-affine-scale.out" \
        "$base_url" \
        "$token" \
        '{"x":100,"y":200,"scale":2.0,"rotation":0.5,"alpha":0.8,"life_ms":700,"delay_ms":9,"tint_rgba":4281550003,"affine_enabled":true,"affine_dx":12,"affine_dy":-7,"affine_m11":2.0,"affine_m12":0.0,"affine_m21":0.0,"affine_m22":2.0}')"
    mfx_assert_eq "$code_affine_scale" "200" "core wasm affine resolve scale status"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"ok\":true" "core wasm affine resolve scale ok"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"resolved_x_int\":112" "core wasm affine resolve scale x"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"resolved_y_int\":193" "core wasm affine resolve scale y"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"resolved_scale_milli\":4000" "core wasm affine resolve scale scale"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"resolved_rotation_millirad\":500" "core wasm affine resolve scale rotation"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"runtime_scale_milli\":4000" "core wasm affine resolve scale runtime scale"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"runtime_alpha_milli\":800" "core wasm affine resolve scale runtime alpha"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"runtime_delay_ms\":9" "core wasm affine resolve scale runtime delay"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"runtime_life_ms\":700" "core wasm affine resolve scale runtime life"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-scale.out" "\"runtime_apply_tint\":true" "core wasm affine resolve scale runtime tint"

    local code_affine_rotate
    code_affine_rotate="$(_mfx_core_http_wasm_test_resolve_image_affine_http_code \
        "$tmp_dir/wasm-affine-rotate.out" \
        "$base_url" \
        "$token" \
        '{"x":100,"y":200,"scale":1.0,"rotation":0.0,"alpha":-1.0,"life_ms":120,"delay_ms":22,"affine_enabled":true,"affine_dx":0,"affine_dy":0,"affine_m11":0.0,"affine_m12":-1.0,"affine_m21":1.0,"affine_m22":0.0}')"
    mfx_assert_eq "$code_affine_rotate" "200" "core wasm affine resolve rotate status"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-rotate.out" "\"ok\":true" "core wasm affine resolve rotate ok"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-rotate.out" "\"resolved_scale_milli\":1000" "core wasm affine resolve rotate scale"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-rotate.out" "\"resolved_rotation_millirad\":1571" "core wasm affine resolve rotate rotation"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-rotate.out" "\"runtime_alpha_milli\":1000" "core wasm affine resolve rotate runtime alpha fallback"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-rotate.out" "\"runtime_delay_ms\":22" "core wasm affine resolve rotate runtime delay"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-rotate.out" "\"runtime_life_ms\":120" "core wasm affine resolve rotate runtime life"

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
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-max.out" "\"runtime_delay_ms\":60000" "core wasm affine resolve unsigned-max runtime delay clamp"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-max.out" "\"runtime_life_ms\":10000" "core wasm affine resolve unsigned-max runtime life clamp"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-max.out" "\"runtime_apply_tint\":true" "core wasm affine resolve unsigned-max runtime tint"

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
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-negative.out" "\"runtime_delay_ms\":0" "core wasm affine resolve unsigned-negative runtime delay"
    mfx_assert_file_contains "$tmp_dir/wasm-affine-unsigned-negative.out" "\"runtime_apply_tint\":false" "core wasm affine resolve unsigned-negative runtime tint"

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
