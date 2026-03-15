import assert from 'node:assert/strict';

import { shortcutFromKeyboardEvent } from '../src/automation/shortcuts.js';

let failed = 0;

function runTest(name, fn) {
  try {
    fn();
    console.log(`[pass] ${name}`);
  } catch (error) {
    failed += 1;
    console.error(`[fail] ${name}`);
    console.error(error instanceof Error ? error.stack || error.message : error);
  }
}

runTest('macos cmd+letter emits standard shortcut', () => {
  const shortcut = shortcutFromKeyboardEvent(
    { key: 'c', code: 'KeyC', metaKey: true, ctrlKey: false, shiftKey: false, altKey: false },
    'macos'
  );
  assert.equal(shortcut, 'Cmd+C');
});

runTest('macos cmd+right bracket falls back to code when key is transformed', () => {
  const shortcut = shortcutFromKeyboardEvent(
    { key: 'Dead', code: 'BracketRight', metaKey: true, ctrlKey: false, shiftKey: false, altKey: false },
    'macos'
  );
  assert.equal(shortcut, 'Cmd+BracketRight');
});

runTest('macos cmd+left bracket falls back to code when key is unavailable', () => {
  const shortcut = shortcutFromKeyboardEvent(
    { key: '', code: 'BracketLeft', metaKey: true, ctrlKey: false, shiftKey: false, altKey: false },
    'macos'
  );
  assert.equal(shortcut, 'Cmd+BracketLeft');
});

if (failed > 0) {
  console.error(`[result] failed: ${failed}`);
  process.exit(1);
}

console.log('[result] all automation shortcut tests passed');
