#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_json_escape() {
    local value="$1"
    printf '%s' "$value" | sed 's/\\/\\\\/g; s/"/\\"/g'
}

_mfx_core_http_automation_first_catalog_process() {
    local file_path="$1"
    sed -n 's/.*"exe":"\([^"]*\)".*/\1/p' "$file_path" | head -n 1
}

_mfx_core_http_automation_assert_scope_match() {
    local base_url="$1"
    local token="$2"
    local process_name="$3"
    local scope_value="$4"
    local expected_matched="$5"
    local output_file="$6"
    local context="$7"

    local process_escaped
    local scope_escaped
    process_escaped="$(_mfx_core_http_automation_json_escape "$process_name")"
    scope_escaped="$(_mfx_core_http_automation_json_escape "$scope_value")"

    local code
    code="$(mfx_http_code "$output_file" "$base_url/api/automation/test-app-scope-match" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "{\"process\":\"$process_escaped\",\"app_scopes\":[\"$scope_escaped\"]}")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"matched\":$expected_matched" "$context matched"
}

_mfx_core_http_automation_assert_binding_priority() {
    local base_url="$1"
    local token="$2"
    local payload="$3"
    local expected_matched="$4"
    local expected_index="$5"
    local expected_keys="$6"
    local expected_scope_specificity="$7"
    local expected_chain_length="$8"
    local output_file="$9"
    local context="${10}"

    local code
    code="$(mfx_http_code "$output_file" "$base_url/api/automation/test-binding-priority" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "$payload")"
    mfx_assert_eq "$code" "200" "$context status"
    mfx_assert_file_contains "$output_file" "\"ok\":true" "$context ok"
    mfx_assert_file_contains "$output_file" "\"matched\":$expected_matched" "$context matched"
    mfx_assert_file_contains "$output_file" "\"selected_binding_index\":$expected_index" "$context selected index"
    mfx_assert_file_contains "$output_file" "\"selected_scope_specificity\":$expected_scope_specificity" "$context selected scope specificity"
    mfx_assert_file_contains "$output_file" "\"selected_chain_length\":$expected_chain_length" "$context selected chain length"
    if [[ "$expected_matched" == "true" ]]; then
        local expected_keys_escaped
        expected_keys_escaped="$(_mfx_core_http_automation_json_escape "$expected_keys")"
        mfx_assert_file_contains "$output_file" "\"selected_keys\":\"$expected_keys_escaped\"" "$context selected keys"
    fi
}

