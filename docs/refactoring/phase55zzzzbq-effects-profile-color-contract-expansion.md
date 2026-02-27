# Phase 55zzzzbq - Effects Profile Color Contract Expansion

## Background
- macOS effect color paths are now profile-driven across click/trail/scroll/hold/hover.
- Existing test-route/profile-state contracts mainly asserted timing/size fields, leaving new color-profile fields unguarded.

## Decision
- Expand effect-profile JSON contracts to include color fields for all relevant categories.
- Add regression assertions for representative color keys in `/api/effects/test-render-profiles`.

## Implementation
1. API/state contract expansion
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestEffectsProfileApiRoute.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.EffectsProfileStateBuilder.cpp`
- Added color fields:
  - `trail`: `line/streamer/electric/meteor/tubes/particle` `*_stroke_argb` + `*_fill_argb`
  - `scroll`: directional `*_stroke_argb` + `*_fill_argb`
  - `hold`: base/style `*_stroke_argb`
  - `hover`: `glow_fill_argb`, `glow_stroke_argb`, `tubes_stroke_argb`

2. Regression contract expansion
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_effect_overlay_checks.sh`
- Added assertions for color fields in effect-profile probe response:
  - `line_stroke_argb`
  - `horizontal_positive_stroke_argb`
  - `left_base_stroke_argb`
  - `tubes_stroke_argb`

## Validation
- `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`

## Impact
- Capability: `特效（配置契约/回归）`
- User-visible behavior unchanged.
- Regression confidence improved for color-profile parity changes.
