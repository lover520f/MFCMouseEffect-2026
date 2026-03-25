import EffectsSectionTabs from '../effects/EffectsSectionTabs.svelte';
import { normalizeEffectsProfile } from '../effects/profile-model.js';
import { normalizeRuntimePlatform } from '../automation/platform.js';
import { createLazyMountBridge } from './lazy-mount.js';
import { readUiState, writeUiState } from './ui-state-storage.js';

const EFFECTS_UI_STATE_STORAGE_NS = 'effects.v1';

let currentActiveState = {
  click: '',
  trail: '',
  scroll: '',
  hold: '',
  hover: '',
};
let currentCursorDecoration = {
  enabled: false,
  plugin_id: 'ring',
  color_hex: '#ff5a5a',
  size_px: 22,
  alpha_percent: 82,
};
let currentSizeScales = {
  click: 100,
  trail: 100,
  scroll: 100,
  hold: 100,
  hover: 100,
};

let currentCapabilities = {
  click: true,
  trail: true,
  scroll: true,
  hold: true,
  hover: true,
};
let currentEffectsProfile = {};
let currentActiveTab = 'active';
let showEffectsProfile = false;
let currentClickOptions = [];
let currentTrailOptions = [];
let currentScrollOptions = [];
let currentHoldOptions = [];
let currentHoverOptions = [];
let currentCursorDecorationOptions = [];
let currentEffectConflictPolicy = {
  hold_move_policy: 'hold_only',
};
let currentEffectConflictPolicyOptions = {
  hold_move_policy: [],
};
let currentEffectsBlacklistApps = [];
let pendingEffectsBlacklistApps = null;
let currentRuntimePlatform = 'windows';

function normalizeActiveTab(input) {
  const value = `${input || ''}`.trim().toLowerCase();
  if (value === 'text') {
    return 'text';
  }
  if (value === 'trail') {
    return 'trail';
  }
  if (value === 'size') {
    return 'size';
  }
  if (value === 'conflict') {
    return 'conflict';
  }
  if (value === 'blacklist') {
    return 'blacklist';
  }
  if (value === 'plugin' || value === 'wasm') {
    return 'plugin';
  }
  return 'active';
}

function readEffectsUiState() {
  return readUiState(EFFECTS_UI_STATE_STORAGE_NS);
}

function writeEffectsUiState(nextState) {
  writeUiState(EFFECTS_UI_STATE_STORAGE_NS, nextState);
}

{
  const persisted = readEffectsUiState();
  if (persisted?.activeTab) {
    currentActiveTab = normalizeActiveTab(persisted.activeTab);
  }
}

function normalizeDebugFlag(value) {
  const text = `${value || ''}`.trim().toLowerCase();
  return text === '1' || text === 'true' || text === 'yes' || text === 'on';
}

function resolveEffectsProfileDebugFlag() {
  if (typeof window === 'undefined' || !window.location) {
    return false;
  }
  const query = new URLSearchParams(window.location.search || '');
  if (query.has('effects_profile_debug')) {
    return normalizeDebugFlag(query.get('effects_profile_debug'));
  }
  if (query.has('debug')) {
    return normalizeDebugFlag(query.get('debug'));
  }
  return false;
}

function normalizeActive(input) {
  const value = input || {};
  return {
    click: value.click || '',
    trail: value.trail || '',
    scroll: value.scroll || '',
    hold: value.hold || '',
    hover: value.hover || '',
  };
}

function normalizeCursorDecoration(input) {
  const value = input || {};
  const toSafeNumber = (candidate, fallback) => {
    const parsed = Number(candidate);
    return Number.isFinite(parsed) ? parsed : fallback;
  };
  return {
    enabled: value.enabled === true,
    plugin_id: `${value.plugin_id || 'ring'}`.trim() || 'ring',
    color_hex: `${value.color_hex || '#ff5a5a'}`.trim() || '#ff5a5a',
    size_px: toSafeNumber(value.size_px, 22),
    alpha_percent: toSafeNumber(value.alpha_percent, 82),
  };
}

