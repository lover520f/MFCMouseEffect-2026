# WASM Action State Refresh Optimization (Phase 3f)

## Summary

WASM panel actions no longer force full settings reload on every operation.

## Problem

Before this phase, actions like:
- enable / disable
- reload plugin
- load manifest
- update policy

always triggered `reload()`:
- fetch `/api/schema`
- fetch `/api/state`
- re-render all sections

This caused unnecessary request cost and visible UI flicker for WASM-local operations.

## Solution

Refactored `WebUI/app.js` with a two-level refresh path:

1. `renderSettingsSnapshot(schema, state)`
- centralizes render + i18n sync + workspace refresh + status finalization.

2. `refreshStateSnapshot(useCachedSchema)`
- fetches fresh `/api/state`
- reuses cached schema when language is unchanged
- falls back to `/api/schema` fetch only when needed

3. `refreshAfterWasmAction()`
- WASM actions now use state-first refresh
- if local refresh fails, falls back to full `reload()` for robustness

## Behavioral Result

- WASM actions update diagnostics/state quickly with less visual churn.
- Full reload path remains intact for manual reload and recovery scenarios.

## Validation

1. `node --check MFCMouseEffect/WebUI/app.js` passed.
2. `pnpm run build` (WebUIWorkspace) passed.
3. `Release|x64` MSBuild passed.
