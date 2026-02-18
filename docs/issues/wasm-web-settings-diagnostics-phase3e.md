# WASM Runtime Diagnostics View (Phase 3e)

## Summary

This phase finalizes the WASM panel diagnostics UX by exposing execution-budget telemetry that was already available in server state.

## What Changed

1. Diagnostics model extraction
- Added `WebUIWorkspace/src/wasm/diagnostics-model.js`.
- Centralizes:
  - diagnostics normalization
  - warning detection (`budget` / `parse` risk)
  - human-readable text formatting for call metrics and budget flags

2. State normalization
- `WebUIWorkspace/src/entries/wasm-main.js` now maps diagnostics fields:
  - `last_call_duration_us`
  - `last_output_bytes`
  - `last_command_count`
  - `last_call_exceeded_budget`
  - `last_call_rejected_by_budget`
  - `last_output_truncated_by_budget`
  - `last_command_truncated_by_budget`
  - `last_budget_reason`
  - `last_parse_error`

3. WASM panel rendering
- `WasmPluginFields.svelte` now shows:
  - Last call metrics
  - Budget flags
  - Budget reason
  - Parse error
- Warning states are highlighted with `is-warn` style when budget/parse risk is detected.

4. i18n and style
- Added EN/CN labels for new diagnostics fields and flag names.
- Added `.wasm-value.is-warn` style.

## Validation

1. `pnpm run build` in `WebUIWorkspace` passed.
2. `Release|x64` MSBuild passed.

## Notes

- This is a pure visibility enhancement: no host/runtime logic change.
- Operators can now distinguish render issues from budget rejection/truncation quickly.