function clampScalePercent(value) {
  const parsed = Number(value);
  const safe = Number.isFinite(parsed) ? Math.round(parsed) : 100;
  return Math.min(200, Math.max(50, safe));
}

function normalizeEffectSizeScales(input) {
  const value = input || {};
  return {
    click: clampScalePercent(value.click),
    trail: clampScalePercent(value.trail),
    scroll: clampScalePercent(value.scroll),
    hold: clampScalePercent(value.hold),
    hover: clampScalePercent(value.hover),
  };
}

function normalizeEffectCapabilities(input) {
  const value = input || {};
  return {
    click: value.click !== false,
    trail: value.trail !== false,
    scroll: value.scroll !== false,
    hold: value.hold !== false,
    hover: value.hover !== false,
  };
}

function normalizeEffectConflictPolicy(input) {
  const value = input || {};
  return {
    hold_move_policy: value.hold_move_policy || value.hold_move || 'hold_only',
  };
}

function normalizeEffectConflictPolicyOptions(input) {
  const value = input || {};
  const normalizeList = (key) => (Array.isArray(value[key]) ? value[key] : []);
  return {
    hold_move_policy: normalizeList('hold_move_policy').length
      ? normalizeList('hold_move_policy')
      : normalizeList('hold_move'),
  };
}

function normalizeEffectsBlacklistApps(input) {
  const source = Array.isArray(input) ? input : [];
  const out = [];
  for (const item of source) {
    const normalized = `${item || ''}`.trim().toLowerCase();
    if (!normalized || out.includes(normalized)) {
      continue;
    }
    out.push(normalized);
  }
  return out;
}

function sameStringList(left, right) {
  const a = Array.isArray(left) ? left : [];
  const b = Array.isArray(right) ? right : [];
  if (a.length !== b.length) {
    return false;
  }
  for (let i = 0; i < a.length; i += 1) {
    if (a[i] !== b[i]) {
      return false;
    }
  }
  return true;
}

const bridge = createLazyMountBridge({
  mountId: 'effects_settings_mount',
  initialProps: {
    activeTab: currentActiveTab,
    effectProps: {
      clickOptions: [],
      trailOptions: [],
      scrollOptions: [],
      holdOptions: [],
      hoverOptions: [],
      cursorDecorationOptions: [],
      cursorDecoration: currentCursorDecoration,
      effectCapabilities: currentCapabilities,
      active: currentActiveState,
      effectSizeScales: currentSizeScales,
      effectConflictPolicy: currentEffectConflictPolicy,
      effectConflictPolicyOptions: currentEffectConflictPolicyOptions,
      effectsBlacklistApps: currentEffectsBlacklistApps,
      platform: currentRuntimePlatform,
      effectsProfile: currentEffectsProfile,
      showEffectsProfile,
    },
  },
  createComponent: (mountNode, props) => {
    const instance = new EffectsSectionTabs({
      target: mountNode,
      props,
    });
    instance.$on('activeChange', (event) => {
      const detail = event?.detail || {};
      currentActiveState = normalizeActive(detail.active || detail);
      currentCursorDecoration = normalizeCursorDecoration(
        detail.cursor_decoration || currentCursorDecoration,
      );
    });
    instance.$on('cursorDecorationChange', (event) => {
      currentCursorDecoration = normalizeCursorDecoration(event?.detail || {});
      syncBridgeProps();
    });
    instance.$on('sizeChange', (event) => {
      const detail = event?.detail || {};
      currentSizeScales = normalizeEffectSizeScales(detail);
    });
    instance.$on('tabChange', (event) => {
      currentActiveTab = normalizeActiveTab(event?.detail?.tabId);
      writeEffectsUiState({ activeTab: currentActiveTab });
    });
    instance.$on('conflictPolicyChange', (event) => {
      const detail = event?.detail || {};
      currentEffectConflictPolicy = normalizeEffectConflictPolicy(detail);
    });
    instance.$on('blacklistChange', (event) => {
      const detail = event?.detail || {};
      const localBlacklistApps = normalizeEffectsBlacklistApps(detail.effects_blacklist_apps);
      currentEffectsBlacklistApps = localBlacklistApps;
      pendingEffectsBlacklistApps = localBlacklistApps;
      syncBridgeProps();
    });
    return instance;
  },
});

