#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_wait_input_capture_state() {
    local base_url="$1"
    local token="$2"
    local expected_active="$3"
    local expected_reason="$4"
    local output_file="$5"
    local context="$6"
    local timeout_seconds="${MFX_CORE_HTTP_INPUT_CAPTURE_TIMEOUT_SECONDS:-10}"
    local deadline=$((SECONDS + timeout_seconds))

    while (( SECONDS < deadline )); do
        local code
        code="$(mfx_http_code "$output_file" "$base_url/api/state" -H "x-mfcmouseeffect-token: $token")"
        if [[ "$code" == "200" ]] && \
           mfx_file_contains_fixed "$output_file" "\"input_capture\"" && \
           mfx_file_contains_fixed "$output_file" "\"active\":$expected_active" && \
           mfx_file_contains_fixed "$output_file" "\"reason\":\"$expected_reason\""; then
            return 0
        fi
        sleep 0.2
    done

    mfx_fail "$context timeout: expected input_capture.active=$expected_active reason=$expected_reason"
}
