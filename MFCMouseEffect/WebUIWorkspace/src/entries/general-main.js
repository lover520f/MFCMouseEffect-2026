import GeneralSettingsFields from '../general/GeneralSettingsFields.svelte';
import { normalizeGeneralState } from '../general/general-state-model.js';
import { createLazyMountBridge } from './lazy-mount.js';

let currentState = normalizeGeneralState({});
let currentActionHandler = null;

const bridge = createLazyMountBridge({
  mountId: 'general_settings_mount',
  initialProps: {
    uiLanguages: [],
    themes: [],
    holdFollowModes: [],
    holdPresenterBackends: [],
    general: currentState,
    onAction: currentActionHandler,
  },
  createComponent: (mountNode, props) => {
    const instance = new GeneralSettingsFields({
      target: mountNode,
      props,
    });
    instance.$on('change', (event) => {
      const detail = event?.detail || {};
      currentState = normalizeGeneralState(detail);
    });
    return instance;
  },
});

function render(payload) {
  const schema = payload?.schema || {};
  const appState = payload?.state || {};
  const general = normalizeGeneralState(appState);
  currentState = general;
  currentActionHandler = typeof payload?.onAction === 'function'
    ? payload.onAction
    : currentActionHandler;
  bridge.updateProps({
    uiLanguages: schema.ui_languages || [],
    themes: schema.themes || [],
    holdFollowModes: schema.hold_follow_modes || [],
    holdPresenterBackends: schema.hold_presenter_backends || [],
    general,
    onAction: currentActionHandler,
  });
}

function read() {
  return normalizeGeneralState(currentState);
}

function onAction(handler) {
  currentActionHandler = typeof handler === 'function' ? handler : null;
  bridge.updateProps({
    onAction: currentActionHandler,
  });
}

window.MfxGeneralSection = {
  render,
  read,
  onAction,
};
