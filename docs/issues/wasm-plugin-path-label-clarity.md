# WASM Plugin Panel: Naming Clarity and Information Structure

## Background
There were two readability issues:
- Section title `WASM Plugin` was too technical for general users.
- `Plugin Catalog` and `Manifest path + synced badge` wording was implementation-oriented.

## Changes
1. Section naming:
- `section_wasm_plugin`: `WASM Plugin` -> `Effect Plugins`.

2. Catalog block naming:
- `title_wasm_block_catalog`: `Plugin Catalog` -> `Plugin Info`.
- `btn_wasm_refresh_catalog`: `Refresh Catalog` -> `Refresh Plugin List`.
- `label_wasm_catalog_errors`: `Catalog errors` -> `Plugin scan errors`.
- `label_wasm_catalog_roots`: `Catalog roots` -> `Plugin scan roots`.

3. Path fields:
- Remove sync badge (`Synced with configured path`).
- `label_wasm_manifest_path` -> `Current plugin path`.
- `label_wasm_configured_manifest_path` -> `Plugin config path`.

## Outcome
- User-facing copy now follows task semantics instead of internal terms.
- Better scan flow: section -> plugin info -> path/diagnostics.
- Lower visual noise by removing low-value sync indicators.

## Files
- `MFCMouseEffect/WebUIWorkspace/src/shell/sections.js`
- `MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
- `MFCMouseEffect/WebUI/i18n.js`
