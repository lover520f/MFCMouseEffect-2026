#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_contract_platform_checks() {
    local platform="$1"
    local tmp_dir="$2"

    if [[ "$platform" != "macos" ]]; then
        return 0
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
}
