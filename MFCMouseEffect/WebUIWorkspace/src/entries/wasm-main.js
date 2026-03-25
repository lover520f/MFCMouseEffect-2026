import WasmPluginFields from '../wasm/WasmPluginFields.svelte';
import PluginManagerFields from '../wasm/PluginManagerFields.svelte';
import {
  emitCursorDecorationChange,
  subscribeCursorDecorationState,
} from '../effects/cursor-decoration-bridge.js';
import { createLazyMountBridge } from './lazy-mount.js';
import { normalizeWasmState } from '../wasm/state-model.js';
import { normalizePolicyRanges } from '../wasm/policy-model.js';

function normalizeWasmSchema(input) {
  const value = input || {};
  return {
    policy_ranges: normalizePolicyRanges(value.policy_ranges || {}),
  };
}

let currentState = normalizeWasmState({});
let currentSchema = normalizeWasmSchema({});
let currentI18n = {};
let currentActionHandler = null;
let currentCursorDecoration = {
  enabled: false,
  plugin_id: 'ring',
  color_hex: '#ff5a5a',
  size_px: 22,
  alpha_percent: 82,
};
let currentCursorDecorationOptions = [];

const bridge = createLazyMountBridge({
  mountId: 'wasm_settings_mount',
  initialProps: {
    cursorDecoration: currentCursorDecoration,
    cursorDecorationOptions: currentCursorDecorationOptions,
    payloadState: currentState,
    i18n: currentI18n,
    onAction: currentActionHandler,
    onCursorDecorationChange: emitCursorDecorationChange,
  },
  createComponent: (mountNode, props) => new WasmPluginFields({
    target: mountNode,
    props,
  }),
});

const pluginManagerBridge = createLazyMountBridge({
  mountId: 'plugin_management_mount',
  initialProps: {
    schemaState: currentSchema,
    payloadState: currentState,
    i18n: currentI18n,
    onAction: currentActionHandler,
  },
  createComponent: (mountNode, props) => new PluginManagerFields({
    target: mountNode,
    props,
  }),
});

function refreshView() {
  bridge.updateProps({
    cursorDecoration: currentCursorDecoration,
    cursorDecorationOptions: currentCursorDecorationOptions,
    payloadState: currentState,
    i18n: currentI18n,
    onAction: currentActionHandler,
    onCursorDecorationChange: emitCursorDecorationChange,
  });
  pluginManagerBridge.updateProps({
    schemaState: currentSchema,
    payloadState: currentState,
    i18n: currentI18n,
    onAction: currentActionHandler,
  });
}

subscribeCursorDecorationState((detail) => {
  currentCursorDecoration = detail?.decoration || currentCursorDecoration;
  currentCursorDecorationOptions = Array.isArray(detail?.pluginOptions)
    ? detail.pluginOptions
    : currentCursorDecorationOptions;
  refreshView();
});

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
