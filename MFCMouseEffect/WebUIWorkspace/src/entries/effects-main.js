import EffectsSectionTabs from '../effects/EffectsSectionTabs.svelte';
import { normalizeEffectsProfile } from '../effects/profile-model.js';
import { createLazyMountBridge } from './lazy-mount.js';

let currentState = {
  click: '',
  trail: '',
  scroll: '',
  hold: '',
  hover: '',
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

function normalizeActiveTab(input) {
  const value = `${input || ''}`.trim().toLowerCase();
  if (value === 'text') {
    return 'text';
  }
  if (value === 'trail') {
    return 'trail';
  }
  return 'active';
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
      effectCapabilities: currentCapabilities,
      active: currentState,
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
      currentState = normalizeActive(detail);
    });
    instance.$on('tabChange', (event) => {
      currentActiveTab = normalizeActiveTab(event?.detail?.tabId);
    });
    return instance;
  },
});

function render(payload) {
  const schema = payload?.schema || {};
  const appState = payload?.state || {};
  const active = normalizeActive(appState.active || {});
  const effectCapabilities = normalizeEffectCapabilities(schema.capabilities?.effects || {});
  const effectsProfile = normalizeEffectsProfile(appState.effects_profile || {});
  showEffectsProfile = resolveEffectsProfileDebugFlag();
  currentState = active;
  currentCapabilities = effectCapabilities;
  currentEffectsProfile = effectsProfile;
  bridge.updateProps({
    activeTab: currentActiveTab,
    effectProps: {
      clickOptions: schema.effects?.click || [],
      trailOptions: schema.effects?.trail || [],
      scrollOptions: schema.effects?.scroll || [],
      holdOptions: schema.effects?.hold || [],
      hoverOptions: schema.effects?.hover || [],
      effectCapabilities,
      active,
      effectsProfile,
      showEffectsProfile,
    },
  });
}

function read() {
  return normalizeActive(currentState);
}

window.MfxEffectsSection = {
  render,
  read,
};
