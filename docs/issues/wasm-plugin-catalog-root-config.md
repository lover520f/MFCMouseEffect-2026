# WASM Plugin Catalog Root Configurable

## Background
- The WASM panel could only scan fixed roots (AppData primary root + built-in roots).
- Users could not persist a custom scan directory from settings.

## Goal
- Allow configuring an optional plugin catalog root from Web settings.
- Persist it in `config.json`.
- Make catalog scan and export-all both honor this configured root.

## Changes
- Added `wasm.catalog_root_path` in config model and JSON codec.
- Added runtime policy wiring:
  - `wasm_set_policy` command supports `catalog_root_path`.
  - `/api/wasm/policy` accepts and forwards `catalog_root_path`.
- Added catalog root support in path resolution:
  - `WasmPluginPaths::ResolveSearchRoots(const std::wstring& configuredRoot)`.
- Updated catalog/export APIs:
  - `/api/wasm/catalog` now scans with configured root included.
  - `/api/wasm/export-all` now exports from the same root set.
- Updated WASM panel UI:
  - New input + action button to save catalog root.
  - New status feedback for save/clear result.
  - Displays current configured catalog root path in metadata area.

## Key Files
- `MFCMouseEffect/MouseFx/Core/Config/EffectConfig.h`
- `MFCMouseEffect/MouseFx/Core/Config/EffectConfig.Internal.cpp`
- `MFCMouseEffect/MouseFx/Core/Config/EffectConfigJsonKeys.Wasm.h`
- `MFCMouseEffect/MouseFx/Core/Config/EffectConfigJsonCodec.Parse.Wasm.cpp`
- `MFCMouseEffect/MouseFx/Core/Config/EffectConfigJsonCodec.Serialize.Wasm.cpp`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginPaths.h`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginPaths.cpp`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.h`
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginTransferService.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.h`
- `MFCMouseEffect/MouseFx/Core/Control/AppController.Wasm.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/CommandHandler.cpp`
- `MFCMouseEffect/MouseFx/Core/Control/CommandHandler.ApplySettings.cpp`
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.Routing.cpp`
- `MFCMouseEffect/MouseFx/Server/SettingsSchemaBuilder.cpp`
- `MFCMouseEffect/MouseFx/Server/SettingsStateMapper.cpp`
- `MFCMouseEffect/WebUIWorkspace/src/wasm/WasmPluginFields.svelte`
- `MFCMouseEffect/WebUIWorkspace/src/entries/wasm-main.js`
- `MFCMouseEffect/WebUI/i18n.js`
- `MFCMouseEffect/WebUI/styles.css`

## Behavior Notes
- Empty `catalog_root_path` means fallback to default scan roots only.
- Configured root is included in catalog root display and used by export-all.
