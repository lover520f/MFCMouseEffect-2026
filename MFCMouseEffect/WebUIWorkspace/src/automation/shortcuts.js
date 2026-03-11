const SPECIAL_KEYS = {
  enter: 'Enter',
  escape: 'Esc',
  tab: 'Tab',
  backspace: 'Backspace',
  delete: 'Delete',
  insert: 'Insert',
  home: 'Home',
  end: 'End',
  pageup: 'PageUp',
  pagedown: 'PageDown',
  arrowup: 'Up',
  arrowdown: 'Down',
  arrowleft: 'Left',
  arrowright: 'Right',
  ' ': 'Space',
};

const SYMBOL_KEYS = {
  '`': 'Backquote',
  '-': 'Minus',
  '=': 'Equals',
  '[': 'BracketLeft',
  ']': 'BracketRight',
  '\\': 'Backslash',
  ';': 'Semicolon',
  "'": 'Quote',
  ',': 'Comma',
  '.': 'Period',
  '/': 'Slash',
};

const ALNUM_RE = /^[a-z0-9]$/i;
const FUNCTION_KEY_RE = /^f([1-9]|1\d|2[0-4])$/i;
function primaryMetaLabel(platform = 'windows') {
  return `${platform || ''}`.trim().toLowerCase() === 'macos' ? 'Cmd' : 'Win';
}

function altModifierLabel(platform = 'windows') {
  return `${platform || ''}`.trim().toLowerCase() === 'macos' ? 'Option' : 'Alt';
}

function modifierKeyLabels(platform = 'windows') {
  const metaLabel = primaryMetaLabel(platform);
  const altLabel = altModifierLabel(platform);
  return {
    control: 'Ctrl',
    ctrl: 'Ctrl',
    shift: 'Shift',
    alt: altLabel,
    option: altLabel,
    meta: metaLabel,
    os: metaLabel,
    command: metaLabel,
  };
}

export function shortcutFromKeyboardEvent(event, platform = 'windows') {
  const modifiers = [];
  const metaLabel = primaryMetaLabel(platform);
  const altLabel = altModifierLabel(platform);
  if (event.ctrlKey) {
    modifiers.push('Ctrl');
  }
  if (event.shiftKey) {
    modifiers.push('Shift');
  }
  if (event.altKey) {
    modifiers.push(altLabel);
  }
  if (event.metaKey) {
    modifiers.push(metaLabel);
  }

  const key = `${event.key || ''}`;
  const code = `${event.code || ''}`;
  const lowered = key.toLowerCase();
  let main = '';

  if (key.length === 1 && ALNUM_RE.test(key)) {
    main = key.toUpperCase();
  } else if (key.length === 1 && SYMBOL_KEYS[key]) {
    main = SYMBOL_KEYS[key];
  } else if (FUNCTION_KEY_RE.test(key)) {
    main = key.toUpperCase();
  } else if (SPECIAL_KEYS[lowered]) {
    main = SPECIAL_KEYS[lowered];
  } else if (/^numpad[0-9]$/i.test(code)) {
    main = code.replace(/^numpad/i, 'Num');
  } else if (code === 'NumpadAdd') {
    main = 'NumPlus';
  } else if (code === 'NumpadSubtract') {
    main = 'NumMinus';
  } else if (code === 'NumpadMultiply') {
    main = 'NumMultiply';
  } else if (code === 'NumpadDivide') {
    main = 'NumDivide';
  } else if (code === 'NumpadDecimal') {
    main = 'NumDecimal';
  }

  const modifierLabels = modifierKeyLabels(platform);
  if (!main && modifierLabels[lowered]) {
    main = modifierLabels[lowered];
  }

  if (!main) {
    return '';
  }
  if (modifiers.indexOf(main) === -1) {
    modifiers.push(main);
  }
  return modifiers.join('+');
}
