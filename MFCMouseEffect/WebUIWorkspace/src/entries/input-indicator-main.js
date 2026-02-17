import InputIndicatorFields from '../input-indicator/InputIndicatorFields.svelte';
import { createLazyMountBridge } from './lazy-mount.js';

let currentState = {
  enabled: true,
  keyboard_enabled: true,
  position_mode: 'relative',
  offset_x: 40,
  offset_y: 40,
  absolute_x: 40,
  absolute_y: 40,
  target_monitor: 'cursor',
  key_display_mode: 'all',
  per_monitor_overrides: {},
  size_px: 110,
  duration_ms: 320,
};

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
    position_mode: value.position_mode || 'relative',
    offset_x: toNumber(value.offset_x, 40),
    offset_y: toNumber(value.offset_y, 40),
    absolute_x: toNumber(value.absolute_x, 40),
    absolute_y: toNumber(value.absolute_y, 40),
    target_monitor: value.target_monitor || 'cursor',
    key_display_mode: value.key_display_mode || 'all',
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
    monitors: [],
    monitorOverrides: {},
    indicator: currentState,
    texts: {},
  },
  createComponent: (mountNode, props) => {
    const instance = new InputIndicatorFields({
      target: mountNode,
      props,
    });
    instance.$on('change', (event) => {
      const detail = event?.detail || {};
      currentState = normalizeIndicator(detail);
    });
    return instance;
  },
});

function render(payload) {
  const schema = payload?.schema || {};
  const indicator = normalizeIndicator(payload?.indicator || {});
  const texts = payload?.texts || {};

  currentState = indicator;
  bridge.updateProps({
    positionModes: schema.input_indicator_position_modes || [],
    targetMonitorOptions: schema.target_monitor_options || [],
    keyDisplayModes: schema.key_display_modes || [],
    monitors: schema.monitors || [],
    monitorOverrides: indicator.per_monitor_overrides || {},
    indicator,
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
