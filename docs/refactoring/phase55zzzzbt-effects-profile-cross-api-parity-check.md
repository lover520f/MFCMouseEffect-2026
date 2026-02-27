# Phase 55zzzzbt - Effects Profile Cross-API Parity Check

## Background
- Effects profile fields are exposed in two paths:
  - test route: `/api/effects/test-render-profiles`
  - runtime state: `/api/state.effects_profile`
- Previous checks mostly verified field presence, but not value parity between the two outputs.

## Decision
- Add script-level cross-API parity checks for representative tempo/color fields.
- Fail regression when the same logical field diverges across the two endpoints.

## Implementation
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_parse_helpers.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_effect_overlay_checks.sh`
- Added:
  - shared scalar parser helper (`_mfx_core_http_automation_parse_scalar_field`)
  - additional `/api/state` probe in effect-profile checks
  - value equality assertions for representative fields:
    - `meteor_duration_scale`
    - `helix_duration_scale`
    - `tubes_spin_scale`
    - `line_stroke_argb`

## Validation
- `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`

## Impact
- Capability: `特效（契约回归）`
- User-visible behavior unchanged.
- Regression now guards cross-API profile consistency, reducing silent contract drift risk.
