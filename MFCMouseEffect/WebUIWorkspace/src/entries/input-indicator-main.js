import InputIndicatorFields from '../input-indicator/InputIndicatorFields.svelte';
import { createLazyMountBridge } from './lazy-mount.js';
import { readUiState, writeUiState } from './ui-state-storage.js';
import { mountLegacyComponent } from './legacy-component.js';

const INPUT_INDICATOR_UI_STATE_STORAGE_NS = 'input-indicator.v1';

let currentState = {
  enabled: true,
  keyboard_enabled: true,
  render_mode: 'native',
  wasm_fallback_to_native: true,
  wasm_manifest_path: '',
  position_mode: 'relative',
  offset_x: 40,
  offset_y: 40,
  absolute_x: 40,
  absolute_y: 40,
  target_monitor: 'cursor',
  key_display_mode: 'all',
  key_label_layout_mode: 'fixed_font',
  per_monitor_overrides: {},
  size_px: 110,
  duration_ms: 320,
};
let currentWasmState = {};
let currentWasmAction = null;
let currentActiveTab = 'basic';

function normalizeActiveTab(value) {
  return `${value || ''}`.trim().toLowerCase() === 'plugin' ? 'plugin' : 'basic';
}

function readInputIndicatorUiState() {
  return readUiState(INPUT_INDICATOR_UI_STATE_STORAGE_NS);
}

function writeInputIndicatorUiState(nextState) {
  writeUiState(INPUT_INDICATOR_UI_STATE_STORAGE_NS, nextState);
}

{
  const persisted = readInputIndicatorUiState();
  if (persisted?.activeTab) {
    currentActiveTab = normalizeActiveTab(persisted.activeTab);
  }
}

function toNumber(value, fallback) {
  const parsed = Number(value);
  if (Number.isFinite(parsed)) return parsed;
  return fallback;
}

function normalizeIndicator(input) {
  const value = input || {};
  return {
    enabled: value.enabled !== false,
    keyboard_enabled: value.keyboard_enabled !== false,
    render_mode: value.render_mode || 'native',
    wasm_fallback_to_native: value.wasm_fallback_to_native !== false,
    wasm_manifest_path: `${value.wasm_manifest_path || ''}`.trim(),
    position_mode: value.position_mode || 'relative',
    offset_x: toNumber(value.offset_x, 40),
    offset_y: toNumber(value.offset_y, 40),
    absolute_x: toNumber(value.absolute_x, 40),
    absolute_y: toNumber(value.absolute_y, 40),
    target_monitor: value.target_monitor || 'cursor',
    key_display_mode: value.key_display_mode || 'all',
    key_label_layout_mode: value.key_label_layout_mode || 'fixed_font',
    per_monitor_overrides: value.per_monitor_overrides || {},
    size_px: toNumber(value.size_px, 110),
    duration_ms: toNumber(value.duration_ms, 320),
  };
}

const bridge = createLazyMountBridge({
  mountId: 'input_indicator_settings_mount',
  initialProps: {
    positionModes: [],
    targetMonitorOptions: [],
    keyDisplayModes: [],
    keyLabelLayoutModes: [],
    monitors: [],
    monitorOverrides: {},
    indicator: currentState,
    wasmState: currentWasmState,
    onWasmAction: currentWasmAction,
    activeTab: currentActiveTab,
    texts: {},
  },
  createComponent: (mountNode, props) => {
    return mountLegacyComponent(InputIndicatorFields, mountNode, {
      ...props,
      onChangeState: (detail) => {
        currentState = normalizeIndicator(detail);
      },
      onTabChange: (detail) => {
        currentActiveTab = normalizeActiveTab(detail?.tabId);
        writeInputIndicatorUiState({ activeTab: currentActiveTab });
      },
    });
  },
});

function render(payload) {
  const schema = payload?.schema || {};
  const indicator = normalizeIndicator(payload?.indicator || {});
  const texts = payload?.texts || {};
  const wasmState = payload?.wasmState || {};
  const onWasmAction = (typeof payload?.onWasmAction === 'function')
    ? payload.onWasmAction
    : null;

  currentState = indicator;
  currentWasmState = wasmState;
  currentWasmAction = onWasmAction;
  bridge.updateProps({
    positionModes: schema.input_indicator_position_modes || [],
    targetMonitorOptions: schema.target_monitor_options || [],
    keyDisplayModes: schema.key_display_modes || [],
    keyLabelLayoutModes: schema.key_label_layout_modes || [],
    monitors: schema.monitors || [],
    monitorOverrides: indicator.per_monitor_overrides || {},
    indicator,
    wasmState,
    onWasmAction,
    activeTab: currentActiveTab,
    texts,
  });
}

function read() {
  return normalizeIndicator(currentState);
}

function syncIndicatorPositionUi() {
  // Svelte component state drives UI; compatibility no-op.
}

window.MfxInputIndicatorSection = {
  render,
  read,
  syncIndicatorPositionUi,
};
