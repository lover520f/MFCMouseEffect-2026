import assert from 'node:assert/strict';

import {
  manifestPathForIndicatorLoad,
  normalizeCatalogItems,
} from '../src/input-indicator/input-indicator-plugin-menu-model.js';

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

runTest('load payload prefers user selected manifest path over stale prop path', () => {
  const selected = '/tmp/plugins/indicator-keyviz/plugin.json';
  const staleProp = '/tmp/plugins/indicator-basic/plugin.json';
  assert.equal(manifestPathForIndicatorLoad(selected, staleProp), selected);
});

runTest('load payload falls back to current manifest path when selected path is empty', () => {
  const selected = '   ';
  const current = '/tmp/plugins/indicator-basic/plugin.json';
  assert.equal(manifestPathForIndicatorLoad(selected, current), current);
});

runTest('catalog keeps both basic and keyviz indicator samples', () => {
  const items = normalizeCatalogItems([
    {
      id: 'demo.indicator.basic.v2',
      name: 'Demo Indicator Basic',
      version: '0.1.0',
      surfaces: ['indicator'],
      has_explicit_surfaces: true,
      input_kinds: ['indicator_click', 'indicator_key'],
      manifest_path: '/tmp/plugins/demo.indicator.basic.v2/plugin.json',
    },
    {
      id: 'demo.indicator.keyviz.v2',
      name: 'Demo Indicator Keyviz Style',
      version: '0.1.0',
      surfaces: ['indicator'],
      has_explicit_surfaces: true,
      input_kinds: ['indicator_click', 'indicator_key'],
      manifest_path: '/tmp/plugins/demo.indicator.keyviz.v2/plugin.json',
    },
  ]);

  assert.equal(items.length, 2);
  assert.equal(items[0].id, 'demo.indicator.basic.v2');
  assert.equal(items[1].id, 'demo.indicator.keyviz.v2');
});

if (failed > 0) {
  console.error(`[result] failed: ${failed}`);
  process.exit(1);
}

console.log('[result] input-indicator plugin menu model tests passed');
