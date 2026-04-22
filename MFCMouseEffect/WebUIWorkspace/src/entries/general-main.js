import GeneralSettingsFields from '../general/GeneralSettingsFields.svelte';
import { normalizeGeneralState } from '../general/general-state-model.js';
import { createLazyMountBridge } from './lazy-mount.js';

let currentState = normalizeGeneralState({});

const bridge = createLazyMountBridge({
  mountId: 'general_settings_mount',
  initialProps: {
    uiLanguages: [],
    themes: [],
    overlayTargetFpsRange: {},
    holdFollowModes: [],
    holdPresenterBackends: [],
    general: currentState,
  },
  createComponent: (mountNode, props) => {
    const instance = new GeneralSettingsFields({
      target: mountNode,
      props: {
        ...props,
        onChangeState: (detail) => {
          currentState = normalizeGeneralState(detail);
        },
      },
    });
    return instance;
  },
});

function render(payload) {
  const schema = payload?.schema || {};
  const appState = payload?.state || {};
  const general = normalizeGeneralState(appState);
  currentState = general;
  bridge.updateProps({
    uiLanguages: schema.ui_languages || [],
    themes: schema.themes || [],
    overlayTargetFpsRange: schema.overlay_target_fps_range || {},
    holdFollowModes: schema.hold_follow_modes || [],
    holdPresenterBackends: schema.hold_presenter_backends || [],
    general,
  });
}

function read() {
  return normalizeGeneralState(currentState);
}

function onAction(handler) {
  void handler;
}

window.MfxGeneralSection = {
  render,
  read,
  onAction,
};
