import GeneralSettingsFields from '../general/GeneralSettingsFields.svelte';
import { createLazyMountBridge } from './lazy-mount.js';

let currentState = {
  ui_language: '',
  theme: '',
  hold_follow_mode: 'smooth',
  hold_presenter_backend: 'auto',
};

function normalizeGeneral(input) {
  const value = input || {};
  return {
    ui_language: value.ui_language || '',
    theme: value.theme || '',
    hold_follow_mode: value.hold_follow_mode || 'smooth',
    hold_presenter_backend: value.hold_presenter_backend || 'auto',
  };
}

const bridge = createLazyMountBridge({
  mountId: 'general_settings_mount',
  initialProps: {
    uiLanguages: [],
    themes: [],
    holdFollowModes: [],
    holdPresenterBackends: [],
    general: currentState,
  },
  createComponent: (mountNode, props) => {
    const instance = new GeneralSettingsFields({
      target: mountNode,
      props,
    });
    instance.$on('change', (event) => {
      const detail = event?.detail || {};
      currentState = normalizeGeneral(detail);
    });
    return instance;
  },
});

function render(payload) {
  const schema = payload?.schema || {};
  const appState = payload?.state || {};
  const general = normalizeGeneral(appState);
  currentState = general;
  bridge.updateProps({
    uiLanguages: schema.ui_languages || [],
    themes: schema.themes || [],
    holdFollowModes: schema.hold_follow_modes || [],
    holdPresenterBackends: schema.hold_presenter_backends || [],
    general,
  });
}

function read() {
  return normalizeGeneral(currentState);
}

window.MfxGeneralSection = {
  render,
  read,
};
