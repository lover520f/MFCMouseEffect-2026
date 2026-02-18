# WASM Web Settings: Plugin Import/Export (Phase3g)

## Goal
Add practical plugin management actions:
- Import selected plugin into the primary plugin root.
- Export all discovered plugins to a timestamped export directory.

## Behavior
1. Import selected
- UI action: `Import To Primary`
- Source: currently selected catalog manifest path.
- Host copies the plugin folder (manifest parent directory) into primary root:
  - `%AppData%\\MFCMouseEffect\\plugins\\wasm\\<plugin_id_or_dirname>`
- Response includes destination manifest path and primary root path.

2. Export all
- UI action: `Export All`
- Source: all currently discovered plugins from catalog scan roots.
- Destination:
  - `%AppData%\\MFCMouseEffect\\exports\\wasm\\all-YYYYMMDD-HHMMSS`
- Response includes export directory path and exported plugin count.

## API
- `POST /api/wasm/import-selected`
  - payload: `{ "manifest_path": "..." }`
  - response: `ok`, `error`, `manifest_path`, `primary_root_path`
- `POST /api/wasm/export-all`
  - payload: `{}`
  - response: `ok`, `error`, `export_path`, `count`

## Implementation
- New service:
  - `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.h`
  - `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.cpp`
- Server routing:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.Routing.cpp`
- UI action wiring:
  - `MFCMouseEffect/WebUI/app.js`
  - `MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
  - `MFCMouseEffect/WebUI/i18n.js`

## Notes
- Import uses copy-into-primary strategy, not move.
- Export writes a snapshot directory (no zip in this phase).
- If no plugin is discovered, export returns `ok=false` with a clear error.
