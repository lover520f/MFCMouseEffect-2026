# WASM Settings Init Race: `wasm action handler unavailable`

## Symptom

On first open of the WASM settings section, an operation result error appears even without manual action:
- `wasm action handler unavailable`

## Root Cause

`WasmPluginFields.svelte` requests catalog data on first mount.
At that moment, the real action callback is not bound yet, and the old placeholder handler was still active, returning the fallback error text.

## Fix

1. `MFCMouseEffect/WebUIWorkspace/src/entries/wasm-main.js`
- default action handler changed from placeholder function to `null`.

2. `MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
- default `onAction` changed to `null`.
- `runAction` now returns a clear `wasm_action_not_ready` message when handler is not bound yet.

3. `MFCMouseEffect/WebUI/i18n.js`
- added `wasm_action_not_ready` in both English and Chinese catalogs.

## Validation

1. Open WASM settings section: no automatic placeholder error should appear.
2. Click `Refresh Catalog`: request should use real `/api/wasm/catalog` action path.
3. If runtime bridge is missing, the UI should only show runtime fallback diagnostics (e.g. missing `mfx_wasm_runtime.dll`), not action-handler placeholder errors.
