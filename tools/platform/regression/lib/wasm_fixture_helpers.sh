#!/usr/bin/env bash

set -euo pipefail

mfx_wasm_fixture_manifest_copy() {
    local source_manifest_path="$1"
    local fixture_dir="$2"
    local context="$3"

    if [[ ! -f "$source_manifest_path" ]]; then
        mfx_fail "$context: source manifest missing: $source_manifest_path"
    fi

    rm -rf "$fixture_dir"
    mkdir -p "$fixture_dir"
    cp -R "$(dirname "$source_manifest_path")/." "$fixture_dir/"

    local fixture_manifest_path="$fixture_dir/$(basename "$source_manifest_path")"
    if [[ ! -f "$fixture_manifest_path" ]]; then
        mfx_fail "$context: fixture manifest missing after copy: $fixture_manifest_path"
    fi

    printf '%s' "$fixture_manifest_path"
}

mfx_wasm_fixture_manifest_entry_relative() {
    local manifest_path="$1"
    local context="$2"

    local entry_relative
    entry_relative="$(sed -n 's/.*"entry"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' "$manifest_path" | head -n 1)"
    if [[ -z "$entry_relative" ]]; then
        mfx_fail "$context: unable to parse manifest entry from: $manifest_path"
    fi
    printf '%s' "$entry_relative"
}

mfx_wasm_fixture_manifest_entry_path() {
    local manifest_path="$1"
    local context="$2"

    local entry_relative
    entry_relative="$(mfx_wasm_fixture_manifest_entry_relative "$manifest_path" "$context")"
    printf '%s' "$(dirname "$manifest_path")/$entry_relative"
}

mfx_wasm_fixture_require_entry_file() {
    local manifest_path="$1"
    local context="$2"

    local entry_path
    entry_path="$(mfx_wasm_fixture_manifest_entry_path "$manifest_path" "$context")"
    if [[ ! -f "$entry_path" ]]; then
        mfx_fail "$context: entry file missing: $entry_path"
    fi
    printf '%s' "$entry_path"
}

mfx_wasm_fixture_remove_entry_file() {
    local manifest_path="$1"
    local context="$2"

    local entry_path
    entry_path="$(mfx_wasm_fixture_require_entry_file "$manifest_path" "$context")"
    rm -f "$entry_path"
    printf '%s' "$entry_path"
}

mfx_wasm_fixture_write_manifest_with_api_version() {
    local manifest_path="$1"
    local plugin_id="$2"
    local api_version="$3"
    local entry_relative="$4"

    cat >"$manifest_path" <<EOF
{
  "id": "$plugin_id",
  "name": "$plugin_id",
  "version": "1.0.0",
  "api_version": $api_version,
  "entry": "$entry_relative"
}
EOF
}
