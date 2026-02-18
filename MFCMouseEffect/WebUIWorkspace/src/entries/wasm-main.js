import WasmPluginFields from '../wasm/WasmPluginFields.svelte';
import { createLazyMountBridge } from './lazy-mount.js';

function normalizeWasmState(input) {
  const value = input || {};
  return {
    enabled: !!value.enabled,
    runtime_backend: `${value.runtime_backend || ''}`.trim(),
    runtime_fallback_reason: `${value.runtime_fallback_reason || ''}`.trim(),
    plugin_loaded: !!value.plugin_loaded,
    plugin_api_version: Number(value.plugin_api_version) || 0,
    active_plugin_id: `${value.active_plugin_id || ''}`.trim(),
    active_plugin_name: `${value.active_plugin_name || ''}`.trim(),
    active_manifest_path: `${value.active_manifest_path || ''}`.trim(),
    active_wasm_path: `${value.active_wasm_path || ''}`.trim(),
    last_rendered_by_wasm: !!value.last_rendered_by_wasm,
    last_executed_text_commands: Number(value.last_executed_text_commands) || 0,
    last_executed_image_commands: Number(value.last_executed_image_commands) || 0,
    last_dropped_render_commands: Number(value.last_dropped_render_commands) || 0,
    last_render_error: `${value.last_render_error || ''}`.trim(),
    last_error: `${value.last_error || ''}`.trim(),
  };
}

let currentState = normalizeWasmState({});
let currentI18n = {};
let currentActionHandler = async () => ({ ok: false, error: 'wasm action handler unavailable' });

const bridge = createLazyMountBridge({
  mountId: 'wasm_settings_mount',
  initialProps: {
    payloadState: currentState,
    i18n: currentI18n,
    onAction: currentActionHandler,
  },
  createComponent: (mountNode, props) => new WasmPluginFields({
    target: mountNode,
    props,
  }),
});

function refreshView() {
  bridge.updateProps({
    payloadState: currentState,
    i18n: currentI18n,
    onAction: currentActionHandler,
  });
}

function render(payload) {
  const value = payload || {};
  currentState = normalizeWasmState(value.state || {});
  currentI18n = value.i18n || {};
  currentActionHandler = typeof value.onAction === 'function'
    ? value.onAction
    : currentActionHandler;
  refreshView();
}

function syncI18n(i18n) {
  currentI18n = i18n || {};
  refreshView();
}

window.MfxWasmSection = {
  render,
  syncI18n,
};
