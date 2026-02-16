import GeneralSettingsFields from '../general/GeneralSettingsFields.svelte';

const mountNode = document.getElementById('general_settings_mount');
let component = null;
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

if (mountNode) {
  component = new GeneralSettingsFields({
    target: mountNode,
    props: {
      uiLanguages: [],
      themes: [],
      holdFollowModes: [],
      holdPresenterBackends: [],
      general: currentState,
    },
  });

  component.$on('change', (event) => {
    const detail = event?.detail || {};
    currentState = normalizeGeneral(detail);
  });
}

function render(payload) {
  if (!component) return;
  const schema = payload?.schema || {};
  const appState = payload?.state || {};
  const general = normalizeGeneral(appState);
  currentState = general;
  component.$set({
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

if (!component) {
  window.MfxGeneralSection = {
    render: () => {},
    read,
  };
}
