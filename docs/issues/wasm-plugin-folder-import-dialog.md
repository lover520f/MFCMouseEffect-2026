# WASM Plugins: Folder Import (Backend Validation + Copy to Primary Root)

## Background
The previous import flow required users to scan first and then import a selected manifest path.  
The new target is simpler: users pick a plugin folder directly and the host validates/imports it.

## Solution
1. Add an `Import Plugin Folder` action in the WASM panel.
2. Trigger a native folder picker on the backend (not manual path typing in web UI).
3. After selection, backend validates:
   - `plugin.json` exists in the selected folder.
   - manifest content passes schema/ABI checks.
   - manifest `entry` `.wasm` exists and is a regular file.
4. If valid, copy plugin directory to primary root:
   - `%AppData%\MFCMouseEffect\plugins\wasm\<plugin_id_or_dirname>`

## API
- `POST /api/wasm/import-from-folder-dialog`
  - request: `{ "initial_path": "..." }` (optional)
  - response:
    - success: `ok=true` with `manifest_path`, `primary_root_path`
    - cancelled: `ok=false` + `cancelled=true`
    - failed: `ok=false` + `error`

## Implementation
- Native folder picker:
  - `MFCMouseEffect/MouseFx/Core/System/NativeFolderPicker.h`
  - `MFCMouseEffect/MouseFx/Core/System/NativeFolderPicker.cpp`
- Routing:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.Routing.cpp`
- Import validation hardening (entry wasm existence check):
  - `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.cpp`
- UI action and copy:
  - `MFCMouseEffect/WebUI/app.js`
  - `MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
  - `MFCMouseEffect/WebUI/i18n.js`

## Outcome
- No manual manifest path handling for users.
- Invalid plugins are rejected before copy.
- Catalog refreshes after import and auto-selects imported manifest for immediate load.
