import ActiveEffectsFields from '../effects/ActiveEffectsFields.svelte';
import { createLazyMountBridge } from './lazy-mount.js';

let currentState = {
  click: '',
  trail: '',
  scroll: '',
  hold: '',
  hover: '',
};

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

const bridge = createLazyMountBridge({
  mountId: 'effects_settings_mount',
  initialProps: {
    clickOptions: [],
    trailOptions: [],
    scrollOptions: [],
    holdOptions: [],
    hoverOptions: [],
    active: currentState,
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
  currentState = active;
  bridge.updateProps({
    clickOptions: schema.effects?.click || [],
    trailOptions: schema.effects?.trail || [],
    scrollOptions: schema.effects?.scroll || [],
    holdOptions: schema.effects?.hold || [],
    hoverOptions: schema.effects?.hover || [],
    active,
  });
}

function read() {
  return normalizeActive(currentState);
}

window.MfxEffectsSection = {
  render,
  read,
};
