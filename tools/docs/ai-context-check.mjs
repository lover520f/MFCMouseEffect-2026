#!/usr/bin/env node
import { buildContextIndex, INDEX_PATH, loadIndexOrNull } from './ai-context-lib.mjs';

function readArgs(argv) {
  return {
    strict: argv.includes('--strict'),
    enforceLineLimits: argv.includes('--enforce-line-limits'),
  };
}

function compareEntries(expected, actual) {
  const failures = [];
  const actualMap = new Map((actual?.entries || []).map((entry) => [entry.relative_path, entry]));
  const expectedMap = new Map((expected?.entries || []).map((entry) => [entry.relative_path, entry]));

  for (const [relativePath, expectedEntry] of expectedMap.entries()) {
    const actualEntry = actualMap.get(relativePath);
    if (!actualEntry) {
      failures.push(`[missing] ${relativePath} not found in context-index`);
      continue;
    }
    if (actualEntry.hash_sha1 !== expectedEntry.hash_sha1) {
      failures.push(`[stale] ${relativePath} hash differs (index not refreshed)`);
    }
    if (actualEntry.priority !== expectedEntry.priority) {
      failures.push(`[drift] ${relativePath} priority changed (${actualEntry.priority} -> ${expectedEntry.priority})`);
    }
  }

  for (const relativePath of actualMap.keys()) {
    if (!expectedMap.has(relativePath)) {
      failures.push(`[orphan] ${relativePath} exists in index but no longer exists in docs tree`);
    }
  }
  return failures;
}

function compareFirstRead(expected, actual) {
  const expectedList = expected.first_read_order || [];
  const actualList = actual?.first_read_order || [];
  if (expectedList.length !== actualList.length) {
    return [`[drift] first_read_order length differs (${actualList.length} -> ${expectedList.length})`];
  }
  const failures = [];
  for (let i = 0; i < expectedList.length; i += 1) {
    if (expectedList[i] !== actualList[i]) {
      failures.push(`[drift] first_read_order[${i}] ${actualList[i] || '(missing)'} -> ${expectedList[i]}`);
    }
  }
  return failures;
}

function main() {
  const args = readArgs(process.argv);
  const actual = loadIndexOrNull();
  if (!actual) {
    console.error(`[fail] missing or invalid context index: ${INDEX_PATH}`);
    process.exitCode = 1;
    return;
  }

  const expected = buildContextIndex();
  const failures = [
    ...compareFirstRead(expected, actual),
    ...compareEntries(expected, actual),
  ];
  const lineLimitWarnings = (expected.entries || [])
    .filter((entry) => entry.line_limit_exceeded)
    .map((entry) => `[line-limit] ${entry.relative_path} ${entry.line_count}/${entry.line_limit}`);

  if (failures.length === 0) {
    if (lineLimitWarnings.length > 0) {
      for (const warning of lineLimitWarnings) {
        console.log(warning);
      }
      if (args.enforceLineLimits) {
        console.error(`[fail] context index check failed: ${lineLimitWarnings.length} line-limit issue(s)`);
        process.exitCode = 1;
        return;
      }
      console.warn(`[warn] context index is fresh but has ${lineLimitWarnings.length} line-limit issue(s)`);
      return;
    }
    console.log('[pass] context index is up to date');
    return;
  }

  for (const failure of failures) {
    console.log(failure);
  }

  if (args.strict) {
    console.error(`[fail] context index check failed with ${failures.length} issue(s)`);
    process.exitCode = 1;
    return;
  }
  console.warn(`[warn] context index check found ${failures.length} issue(s)`);
}

main();
