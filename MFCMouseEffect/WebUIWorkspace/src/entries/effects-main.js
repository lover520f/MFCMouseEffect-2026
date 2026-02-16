import ActiveEffectsFields from '../effects/ActiveEffectsFields.svelte';

const mountNode = document.getElementById('effects_settings_mount');
let component = null;
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

if (mountNode) {
  component = new ActiveEffectsFields({
    target: mountNode,
    props: {
      clickOptions: [],
      trailOptions: [],
      scrollOptions: [],
      holdOptions: [],
      hoverOptions: [],
      active: currentState,
    },
  });

  component.$on('change', (event) => {
    const detail = event?.detail || {};
    currentState = normalizeActive(detail);
  });
}

function render(payload) {
  if (!component) return;
  const schema = payload?.schema || {};
  const appState = payload?.state || {};
  const active = normalizeActive(appState.active || {});
  currentState = active;
  component.$set({
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

if (!component) {
  window.MfxEffectsSection = {
    render: () => {},
    read,
  };
}
