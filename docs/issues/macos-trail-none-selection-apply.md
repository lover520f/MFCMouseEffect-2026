# macOS trail “none” selection not sticking

## Symptom
- In WebUI, selecting trail “None/无” and applying still keeps `active_effects.trail` as `line` and the trail continues to render.

## Root Cause
- `apply_settings` only accepted internal effect ids. If the UI sends a label (ZH/EN) or a value with stray whitespace, `NormalizeTrailEffectType` fell back to `line`, so `active_effects.trail` never switched to `none`.

## Fix
- `ApplyActiveSettings` now trims input and resolves it against effect metadata (value/alias/ZH/EN labels) before normalization. This makes label-based inputs map to canonical ids and allows `none` to persist.

## Verification
- Set trail to `None/无`, click Apply, then confirm:
  - `~/Library/Application Support/MFCMouseEffect/config.json` has `active_effects.trail: "none"`.
  - `/api/state.effects_profile.active_normalized.trail == "none"`.
