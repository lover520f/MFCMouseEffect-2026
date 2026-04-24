import TextContentFields from '../text/TextContentFields.svelte';
import { createLazyMountBridge } from './lazy-mount.js';
import { mountLegacyComponent } from './legacy-component.js';

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

const bridge = createLazyMountBridge({
  mountId: 'text_settings_mount',
  initialProps: {
    text: currentState,
  },
  createComponent: (mountNode, props) => {
    return mountLegacyComponent(TextContentFields, mountNode, {
      ...props,
      onChangeState: (detail) => {
        currentState = normalizeText(detail);
      },
    });
  },
});

function render(payload) {
  const appState = payload?.state || {};
  const text = normalizeText(appState);
  currentState = text;
  bridge.updateProps({ text });
}

function read() {
  return normalizeText(currentState);
}

window.MfxTextSection = {
  render,
  read,
};
