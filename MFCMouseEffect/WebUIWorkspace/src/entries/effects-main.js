import ActiveEffectsFields from '../effects/ActiveEffectsFields.svelte';
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
    clickOptions: [],
    trailOptions: [],
    scrollOptions: [],
    holdOptions: [],
    hoverOptions: [],
    effectCapabilities: currentCapabilities,
    active: currentState,
    effectsProfile: currentEffectsProfile,
  },
  createComponent: (mountNode, props) => {
    const instance = new ActiveEffectsFields({
      target: mountNode,
      props,
    });
    instance.$on('change', (event) => {
      const detail = event?.detail || {};
      currentState = normalizeActive(detail);
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
  currentState = active;
  currentCapabilities = effectCapabilities;
  currentEffectsProfile = effectsProfile;
  bridge.updateProps({
    clickOptions: schema.effects?.click || [],
    trailOptions: schema.effects?.trail || [],
    scrollOptions: schema.effects?.scroll || [],
    holdOptions: schema.effects?.hold || [],
    hoverOptions: schema.effects?.hover || [],
    effectCapabilities,
    active,
    effectsProfile,
  });
}

function read() {
  return normalizeActive(currentState);
}

window.MfxEffectsSection = {
  render,
  read,
};
