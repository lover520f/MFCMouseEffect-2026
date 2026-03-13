#!/usr/bin/env node
import { buildContextIndex, buildRoute, loadIndexOrNull } from './ai-context-lib.mjs';

function readArgs(argv) {
  const out = {
    task: '',
    budget: NaN,
    top: NaN,
    json: false,
  };
  for (let i = 2; i < argv.length; i += 1) {
    const token = argv[i];
    if (token === '--task' && i + 1 < argv.length) {
      out.task = argv[i + 1];
      i += 1;
      continue;
    }
    if (token === '--budget' && i + 1 < argv.length) {
      out.budget = Number(argv[i + 1]);
      i += 1;
      continue;
    }
    if (token === '--top' && i + 1 < argv.length) {
      out.top = Number(argv[i + 1]);
      i += 1;
      continue;
    }
    if (token === '--json') {
      out.json = true;
    }
  }
  return out;
}

function renderHuman(route) {
  const lines = [];
  lines.push(`# Task Route`);
  lines.push(`task: ${route.task || '(baseline)'}`);
  lines.push(`budget: ${route.selected_tokens}/${route.budget_tokens} tokens`);
  lines.push('');
  for (const file of route.files) {
    lines.push(`- ${file.path} [${file.priority}] ${file.max_read_tokens} tok`);
    lines.push(`  reason: ${file.reason}`);
  }
  return `${lines.join('\n')}\n`;
}

function main() {
  const args = readArgs(process.argv);
  const existing = loadIndexOrNull();
  const index = existing || buildContextIndex();
  const route = buildRoute(index, args.task, {
    budgetTokens: Number.isFinite(args.budget) ? args.budget : undefined,
    topK: Number.isFinite(args.top) ? args.top : undefined,
  });

  if (args.json) {
    process.stdout.write(`${JSON.stringify(route, null, 2)}\n`);
    return;
  }
  process.stdout.write(renderHuman(route));
}

main();
