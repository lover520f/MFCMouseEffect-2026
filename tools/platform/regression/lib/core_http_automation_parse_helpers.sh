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

_mfx_core_http_automation_parse_uint_field() {
    local input_file="$1"
    local field_name="$2"
    sed -n "s/.*\"${field_name}\":\\([0-9][0-9]*\\).*/\\1/p" "$input_file" | head -n 1
}

_mfx_core_http_automation_parse_scalar_field() {
    local input_file="$1"
    local field_name="$2"
    sed -n "s/.*\"${field_name}\":[[:space:]]*\\([^,}]*\\).*/\\1/p" "$input_file" | head -n 1 | tr -d '[:space:]'
}

_mfx_core_http_automation_parse_active_field() {
    local input_file="$1"
    local field_name="$2"
    tr -d '\n\r' <"$input_file" | \
        sed -n "s/.*\"active\":{[^}]*\"${field_name}\":\"\\([^\"]*\\)\"[^}]*}.*/\\1/p" | \
        head -n 1
}
