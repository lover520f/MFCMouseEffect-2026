#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import {
  REPO_ROOT,
  DOCS_ROOT,
  buildContextIndex,
  writeContextArtifacts,
} from './ai-context-lib.mjs';

const watchers = new Map();
let timer = null;
let running = false;
let pending = false;

function listDocDirectories(rootDir) {
  const output = [];
  const stack = [rootDir];
  while (stack.length > 0) {
    const current = stack.pop();
    output.push(current);
    const entries = fs.readdirSync(current, { withFileTypes: true });
    for (const entry of entries) {
      if (!entry.isDirectory()) {
        continue;
      }
      if (entry.name === '.ai') {
        continue;
      }
      stack.push(path.join(current, entry.name));
    }
  }
  return output;
}

async function rebuild(reason) {
  if (running) {
    pending = true;
    return;
  }
  running = true;
  try {
    const index = buildContextIndex();
    await writeContextArtifacts(index);
    console.log(`[watch] rebuilt context index (${reason})`);
  } catch (error) {
    console.error('[watch] rebuild failed');
    console.error(error?.stack || String(error));
  } finally {
    running = false;
    if (pending) {
      pending = false;
      await rebuild('pending changes');
    }
  }
}

function scheduleRebuild(reason) {
  if (timer) {
    clearTimeout(timer);
  }
  timer = setTimeout(() => {
    timer = null;
    rebuild(reason);
  }, 350);
}

function relevantFile(filename) {
  const name = `${filename || ''}`;
  if (!name) {
    return false;
  }
  return name.endsWith('.md') || name === 'AGENTS.md';
}

function ensureWatchers() {
  const targets = new Set([
    ...listDocDirectories(DOCS_ROOT),
    REPO_ROOT,
  ]);

  for (const existingPath of watchers.keys()) {
    if (!targets.has(existingPath)) {
      watchers.get(existingPath).close();
      watchers.delete(existingPath);
    }
  }

  for (const targetPath of targets) {
    if (watchers.has(targetPath)) {
      continue;
    }
    const watcher = fs.watch(targetPath, (_event, filename) => {
      if (!relevantFile(filename)) {
        return;
      }
      scheduleRebuild(`${targetPath}/${filename}`);
      ensureWatchers();
    });
    watchers.set(targetPath, watcher);
  }
}

async function main() {
  ensureWatchers();
  await rebuild('initial');
  console.log('[watch] watching AGENTS.md and docs/**/*.md');
}

main().catch((error) => {
  console.error('[watch] fatal error');
  console.error(error?.stack || String(error));
  process.exitCode = 1;
});
