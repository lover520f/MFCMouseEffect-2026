# WASM Panel Button Clarity + Hover Tooltips

## Background
- `Save Catalog Root` and `Save Policy` were easy to confuse.
- The WASM panel has many actions, but action intent was not obvious without hover help.

## Changes
- Button wording cleanup:
  - `Save Catalog Root` -> `Apply Scan Path`
  - `Save Policy` -> `Save Fallback & Budget`
- Runtime control cleanup:
  - Replaced separate `Enable` / `Disable` buttons with a single toggle control.
  - Toggle label now exposes state directly: `Using Plugin` / `Plugin Off`.
- Added i18n-aware hover tooltips (`title` + `data-i18n-title`) to all core WASM action buttons:
  - Apply scan path, refresh plugin list, load selected plugin, import plugin folder, export all
  - Runtime toggle, reload plugin, save fallback & budget, reset defaults

## Behavior Clarification
- `Apply Scan Path`: persists `catalog_root_path` and immediately rescans plugin discovery.
- `Save Fallback & Budget`: persists fallback behavior and runtime budget limits to `config.json`.

## Files
- `MFCMouseEffect/WebUI/i18n.js`
- `MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
