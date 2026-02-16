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

const ALNUM_RE = /^[a-z0-9]$/i;
const FUNCTION_KEY_RE = /^f([1-9]|1\d|2[0-4])$/i;

export function shortcutFromKeyboardEvent(event) {
  const modifiers = [];
  if (event.ctrlKey) {
    modifiers.push('Ctrl');
  }
  if (event.shiftKey) {
    modifiers.push('Shift');
  }
  if (event.altKey) {
    modifiers.push('Alt');
  }
  if (event.metaKey) {
    modifiers.push('Win');
  }

  const key = `${event.key || ''}`;
  const lowered = key.toLowerCase();
  let main = '';

  if (key.length === 1 && ALNUM_RE.test(key)) {
    main = key.toUpperCase();
  } else if (FUNCTION_KEY_RE.test(key)) {
    main = key.toUpperCase();
  } else if (SPECIAL_KEYS[lowered]) {
    main = SPECIAL_KEYS[lowered];
  }

  if (!main) {
    return '';
  }
  if (modifiers.indexOf(main) === -1) {
    modifiers.push(main);
  }
  return modifiers.join('+');
}
