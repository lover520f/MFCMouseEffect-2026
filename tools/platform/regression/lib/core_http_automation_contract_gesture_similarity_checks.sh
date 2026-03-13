#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_contract_gesture_similarity_checks() {
    local tmp_dir="$1"
    local base_url="$2"
    local token="$3"

    local code_windowed
    code_windowed="$(mfx_http_code "$tmp_dir/gesture-similarity-windowed.out" "$base_url/api/automation/test-gesture-similarity" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"threshold":72,"margin":4,"options":{"enable_window_search":true},"captured_strokes":[[{"x":8,"y":72},{"x":14,"y":68},{"x":20,"y":64},{"x":26,"y":58},{"x":30,"y":52},{"x":34,"y":50},{"x":44,"y":50},{"x":54,"y":50},{"x":64,"y":50},{"x":74,"y":50},{"x":84,"y":50},{"x":90,"y":49},{"x":94,"y":48}]],"candidates":[{"id":"right","source":"preset","action_id":"right"},{"id":"left","source":"preset","action_id":"left"},{"id":"v","source":"preset","action_id":"diag_down_right_diag_up_right"}]}')"
    mfx_assert_eq "$code_windowed" "200" "core test-gesture-similarity windowed status"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-windowed.out" "\"ok\":true" "core test-gesture-similarity windowed ok"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-windowed.out" "\"accepted\":true" "core test-gesture-similarity windowed accepted"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-windowed.out" "\"best_id\":\"right\"" "core test-gesture-similarity windowed best id"

    local code_w_priority
    code_w_priority="$(mfx_http_code "$tmp_dir/gesture-similarity-w-priority.out" "$base_url/api/automation/test-gesture-similarity" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"threshold":72,"margin":4,"options":{"enable_window_search":true},"captured_strokes":[[{"x":8,"y":18},{"x":26,"y":82},{"x":44,"y":36},{"x":62,"y":82},{"x":84,"y":20}]],"candidates":[{"id":"w","source":"preset","action_id":"diag_down_right_diag_up_right_diag_down_right"},{"id":"v","source":"preset","action_id":"diag_down_right_diag_up_right"},{"id":"right","source":"preset","action_id":"right"}]}')"
    mfx_assert_eq "$code_w_priority" "200" "core test-gesture-similarity w-priority status"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-w-priority.out" "\"ok\":true" "core test-gesture-similarity w-priority ok"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-w-priority.out" "\"accepted\":true" "core test-gesture-similarity w-priority accepted"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-w-priority.out" "\"best_id\":\"w\"" "core test-gesture-similarity w-priority best id"

    local code_ambiguous
    code_ambiguous="$(mfx_http_code "$tmp_dir/gesture-similarity-ambiguous.out" "$base_url/api/automation/test-gesture-similarity" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"threshold":70,"margin":3,"options":{"enable_window_search":true},"captured_strokes":[[{"x":10,"y":60},{"x":30,"y":60},{"x":50,"y":60},{"x":70,"y":60},{"x":90,"y":60}]],"candidates":[{"id":"same_a","source":"custom","template_strokes":[[{"x":10,"y":60},{"x":30,"y":60},{"x":50,"y":60},{"x":70,"y":60},{"x":90,"y":60}]]},{"id":"same_b","source":"custom","template_strokes":[[{"x":10,"y":60},{"x":30,"y":60},{"x":50,"y":60},{"x":70,"y":60},{"x":90,"y":60}]]}]}')"
    mfx_assert_eq "$code_ambiguous" "200" "core test-gesture-similarity ambiguous status"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-ambiguous.out" "\"ok\":true" "core test-gesture-similarity ambiguous ok"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-ambiguous.out" "\"accepted\":false" "core test-gesture-similarity ambiguous rejected"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-ambiguous.out" "\"reason\":\"ambiguous\"" "core test-gesture-similarity ambiguous reason"

    local code_custom_order
    code_custom_order="$(mfx_http_code "$tmp_dir/gesture-similarity-custom-order.out" "$base_url/api/automation/test-gesture-similarity" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"threshold":72,"margin":4,"options":{"enable_window_search":true,"strict_stroke_count":true,"strict_stroke_order":true},"captured_strokes":[[{"x":14,"y":18},{"x":88,"y":18}],[{"x":88,"y":78},{"x":18,"y":78}]],"candidates":[{"id":"order_ok","source":"custom","template_strokes":[[{"x":12,"y":20},{"x":90,"y":20}],[{"x":88,"y":80},{"x":16,"y":80}]]},{"id":"order_bad","source":"custom","template_strokes":[[{"x":88,"y":80},{"x":16,"y":80}],[{"x":12,"y":20},{"x":90,"y":20}]]}]}')"
    mfx_assert_eq "$code_custom_order" "200" "core test-gesture-similarity custom-order status"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-custom-order.out" "\"ok\":true" "core test-gesture-similarity custom-order ok"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-custom-order.out" "\"accepted\":true" "core test-gesture-similarity custom-order accepted"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-custom-order.out" "\"best_id\":\"order_ok\"" "core test-gesture-similarity custom-order best id"

    local code_min_length
    code_min_length="$(mfx_http_code "$tmp_dir/gesture-similarity-min-length.out" "$base_url/api/automation/test-gesture-similarity" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"threshold":70,"margin":3,"options":{"enable_window_search":true,"strict_stroke_count":true,"strict_stroke_order":true,"min_effective_stroke_length_px":40},"captured_strokes":[[{"x":10,"y":10},{"x":18,"y":10}]],"candidates":[{"id":"short_noise","source":"custom","template_strokes":[[{"x":10,"y":10},{"x":90,"y":10}]]}]}')"
    mfx_assert_eq "$code_min_length" "200" "core test-gesture-similarity min-length status"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-min-length.out" "\"ok\":true" "core test-gesture-similarity min-length ok"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-min-length.out" "\"accepted\":false" "core test-gesture-similarity min-length rejected"
    mfx_assert_file_contains "$tmp_dir/gesture-similarity-min-length.out" "\"reason\":\"no_valid_candidate\"" "core test-gesture-similarity min-length reason"
}
