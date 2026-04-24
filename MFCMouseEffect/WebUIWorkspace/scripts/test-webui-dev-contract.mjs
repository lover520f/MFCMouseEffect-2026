import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';

const workspaceRoot = path.resolve(import.meta.dirname, '..');

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

function readWorkspaceFile(relativePath) {
  return fs.readFileSync(path.join(workspaceRoot, relativePath), 'utf8');
}

function stripLineComments(source) {
  return source
    .split('\n')
    .map((line) => line.replace(/\/\/.*$/g, ''))
    .join('\n');
}

runTest('entry adapters avoid direct Svelte instance event API', () => {
  const entriesDir = path.join(workspaceRoot, 'src', 'entries');
  const entryFiles = fs.readdirSync(entriesDir)
    .filter((name) => name.endsWith('.js'))
    .sort();

  const offenders = [];
  for (const name of entryFiles) {
    const relativePath = path.join('src', 'entries', name);
    const source = stripLineComments(readWorkspaceFile(relativePath));
    if (source.includes('.$on(')) {
      offenders.push(relativePath);
    }
  }

  assert.deepEqual(
    offenders,
    [],
    `source-mode entries must use callback props / bridges instead of component.$on(): ${offenders.join(', ')}`,
  );
});

runTest('automation api avoids component method lookup by string name', () => {
  const source = stripLineComments(readWorkspaceFile(path.join('src', 'automation', 'api.js')));
  assert.equal(source.includes('target[name]'), false, 'forbidden target[name] lookup detected in automation/api.js');
  assert.equal(source.includes('typeof target[name]'), false, 'forbidden typeof target[name] lookup detected in automation/api.js');
});

if (failed > 0) {
  console.error(`[result] failed: ${failed}`);
  process.exit(1);
}

console.log('[result] webui dev contract tests passed');
