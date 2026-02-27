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
    config_basis: { ripple_duration_ms: 300, test_tuning: { duration_scale: 0.5 } },
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
    'config_basis',
    'hold',
    'hover',
    'platform',
    'scroll',
    'trail',
    'trail_throttle',
  ]);
});

runTest('preserves config basis test tuning diagnostics', () => {
  const profile = normalizeEffectsProfile({
    platform: 'macos',
    config_basis: {
      ripple_duration_ms: 300,
      test_tuning: {
        duration_scale: 0.5,
        size_scale: 1.2,
        opacity_scale: 0.78,
        trail_throttle_scale: 0.6,
        duration_overridden: true,
        size_overridden: true,
        opacity_overridden: true,
        trail_throttle_overridden: true,
      },
    },
    click: { base_opacity: 0.74 },
    trail: { base_opacity: 0.74 },
    scroll: { base_opacity: 0.75 },
    hold: { base_opacity: 0.72 },
    hover: { base_opacity: 0.7 },
  });

  assert.equal(profile.config_basis?.ripple_duration_ms, 300);
  assert.equal(profile.config_basis?.test_tuning?.duration_scale, 0.5);
  assert.equal(profile.config_basis?.test_tuning?.size_scale, 1.2);
  assert.equal(profile.config_basis?.test_tuning?.opacity_scale, 0.78);
  assert.equal(profile.config_basis?.test_tuning?.trail_throttle_scale, 0.6);
  assert.equal(profile.config_basis?.test_tuning?.duration_overridden, true);
  assert.equal(profile.config_basis?.test_tuning?.size_overridden, true);
  assert.equal(profile.config_basis?.test_tuning?.opacity_overridden, true);
  assert.equal(profile.config_basis?.test_tuning?.trail_throttle_overridden, true);
  assert.equal(profile.click?.base_opacity, 0.74);
  assert.equal(profile.trail?.base_opacity, 0.74);
  assert.equal(profile.scroll?.base_opacity, 0.75);
  assert.equal(profile.hold?.base_opacity, 0.72);
  assert.equal(profile.hover?.base_opacity, 0.7);
});

if (failed > 0) {
  console.error(`[result] failed: ${failed}`);
  process.exit(1);
}

console.log('[result] effects profile model tests passed');