function syncBridgeProps() {
  bridge.updateProps({
    activeTab: currentActiveTab,
    effectProps: {
      clickOptions: currentClickOptions,
      trailOptions: currentTrailOptions,
      scrollOptions: currentScrollOptions,
      holdOptions: currentHoldOptions,
      hoverOptions: currentHoverOptions,
      cursorDecorationOptions: currentCursorDecorationOptions,
      cursorDecoration: currentCursorDecoration,
      effectCapabilities: currentCapabilities,
      active: currentActiveState,
      effectSizeScales: currentSizeScales,
      effectConflictPolicy: currentEffectConflictPolicy,
      effectConflictPolicyOptions: currentEffectConflictPolicyOptions,
      effectsBlacklistApps: currentEffectsBlacklistApps,
      platform: currentRuntimePlatform,
      effectsProfile: currentEffectsProfile,
      showEffectsProfile,
    },
  });
}

function render(payload) {
  const schema = payload?.schema || {};
  const appState = payload?.state || {};
  const active = normalizeActive(appState.active || {});
  const effectSizeScales = normalizeEffectSizeScales(appState.effect_size_scales || {});
  const effectCapabilities = normalizeEffectCapabilities(schema.capabilities?.effects || {});
  const effectConflictPolicy = normalizeEffectConflictPolicy(appState.effect_conflict_policy || {});
  const effectConflictPolicyOptions = normalizeEffectConflictPolicyOptions(
    schema.effect_conflict_policy_options || {},
  );
  const runtimePlatform = normalizeRuntimePlatform(schema?.capabilities?.platform);
  const effectsProfile = normalizeEffectsProfile(appState.effects_profile || {});
  const backendBlacklistApps = normalizeEffectsBlacklistApps(appState.effects_blacklist_apps);
  if (pendingEffectsBlacklistApps && sameStringList(pendingEffectsBlacklistApps, backendBlacklistApps)) {
    pendingEffectsBlacklistApps = null;
  }
  const effectiveBlacklistApps = pendingEffectsBlacklistApps || backendBlacklistApps;
  showEffectsProfile = resolveEffectsProfileDebugFlag();
  currentActiveState = active;
  currentSizeScales = effectSizeScales;
  currentCapabilities = effectCapabilities;
  currentEffectConflictPolicy = effectConflictPolicy;
  currentEffectConflictPolicyOptions = effectConflictPolicyOptions;
  currentEffectsBlacklistApps = effectiveBlacklistApps;
  currentRuntimePlatform = runtimePlatform;
  currentEffectsProfile = effectsProfile;
  currentClickOptions = schema.effects?.click || [];
  currentTrailOptions = schema.effects?.trail || [];
  currentScrollOptions = schema.effects?.scroll || [];
  currentHoldOptions = schema.effects?.hold || [];
  currentHoverOptions = schema.effects?.hover || [];
  currentCursorDecorationOptions = schema.cursor_decoration?.plugins || [];
  currentCursorDecoration = normalizeCursorDecoration(appState.cursor_decoration || {});
  syncBridgeProps();
}

function read() {
  return {
    active: normalizeActive(currentActiveState),
    cursor_decoration: normalizeCursorDecoration(currentCursorDecoration),
    effect_size_scales: normalizeEffectSizeScales(currentSizeScales),
    effect_conflict_policy: normalizeEffectConflictPolicy(currentEffectConflictPolicy),
    effects_blacklist_apps: normalizeEffectsBlacklistApps(currentEffectsBlacklistApps),
  };
}

function setActiveTab(tabId) {
  currentActiveTab = normalizeActiveTab(tabId);
  writeEffectsUiState({ activeTab: currentActiveTab });
  syncBridgeProps();
}

window.MfxEffectsSection = {
  render,
  read,
  setActiveTab,
};
