const ERROR_CODE_MESSAGES = {
  no_controller: ['wasm_error_no_controller', 'Controller is unavailable.'],
  wasm_host_unavailable: ['wasm_error_wasm_host_unavailable', 'WASM host is unavailable.'],
  unknown_error: ['wasm_error_unknown_error', 'Unknown WASM error.'],
  load_manifest_failed: ['wasm_error_load_manifest_failed', 'Failed to switch WASM manifest.'],
  reload_failed: ['wasm_error_reload_failed', 'Failed to reload current WASM plugin.'],
  reload_target_missing: ['wasm_error_reload_target_missing', 'No active WASM plugin target to reload.'],
  runtime_unavailable: ['wasm_error_runtime_unavailable', 'WASM runtime is unavailable.'],
  module_load_failed: ['wasm_error_module_load_failed', 'Failed to load WASM module from entry file.'],
  api_version_call_failed: ['wasm_error_api_version_call_failed', 'Failed to read plugin API version from WASM module.'],
  api_version_unsupported: ['wasm_error_api_version_unsupported', 'WASM module API version is unsupported.'],
  manifest_io_error: ['wasm_error_manifest_io_error', 'Failed to read plugin manifest file.'],
  manifest_json_parse_error: ['wasm_error_manifest_json_parse_error', 'Plugin manifest JSON is invalid.'],
  manifest_invalid: ['wasm_error_manifest_invalid', 'Plugin manifest is invalid.'],
  manifest_api_unsupported: ['wasm_error_manifest_api_unsupported', 'Manifest API version is unsupported by current host.'],
  entry_wasm_path_invalid: ['wasm_error_entry_wasm_path_invalid', 'Manifest entry wasm path is invalid.'],
  manifest_path_required: ['wasm_error_manifest_path_required', 'Plugin manifest path is required.'],
  manifest_path_not_found: ['wasm_error_manifest_path_not_found', 'Plugin manifest path does not exist.'],
  manifest_path_not_file: ['wasm_error_manifest_path_not_file', 'Plugin manifest path is not a file.'],
  manifest_load_failed: ['wasm_error_manifest_load_failed', 'Failed to load plugin manifest.'],
  source_entry_invalid: ['wasm_error_source_entry_invalid', 'Source plugin entry file is invalid.'],
  primary_root_resolve_failed: ['wasm_error_primary_root_resolve_failed', 'Failed to resolve primary plugin root.'],
  copy_failed: ['wasm_error_copy_failed', 'Failed to copy plugin files.'],
  destination_manifest_missing: ['wasm_error_destination_manifest_missing', 'Destination plugin manifest is missing.'],
  destination_entry_invalid: ['wasm_error_destination_entry_invalid', 'Destination plugin entry file is invalid.'],
  no_plugins_discovered: ['wasm_error_no_plugins_discovered', 'No plugins were discovered for export.'],
  export_root_resolve_failed: ['wasm_error_export_root_resolve_failed', 'Failed to resolve plugin export root.'],
  create_export_directory_failed: ['wasm_error_create_export_directory_failed', 'Failed to create plugin export directory.'],
  no_plugin_copied: ['wasm_error_no_plugin_copied', 'No plugin was copied during export.'],
  selected_folder_manifest_missing: ['wasm_error_selected_folder_manifest_missing', 'plugin.json is missing in selected folder.'],
  folder_picker_cancelled: ['wasm_error_folder_picker_cancelled', 'Folder selection was cancelled.'],
  folder_picker_failed: ['wasm_error_folder_picker_failed', 'Folder picker failed.'],
  native_folder_picker_not_supported: ['wasm_error_native_folder_picker_not_supported', 'Native folder picker is not supported on this platform.'],
};

export function normalizeActionErrorCode(value) {
  return `${value || ''}`.trim().toLowerCase();
}

export function resolveWasmActionErrorMessage(errorCode, translate) {
  const code = normalizeActionErrorCode(errorCode);
  if (!code) {
    return '';
  }
  const entry = ERROR_CODE_MESSAGES[code];
  if (!entry) {
    return '';
  }
  const [key, fallback] = entry;
  if (typeof translate === 'function') {
    return translate(key, fallback);
  }
  return fallback;
}

export function listWasmActionErrorI18nKeys() {
  const keys = [];
  for (const [, value] of Object.entries(ERROR_CODE_MESSAGES)) {
    const [i18nKey] = value;
    keys.push(i18nKey);
  }
  return keys;
}
