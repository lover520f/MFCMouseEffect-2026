#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_automation_parse_helper_surface_gate() {
    local script_dir
    local repo_root
    local matches
    script_dir="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    repo_root="$(cd -- "$script_dir/../../../.." && pwd)"

    if command -v rg >/dev/null 2>&1; then
        matches="$(rg "_mfx_core_http_automation_parse_(active_field|section_scalar_field|nested_section_scalar_field|scalar_field)" \
            -n \
            "$repo_root/tools/platform/regression/lib" \
            "$repo_root/tools/platform/manual" \
            --glob '!core_http_automation_parse_helpers.sh' \
            --glob '!core_http_automation_parse_helper_contract_checks.sh' || true)"
    else
        matches="$(grep -R -n -E "_mfx_core_http_automation_parse_(active_field|section_scalar_field|nested_section_scalar_field|scalar_field)" \
            "$repo_root/tools/platform/regression/lib" \
            "$repo_root/tools/platform/manual" \
            | grep -v "core_http_automation_parse_helpers.sh" \
            | grep -v "core_http_automation_parse_helper_contract_checks.sh" || true)"
    fi

    if [[ -n "$matches" ]]; then
        mfx_fail "core automation parse helper surface gate failed; legacy parser helpers still used outside compatibility fixture:\n$matches"
    fi
}

_mfx_core_http_automation_parse_helper_contract_checks() {
    local tmp_dir="$1"
    local fixture_file="$tmp_dir/automation-parse-helper-fixture.json"
    cat >"$fixture_file" <<'JSON'
{
  "catalog": [
    { "exe": "code.app" }
  ],
  "before_active_overlay_windows_total": 7,
  "active": {
    "click": "text",
    "trail": "line"
  },
  "click": {
    "normalized_type": "text",
    "base_opacity": 0.95
  },
  "misc": {
    "enabled": true,
    "display_name": "sample",
    "nullable_value": null
  },
  "command_samples": {
    "trail": {
      "emit": false
    },
    "hold": {
      "start": {
        "normalized_type": "hologram"
      }
    }
  }
}
JSON

    local parsed_catalog_process
    local parsed_uint
    local parsed_bool_scalar
    local parsed_string_scalar
    local parsed_null_scalar
    local parsed_active_click
    local parsed_click_type
    local parsed_click_opacity
    local parsed_trail_emit
    local parsed_hold_start_type

    parsed_catalog_process="$(_mfx_core_http_automation_first_catalog_process "$fixture_file")"
    parsed_uint="$(_mfx_core_http_automation_parse_uint_field "$fixture_file" "before_active_overlay_windows_total")"
    parsed_bool_scalar="$(_mfx_core_http_automation_parse_scalar_field "$fixture_file" "enabled")"
    parsed_string_scalar="$(_mfx_core_http_automation_parse_scalar_field "$fixture_file" "display_name")"
    parsed_null_scalar="$(_mfx_core_http_automation_parse_scalar_field "$fixture_file" "nullable_value")"
    parsed_active_click="$(_mfx_core_http_automation_parse_active_field "$fixture_file" "click")"
    parsed_click_type="$(_mfx_core_http_automation_parse_section_scalar_field "$fixture_file" "click" "normalized_type")"
    parsed_click_opacity="$(_mfx_core_http_automation_parse_section_scalar_field "$fixture_file" "click" "base_opacity")"
    parsed_trail_emit="$(_mfx_core_http_automation_parse_command_section_scalar_field "$fixture_file" "trail" "emit")"
    parsed_hold_start_type="$(_mfx_core_http_automation_parse_command_nested_section_scalar_field "$fixture_file" "hold" "start" "normalized_type")"

    mfx_assert_eq "$parsed_catalog_process" "code.app" "core automation parse helper first catalog process"
    mfx_assert_eq "$parsed_uint" "7" "core automation parse helper uint field"
    mfx_assert_eq "$parsed_bool_scalar" "true" "core automation parse helper boolean scalar"
    mfx_assert_eq "$parsed_string_scalar" "\"sample\"" "core automation parse helper string scalar"
    mfx_assert_eq "$parsed_null_scalar" "null" "core automation parse helper null scalar"
    mfx_assert_eq "$parsed_active_click" "text" "core automation parse helper active field"
    mfx_assert_eq "$parsed_click_type" "\"text\"" "core automation parse helper section string scalar"
    mfx_assert_eq "$parsed_click_opacity" "0.95" "core automation parse helper section number scalar"
    mfx_assert_eq "$parsed_trail_emit" "false" "core automation parse helper section fallback scalar"
    mfx_assert_eq "$parsed_hold_start_type" "\"hologram\"" "core automation parse helper nested section fallback scalar"

    _mfx_core_http_automation_parse_helper_surface_gate
}
