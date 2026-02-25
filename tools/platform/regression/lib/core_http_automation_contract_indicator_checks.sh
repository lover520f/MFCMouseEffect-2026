#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_contract_indicator_checks() {
    local tmp_dir="$1"
    local base_url="$2"
    local token="$3"

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
}
