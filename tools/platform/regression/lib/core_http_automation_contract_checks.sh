#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_contract_checks_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$_mfx_core_http_automation_contract_checks_dir/core_http_automation_parse_helpers.sh"
source "$_mfx_core_http_automation_contract_checks_dir/core_http_automation_assert_helpers.sh"
source "$_mfx_core_http_automation_contract_checks_dir/core_http_automation_contract_basic_checks.sh"
source "$_mfx_core_http_automation_contract_checks_dir/core_http_automation_contract_app_scope_checks.sh"
source "$_mfx_core_http_automation_contract_checks_dir/core_http_automation_contract_priority_checks.sh"
source "$_mfx_core_http_automation_contract_checks_dir/core_http_automation_contract_match_inject_checks.sh"
source "$_mfx_core_http_automation_contract_checks_dir/core_http_automation_contract_shortcut_checks.sh"
source "$_mfx_core_http_automation_contract_checks_dir/core_http_automation_contract_indicator_checks.sh"
source "$_mfx_core_http_automation_contract_checks_dir/core_http_automation_contract_effect_overlay_checks.sh"
source "$_mfx_core_http_automation_contract_checks_dir/core_http_automation_contract_platform_checks.sh"

_mfx_core_http_run_automation_contract_checks() {
    local platform="$1"
    local tmp_dir="$2"
    local base_url="$3"
    local token="$4"

    _mfx_core_http_automation_contract_basic_checks \
        "$tmp_dir" \
        "$base_url" \
        "$token"

    _mfx_core_http_automation_contract_app_scope_checks \
        "$platform" \
        "$tmp_dir" \
        "$base_url" \
        "$token"

    _mfx_core_http_automation_contract_priority_checks \
        "$platform" \
        "$tmp_dir" \
        "$base_url" \
        "$token"

    _mfx_core_http_automation_contract_match_inject_checks \
        "$tmp_dir" \
        "$base_url" \
        "$token"

    _mfx_core_http_automation_contract_shortcut_checks \
        "$tmp_dir" \
        "$base_url" \
        "$token"

    _mfx_core_http_automation_contract_indicator_checks \
        "$tmp_dir" \
        "$base_url" \
        "$token"

    _mfx_core_http_automation_contract_effect_overlay_checks \
        "$platform" \
        "$tmp_dir" \
        "$base_url" \
        "$token"

    _mfx_core_http_automation_contract_platform_checks \
        "$platform" \
        "$tmp_dir"
}

_mfx_core_http_run_automation_effect_overlay_contract_checks() {
    local platform="$1"
    local tmp_dir="$2"
    local base_url="$3"
    local token="$4"

    _mfx_core_http_automation_contract_effect_overlay_checks \
        "$platform" \
        "$tmp_dir" \
        "$base_url" \
        "$token"
}
