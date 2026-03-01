#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_wasm_load_manifest_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"

    local manifest_path_escaped
    manifest_path_escaped="$(_mfx_core_http_wasm_json_escape "$manifest_path")"
    mfx_http_code "$output_file" "$base_url/api/wasm/load-manifest" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "{\"manifest_path\":\"$manifest_path_escaped\"}"
}

_mfx_core_http_wasm_load_manifest_trimmed_payload_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"

    local manifest_path_escaped
    manifest_path_escaped="$(_mfx_core_http_wasm_json_escape "$manifest_path")"
    mfx_http_code "$output_file" "$base_url/api/wasm/load-manifest" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "{\"manifest_path\":\"  $manifest_path_escaped  \"}"
}

_mfx_core_http_wasm_load_manifest_payload_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local payload_json="$4"
    mfx_http_code "$output_file" "$base_url/api/wasm/load-manifest" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "$payload_json"
}

_mfx_core_http_wasm_import_selected_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local manifest_path="$4"

    local manifest_path_escaped
    manifest_path_escaped="$(_mfx_core_http_wasm_json_escape "$manifest_path")"
    mfx_http_code "$output_file" "$base_url/api/wasm/import-selected" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "{\"manifest_path\":\"$manifest_path_escaped\"}"
}

_mfx_core_http_wasm_export_all_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    mfx_http_code "$output_file" "$base_url/api/wasm/export-all" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{}'
}

_mfx_core_http_wasm_import_dialog_probe_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local initial_path="${4:-}"

    local payload='{"probe_only":true}'
    if [[ -n "$initial_path" ]]; then
        local initial_path_escaped
        initial_path_escaped="$(_mfx_core_http_wasm_json_escape "$initial_path")"
        payload="{\"probe_only\":true,\"initial_path\":\"  $initial_path_escaped  \"}"
    fi

    mfx_http_code "$output_file" "$base_url/api/wasm/import-from-folder-dialog" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "$payload"
}

_mfx_core_http_wasm_test_dispatch_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    mfx_http_code "$output_file" "$base_url/api/wasm/test-dispatch-click" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{"x":640,"y":360,"button":1}'
}

_mfx_core_http_wasm_test_resolve_image_affine_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    local payload_json="$4"
    mfx_http_code "$output_file" "$base_url/api/wasm/test-resolve-image-affine" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d "$payload_json"
}

_mfx_core_http_wasm_test_reset_runtime_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    mfx_http_code "$output_file" "$base_url/api/wasm/test-reset-runtime" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{}'
}

_mfx_core_http_wasm_reload_http_code() {
    local output_file="$1"
    local base_url="$2"
    local token="$3"
    mfx_http_code "$output_file" "$base_url/api/wasm/reload" \
        -X POST \
        -H "x-mfcmouseeffect-token: $token" \
        -H "Content-Type: application/json" \
        -d '{}'
}
