import assert from 'node:assert/strict';

import { normalizeGeneralState } from '../src/general/general-state-model.js';

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

runTest('empty input returns default general state', () => {
  assert.deepEqual(normalizeGeneralState(undefined), {
    ui_language: '',
    theme: '',
    theme_catalog_root_path: '',
    overlay_target_fps: 0,
    hold_follow_mode: 'smooth',
    hold_presenter_backend: 'auto',
  });
  assert.deepEqual(normalizeGeneralState(null), {
    ui_language: '',
    theme: '',
    theme_catalog_root_path: '',
    overlay_target_fps: 0,
    hold_follow_mode: 'smooth',
    hold_presenter_backend: 'auto',
  });
});

runTest('normalizes known keys and preserves catalog root path', () => {
  assert.deepEqual(normalizeGeneralState({
    ui_language: 'en-US',
    theme: 'neon',
    theme_catalog_root_path: '/tmp/theme-catalog',
    overlay_target_fps: 144,
    hold_follow_mode: 'precise',
    hold_presenter_backend: 'metal',
  }), {
    ui_language: 'en-US',
    theme: 'neon',
    theme_catalog_root_path: '/tmp/theme-catalog',
    overlay_target_fps: 144,
    hold_follow_mode: 'precise',
    hold_presenter_backend: 'metal',
  });
});

runTest('falls back per field when values are empty', () => {
  assert.deepEqual(normalizeGeneralState({
    ui_language: '',
    theme: '',
    theme_catalog_root_path: '',
    overlay_target_fps: '',
    hold_follow_mode: '',
    hold_presenter_backend: '',
  }), {
    ui_language: '',
    theme: '',
    theme_catalog_root_path: '',
    overlay_target_fps: 0,
    hold_follow_mode: 'smooth',
    hold_presenter_backend: 'auto',
  });
});

runTest('overlay target fps is clamped to safe range', () => {
  assert.equal(normalizeGeneralState({ overlay_target_fps: 999 }).overlay_target_fps, 360);
  assert.equal(normalizeGeneralState({ overlay_target_fps: -1 }).overlay_target_fps, 0);
  assert.equal(normalizeGeneralState({ overlay_target_fps: '165' }).overlay_target_fps, 165);
});

if (failed > 0) {
  console.error(`[result] failed: ${failed}`);
  process.exit(1);
}

console.log('[result] general state model tests passed');
