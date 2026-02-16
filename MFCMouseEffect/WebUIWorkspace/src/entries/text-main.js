import TextContentFields from '../text/TextContentFields.svelte';

const mountNode = document.getElementById('text_settings_mount');
let component = null;
let currentState = {
  text_content: '',
  text_font_size: 0,
};

function toNumber(value, fallback) {
  const parsed = Number(value);
  if (Number.isFinite(parsed)) return parsed;
  return fallback;
}

function normalizeText(input) {
  const value = input || {};
  return {
    text_content: value.text_content || '',
    text_font_size: toNumber(value.text_font_size, 0),
  };
}

if (mountNode) {
  component = new TextContentFields({
    target: mountNode,
    props: {
      text: currentState,
    },
  });

  component.$on('change', (event) => {
    const detail = event?.detail || {};
    currentState = normalizeText(detail);
  });
}

function render(payload) {
  if (!component) return;
  const appState = payload?.state || {};
  const text = normalizeText(appState);
  currentState = text;
  component.$set({ text });
}

function read() {
  return normalizeText(currentState);
}

window.MfxTextSection = {
  render,
  read,
};

if (!component) {
  window.MfxTextSection = {
    render: () => {},
    read,
  };
}
