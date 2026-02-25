#!/usr/bin/env bash

set -euo pipefail

mfx_wasm_selfcheck_json_escape() {
    local value="$1"
    printf '%s' "$value" | sed 's/\\/\\\\/g; s/"/\\"/g'
}

mfx_wasm_selfcheck_parse_uint_field() {
    local input_file="$1"
    local field_name="$2"
    sed -n "s/.*\"${field_name}\":\\([0-9][0-9]*\\).*/\\1/p" "$input_file" | head -n 1
}

mfx_wasm_selfcheck_parse_string_field() {
    local input_file="$1"
    local field_name="$2"
    sed -n "s/.*\"${field_name}\":\"\\([^\"]*\\)\".*/\\1/p" "$input_file" | head -n 1
}
