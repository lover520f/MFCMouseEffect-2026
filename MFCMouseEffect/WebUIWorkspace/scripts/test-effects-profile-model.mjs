import assert from 'node:assert/strict';

import { normalizeEffectsProfile } from '../src/effects/profile-model.js';

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

runTest('empty input returns empty object', () => {
  assert.deepEqual(normalizeEffectsProfile(null), { platform: '', active: {} });
  assert.deepEqual(normalizeEffectsProfile(undefined), { platform: '', active: {} });
  assert.deepEqual(normalizeEffectsProfile(42), {});
});

runTest('normalizes platform and active fields', () => {
  const profile = normalizeEffectsProfile({
    platform: '  macos ',
    active: { click: 'text' },
  });
  assert.equal(profile.platform, 'macos');
  assert.deepEqual(profile.active, { click: 'text' });
});

runTest('keeps known profile sections only', () => {
  const profile = normalizeEffectsProfile({
    platform: 'macos',
    click: { normal_size_px: 100 },
    trail: { duration_sec: 0.2 },
    trail_throttle: { min_interval_ms: 10 },
    scroll: { base_duration_sec: 0.3 },
    hold: { progress_full_ms: 1200 },
    hover: { spin_duration_sec: 1.4 },
    other: { ignored: true },
  });

  assert.deepEqual(Object.keys(profile).sort(), [
    'active',
    'click',
    'hold',
    'hover',
    'platform',
    'scroll',
    'trail',
    'trail_throttle',
  ]);
});

if (failed > 0) {
  console.error(`[result] failed: ${failed}`);
  process.exit(1);
}

console.log('[result] effects profile model tests passed');
