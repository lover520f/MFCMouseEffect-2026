#!/usr/bin/env node
import { buildContextIndex, writeContextArtifacts, INDEX_PATH, MAP_PATH } from './ai-context-lib.mjs';

async function main() {
  const index = buildContextIndex();
  await writeContextArtifacts(index);
  console.log(`[ok] context index generated: ${INDEX_PATH}`);
  console.log(`[ok] context map generated:   ${MAP_PATH}`);
  console.log(`[info] entries: ${index.entries.length}`);
}

main().catch((error) => {
  console.error('[error] failed to generate context artifacts');
  console.error(error?.stack || String(error));
  process.exitCode = 1;
});
