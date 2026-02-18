# WASM Budget Schema-Driven UI (Phase 3d)

## Summary

This phase removes hard-coded budget bounds from the Web panel and makes policy editing schema-driven.

## Changes

1. Schema defaults/ranges
- `SettingsSchemaBuilder` now emits WASM budget `default` values from built-in config defaults, not current runtime config.
- This makes "reset defaults" deterministic across machines/config states.

2. Web panel range model
- Added `WebUIWorkspace/src/wasm/policy-model.js`:
  - normalize policy ranges from schema
  - clamp/snap int/float values by min/max/step
  - resolve effective input defaults

3. WASM section integration
- `settings-form.js` now passes `schema.wasm` into the WASM section.
- `wasm-main.js` now forwards normalized schema state.
- `WasmPluginFields.svelte` now:
  - uses schema min/max/step for budget inputs
  - clamps outgoing policy values with shared model
  - adds `Reset Defaults` action to restore default policy and persist it

4. i18n
- Added `btn_wasm_reset_policy` (EN/CN).

## Validation

1. `pnpm run build` in `WebUIWorkspace` passed.
2. `Release|x64` MSBuild passed.

## Notes

- Policy constraints now have a single source of truth (server schema), reducing drift risk.