_mfx_core_http_run_automation_contract_checks() {
    local platform="$1"
    local tmp_dir="$2"
    local base_url="$3"
    local token="$4"

    local code_active_process
    code_active_process="$(mfx_http_code "$tmp_dir/active-process.out" "$base_url/api/automation/active-process" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{}')"
    mfx_assert_eq "$code_active_process" "200" "core active-process status"
    mfx_assert_file_contains "$tmp_dir/active-process.out" "\"ok\":true" "core active-process ok"
    mfx_assert_file_contains "$tmp_dir/active-process.out" "\"process\":\"" "core active-process process field"

    local code_test_inject_shortcut
    code_test_inject_shortcut="$(mfx_http_code "$tmp_dir/test-inject-shortcut.out" "$base_url/api/automation/test-inject-shortcut" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"keys":"Cmd+C"}')"
    mfx_assert_eq "$code_test_inject_shortcut" "200" "core test-inject-shortcut status"
    mfx_assert_file_contains "$tmp_dir/test-inject-shortcut.out" "\"ok\":true" "core test-inject-shortcut ok"
    mfx_assert_file_contains "$tmp_dir/test-inject-shortcut.out" "\"keys\":\"Cmd+C\"" "core test-inject-shortcut keys echo"

    local code_app_catalog
    code_app_catalog="$(mfx_http_code "$tmp_dir/app-catalog.out" "$base_url/api/automation/app-catalog" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{"force":true}')"
    mfx_assert_eq "$code_app_catalog" "200" "core app-catalog status"
    mfx_assert_file_contains "$tmp_dir/app-catalog.out" "\"ok\":true" "core app-catalog ok"
    mfx_assert_file_contains "$tmp_dir/app-catalog.out" "\"count\":" "core app-catalog count field"

    local code_app_catalog_cached
    code_app_catalog_cached="$(mfx_http_code "$tmp_dir/app-catalog-cached.out" "$base_url/api/automation/app-catalog" -X POST -H "x-mfcmouseeffect-token: $token" -H "Content-Type: application/json" -d '{"force":false}')"
    mfx_assert_eq "$code_app_catalog_cached" "200" "core app-catalog cached status"
    mfx_assert_file_contains "$tmp_dir/app-catalog-cached.out" "\"ok\":true" "core app-catalog cached ok"
    mfx_assert_file_contains "$tmp_dir/app-catalog-cached.out" "\"count\":" "core app-catalog cached count field"

    local selected_process
    selected_process="$(_mfx_core_http_automation_first_catalog_process "$tmp_dir/app-catalog.out")"
    if [[ -n "$selected_process" ]]; then
        local selected_scope="process:$selected_process"
        local selected_scope_escaped
        selected_scope_escaped="$(_mfx_core_http_automation_json_escape "$selected_scope")"

        local code_state_apply_scope
        code_state_apply_scope="$(mfx_http_code "$tmp_dir/state-apply-scope.out" "$base_url/api/state" \
            -X POST \
            -H "x-mfcmouseeffect-token: $token" \
            -H "Content-Type: application/json" \
            -d "{\"automation\":{\"enabled\":true,\"mouse_mappings\":[{\"enabled\":true,\"trigger\":\"left_click\",\"app_scopes\":[\"$selected_scope_escaped\"],\"keys\":\"Cmd+C\"}]}}")"
        mfx_assert_eq "$code_state_apply_scope" "200" "core state apply selected-scope status"
        mfx_assert_file_contains "$tmp_dir/state-apply-scope.out" "\"ok\":true" "core state apply selected-scope ok"

        local code_state_after_scope
        code_state_after_scope="$(mfx_http_code "$tmp_dir/state-after-scope.out" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
        mfx_assert_eq "$code_state_after_scope" "200" "core state after selected-scope status"
        mfx_assert_file_contains "$tmp_dir/state-after-scope.out" "\"app_scopes\":[\"$selected_scope\"]" "core selected-scope persistence"
    fi

    _mfx_core_http_automation_assert_scope_match "$base_url" "$token" "code" "process:code.exe" "true" "$tmp_dir/scope-code-vs-exe.out" "core app-scope code<->exe"
    _mfx_core_http_automation_assert_scope_match "$base_url" "$token" "code.app" "process:code" "true" "$tmp_dir/scope-app-vs-base.out" "core app-scope app<->base"
    _mfx_core_http_automation_assert_scope_match "$base_url" "$token" "code.exe" "process:code.app" "true" "$tmp_dir/scope-exe-vs-app.out" "core app-scope exe<->app"
    _mfx_core_http_automation_assert_scope_match "$base_url" "$token" "safari" "process:code" "false" "$tmp_dir/scope-negative.out" "core app-scope negative"

    local payload_priority_scope_specific
    payload_priority_scope_specific='{"process":"code.app","history":["left_click"],"mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["all"],"keys":"Cmd+V"},{"enabled":true,"trigger":"left_click","app_scopes":["process:code.exe"],"keys":"Cmd+C"}]}'
    _mfx_core_http_automation_assert_binding_priority \
        "$base_url" \
        "$token" \
        "$payload_priority_scope_specific" \
        "true" \
        "1" \
        "Cmd+C" \
        "1" \
        "1" \
        "$tmp_dir/scope-priority-specific.out" \
        "core app-scope priority process-over-all"

    local payload_priority_scope_fallback
    payload_priority_scope_fallback='{"process":"safari","history":["left_click"],"mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["all"],"keys":"Cmd+V"},{"enabled":true,"trigger":"left_click","app_scopes":["process:code.exe"],"keys":"Cmd+C"}]}'
    _mfx_core_http_automation_assert_binding_priority \
        "$base_url" \
        "$token" \
        "$payload_priority_scope_fallback" \
        "true" \
        "0" \
        "Cmd+V" \
        "0" \
        "1" \
        "$tmp_dir/scope-priority-fallback.out" \
        "core app-scope priority all-fallback"

    local payload_priority_long_chain
    payload_priority_long_chain='{"process":"code","history":["left_click","left_click"],"mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["process:code.app"],"keys":"Cmd+V"},{"enabled":true,"trigger":"left_click>left_click","app_scopes":["all"],"keys":"Cmd+T"}]}'
    _mfx_core_http_automation_assert_binding_priority \
        "$base_url" \
        "$token" \
        "$payload_priority_long_chain" \
        "true" \
        "1" \
        "Cmd+T" \
        "0" \
        "2" \
        "$tmp_dir/scope-priority-long-chain.out" \
        "core app-scope priority longest-chain-first"

    local code_match_and_inject
    code_match_and_inject="$(mfx_http_code "$tmp_dir/match-and-inject.out" "$base_url/api/automation/test-match-and-inject" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"process":"code.app","history":["left_click"],"mappings":[{"enabled":true,"trigger":"left_click","app_scopes":["all"],"keys":"Cmd+C"}]}')"
    mfx_assert_eq "$code_match_and_inject" "200" "core test-match-and-inject status"
    mfx_assert_file_contains "$tmp_dir/match-and-inject.out" "\"ok\":true" "core test-match-and-inject ok"
    mfx_assert_file_contains "$tmp_dir/match-and-inject.out" "\"matched\":true" "core test-match-and-inject matched"
    mfx_assert_file_contains "$tmp_dir/match-and-inject.out" "\"injected\":true" "core test-match-and-inject injected"
    mfx_assert_file_contains "$tmp_dir/match-and-inject.out" "\"selected_keys\":\"Cmd+C\"" "core test-match-and-inject selected keys"

    local code_shortcut_map_cmd_v
    code_shortcut_map_cmd_v="$(mfx_http_code "$tmp_dir/shortcut-map-cmd-v.out" "$base_url/api/automation/test-shortcut-from-mac-keycode" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"mac_key_code":9,"cmd":true}')"
    mfx_assert_eq "$code_shortcut_map_cmd_v" "200" "core shortcut-map cmd+v status"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-v.out" "\"ok\":true" "core shortcut-map cmd+v ok"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-v.out" "\"supported\":true" "core shortcut-map cmd+v supported"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-v.out" "\"vk_code\":86" "core shortcut-map cmd+v vk"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-v.out" "\"shortcut\":\"Win+V\"" "core shortcut-map cmd+v text"

    local code_shortcut_map_cmd_tab
    code_shortcut_map_cmd_tab="$(mfx_http_code "$tmp_dir/shortcut-map-cmd-tab.out" "$base_url/api/automation/test-shortcut-from-mac-keycode" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"mac_key_code":48,"cmd":true}')"
    mfx_assert_eq "$code_shortcut_map_cmd_tab" "200" "core shortcut-map cmd+tab status"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-tab.out" "\"ok\":true" "core shortcut-map cmd+tab ok"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-tab.out" "\"supported\":true" "core shortcut-map cmd+tab supported"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-tab.out" "\"vk_code\":9" "core shortcut-map cmd+tab vk"
    mfx_assert_file_contains "$tmp_dir/shortcut-map-cmd-tab.out" "\"shortcut\":\"Win+Tab\"" "core shortcut-map cmd+tab text"

    local code_input_indicator_labels
    code_input_indicator_labels="$(mfx_http_code "$tmp_dir/input-indicator-labels.out" "$base_url/api/input-indicator/test-mouse-labels" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{}')"
    mfx_assert_eq "$code_input_indicator_labels" "200" "core input-indicator labels probe status"
    mfx_assert_file_contains "$tmp_dir/input-indicator-labels.out" "\"ok\":true" "core input-indicator labels probe ok"
    mfx_assert_file_contains "$tmp_dir/input-indicator-labels.out" "\"supported\":true" "core input-indicator labels probe supported"
    mfx_assert_file_contains "$tmp_dir/input-indicator-labels.out" "\"matched\":true" "core input-indicator labels probe matched"
    mfx_assert_file_contains "$tmp_dir/input-indicator-labels.out" "\"labels\":[\"L\",\"R\",\"M\"]" "core input-indicator labels probe labels"

    local code_input_indicator_keyboard_labels
    code_input_indicator_keyboard_labels="$(mfx_http_code "$tmp_dir/input-indicator-keyboard-labels.out" "$base_url/api/input-indicator/test-keyboard-labels" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{}')"
    mfx_assert_eq "$code_input_indicator_keyboard_labels" "200" "core input-indicator keyboard labels probe status"
    mfx_assert_file_contains "$tmp_dir/input-indicator-keyboard-labels.out" "\"ok\":true" "core input-indicator keyboard labels probe ok"
    mfx_assert_file_contains "$tmp_dir/input-indicator-keyboard-labels.out" "\"supported\":true" "core input-indicator keyboard labels probe supported"
    mfx_assert_file_contains "$tmp_dir/input-indicator-keyboard-labels.out" "\"matched\":true" "core input-indicator keyboard labels probe matched"
    mfx_assert_file_contains "$tmp_dir/input-indicator-keyboard-labels.out" "\"labels\":[\"A\",\"Cmd+K9\",\"K6\"]" "core input-indicator keyboard labels probe labels"

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
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"before_invariant_ok\":true" "core effect overlay probe before invariant"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"after_invariant_ok\":true" "core effect overlay probe after invariant"
    mfx_assert_file_contains "$tmp_dir/effect-overlay-probe.out" "\"restored_to_baseline\":true" "core effect overlay probe restore baseline"

    if [[ "$platform" == "macos" ]]; then
        if ! mfx_file_contains_fixed "$tmp_dir/effect-overlay-probe.out" "\"supported\":true"; then
            mfx_fail "core effect overlay probe support on macos: expected supported=true"
        fi
        if mfx_file_contains_fixed "$tmp_dir/app-catalog.out" "\"count\":0"; then
            mfx_fail "core app-catalog non-empty on macos: unexpected count=0"
        fi
        if ! mfx_file_contains_regex "$tmp_dir/app-catalog.out" "\"exe\":\"[^\"]+\\.app\""; then
            mfx_fail "core app-catalog app suffix on macos: missing .app process entry"
        fi
        if ! mfx_file_contains_regex "$tmp_dir/active-process.out" "\"process\":\"[^\"]+\""; then
            mfx_fail "core active-process non-empty on macos: expected non-empty process base name"
        fi
        if ! mfx_file_contains_fixed "$tmp_dir/schema.out" "\"keyboard_injector\":true"; then
            mfx_fail "core schema keyboard injector capability on macos: expected true"
        fi
        if ! mfx_file_contains_fixed "$tmp_dir/test-inject-shortcut.out" "\"accepted\":true"; then
            mfx_fail "core test-inject-shortcut on macos: expected accepted=true (dry-run injector mode)"
        fi
    fi
}
