import WasmPluginFields from '../wasm/WasmPluginFields.svelte';
import { createLazyMountBridge } from './lazy-mount.js';
import { normalizeWasmDiagnostics } from '../wasm/diagnostics-model.js';
import { normalizePolicyRanges } from '../wasm/policy-model.js';

function normalizeWasmState(input) {
  const value = input || {};
  const diagnostics = normalizeWasmDiagnostics(value);
  return {
    enabled: !!value.enabled,
    configured_enabled: !!value.configured_enabled,
    fallback_to_builtin_click: value.fallback_to_builtin_click !== false,
    configured_manifest_path: `${value.configured_manifest_path || ''}`.trim(),
    configured_output_buffer_bytes: Number(value.configured_output_buffer_bytes) || 0,
    configured_max_commands: Number(value.configured_max_commands) || 0,
    configured_max_execution_ms: Number(value.configured_max_execution_ms) || 0,
    runtime_output_buffer_bytes: Number(value.runtime_output_buffer_bytes) || 0,
    runtime_max_commands: Number(value.runtime_max_commands) || 0,
    runtime_max_execution_ms: Number(value.runtime_max_execution_ms) || 0,
    last_call_duration_us: diagnostics.last_call_duration_us,
    last_output_bytes: diagnostics.last_output_bytes,
    last_command_count: diagnostics.last_command_count,
    last_call_exceeded_budget: diagnostics.last_call_exceeded_budget,
    last_call_rejected_by_budget: diagnostics.last_call_rejected_by_budget,
    last_output_truncated_by_budget: diagnostics.last_output_truncated_by_budget,
    last_command_truncated_by_budget: diagnostics.last_command_truncated_by_budget,
    last_budget_reason: diagnostics.last_budget_reason,
    last_parse_error: diagnostics.last_parse_error,
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

function normalizeWasmSchema(input) {
  const value = input || {};
  return {
    policy_ranges: normalizePolicyRanges(value.policy_ranges || {}),
  };
}

let currentState = normalizeWasmState({});
let currentSchema = normalizeWasmSchema({});
let currentI18n = {};
let currentActionHandler = async () => ({ ok: false, error: 'wasm action handler unavailable' });

const bridge = createLazyMountBridge({
  mountId: 'wasm_settings_mount',
  initialProps: {
    schemaState: currentSchema,
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
    schemaState: currentSchema,
    payloadState: currentState,
    i18n: currentI18n,
    onAction: currentActionHandler,
  });
}

function render(payload) {
  const value = payload || {};
  currentSchema = normalizeWasmSchema(value.schema || {});
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
