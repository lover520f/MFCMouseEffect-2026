#!/usr/bin/env node
import fs from 'node:fs';
import fsp from 'node:fs/promises';
import path from 'node:path';
import crypto from 'node:crypto';
import { fileURLToPath } from 'node:url';

const THIS_FILE = fileURLToPath(import.meta.url);
export const REPO_ROOT = path.resolve(path.dirname(THIS_FILE), '../..');
export const DOCS_ROOT = path.join(REPO_ROOT, 'docs');
export const AI_DOCS_ROOT = path.join(DOCS_ROOT, '.ai');
export const INDEX_PATH = path.join(AI_DOCS_ROOT, 'context-index.json');
export const MAP_PATH = path.join(AI_DOCS_ROOT, 'context-map.md');
const AGENTS_PATH = path.join(REPO_ROOT, 'AGENTS.md');

const LINE_LIMITS = {
  'AGENTS.md': 260,
  'docs/agent-context/current.md': 220,
  'docs/README.md': 140,
  'docs/README.zh-CN.md': 140,
};

const PRIORITY_BUDGETS = {
  P0: 1500,
  P1: 1100,
  P2: 750,
  P3: 260,
};

const DEFAULT_FIRST_READ_ORDER = [
  'AGENTS.md',
  'docs/agent-context/current.md',
  'docs/refactoring/phase-roadmap-macos-m1-status.md',
];

const STOP_TOKENS = new Set([
  'docs',
  'doc',
  'readme',
  'md',
  'architecture',
  'refactoring',
  'issues',
  'issue',
  'archive',
  'agent',
  'context',
  'phase',
  'roadmap',
]);

function toPosix(relativePath) {
  return relativePath.split(path.sep).join('/');
}

function toAbsolute(relativePath) {
  return path.join(REPO_ROOT, relativePath);
}

function stableSortByPath(items) {
  return items.sort((a, b) => a.relative_path.localeCompare(b.relative_path));
}

function estimateTokens(bytes) {
  return Math.max(1, Math.ceil(bytes / 4));
}

function computeReadBudget(priority, estimatedTokens) {
  const cap = PRIORITY_BUDGETS[priority] || PRIORITY_BUDGETS.P2;
  const requested = Math.max(180, estimatedTokens + 120);
  return Math.min(cap, requested);
}

function readFileOrEmpty(absolutePath) {
  try {
    return fs.readFileSync(absolutePath, 'utf8');
  } catch (_error) {
    return '';
  }
}

function fileStatOrNull(absolutePath) {
  try {
    return fs.statSync(absolutePath);
  } catch (_error) {
    return null;
  }
}

function hashText(content) {
  return crypto.createHash('sha1').update(content).digest('hex');
}

function listMarkdownDocs(rootDir) {
  const output = [];
  const stack = [rootDir];
  while (stack.length > 0) {
    const current = stack.pop();
    const entries = fs.readdirSync(current, { withFileTypes: true });
    for (const entry of entries) {
      const fullPath = path.join(current, entry.name);
      if (entry.isDirectory()) {
        if (entry.name === '.ai') {
          continue;
        }
        stack.push(fullPath);
        continue;
      }
      if (!entry.isFile()) {
        continue;
      }
      if (!entry.name.endsWith('.md')) {
        continue;
      }
      output.push(fullPath);
    }
  }
  return output;
}

function parseFirstReadOrder(agentsContent) {
  const lines = `${agentsContent || ''}`.split(/\r?\n/);
  const ordered = [];
  let inSection = false;
  for (const lineRaw of lines) {
    const line = lineRaw.trim();
    if (!inSection) {
      if (line.startsWith('## First-Read Order')) {
        inSection = true;
      }
      continue;
    }
    if (line.startsWith('## ')) {
      break;
    }
    if (!line) {
      continue;
    }
    const numberedPathOnly = line.match(
      /^\d+\.\s+`?(\/Users\/sunqin\/study\/language\/cpp\/code\/MFCMouseEffect\/[^`\s)]+)`?\s*$/);
    if (!numberedPathOnly) {
      continue;
    }
    const match = numberedPathOnly[1];
    const absolute = match.replace(/[`"'.,;:]+$/g, '');
    const relative = toPosix(path.relative(REPO_ROOT, absolute));
    if (!relative || relative.startsWith('..')) {
      continue;
    }
    if (!ordered.includes(relative)) {
      ordered.push(relative);
    }
  }
  return ordered;
}

function classifyPriority(relativePath, firstReadOrderSet) {
  if (relativePath === 'AGENTS.md') {
    return 'P0';
  }
  if (firstReadOrderSet.has(relativePath)) {
    return 'P1';
  }
  if (relativePath.startsWith('docs/archive/')) {
    return 'P3';
  }
  return 'P2';
}

function classifyRole(relativePath, firstReadRank) {
  if (relativePath === 'AGENTS.md') {
    return 'global-contract';
  }
  if (firstReadRank > 0) {
    return 'first-read';
  }
  if (relativePath.startsWith('docs/archive/')) {
    return 'archive';
  }
  if (relativePath.includes('/issues/')) {
    return 'issue';
  }
  if (relativePath.includes('/refactoring/')) {
    return 'roadmap';
  }
  if (relativePath.includes('/automation/')) {
    return 'automation';
  }
  if (relativePath.includes('/architecture/')) {
    return 'architecture';
  }
  if (relativePath.endsWith('/README.md') || relativePath.endsWith('/README.zh-CN.md')) {
    return 'index';
  }
  return 'capability';
}

function extractSummary(content, fallback) {
  const lines = `${content || ''}`.split(/\r?\n/);
  for (const lineRaw of lines) {
    const line = lineRaw.trim();
    if (!line || line.startsWith('#') || line.startsWith('- ') || line.startsWith('```')) {
      continue;
    }
    if (line.length < 6) {
      continue;
    }
    return line.slice(0, 160);
  }
  return fallback;
}

function extractPathTags(relativePath) {
  const stem = relativePath.replace(/\.md$/i, '');
  const tokens = stem
    .split(/[\/_.-]+/)
    .map((item) => item.trim().toLowerCase())
    .filter(Boolean)
    .filter((item) => !STOP_TOKENS.has(item))
    .filter((item) => !/^\d+$/.test(item));
  return Array.from(new Set(tokens));
}

function extractKeywordTags(contentLower) {
  const keywords = [
    'automation',
    'gesture',
    'wasm',
    'plugin',
    'effects',
    'indicator',
    'theme',
    'regression',
    'workflow',
    'swift',
    'macos',
    'windows',
    'linux',
    'contract',
    'abi',
    'routing',
    'debug',
  ];
  const matched = [];
  for (const keyword of keywords) {
    if (contentLower.includes(keyword)) {
      matched.push(keyword);
    }
  }
  return matched;
}

function composeWhenToRead(priority, role, tags) {
  if (priority === 'P0') {
    return 'Always read first.';
  }
  if (priority === 'P1') {
    return 'Read after P0 for current decisions and active constraints.';
  }
  if (priority === 'P3') {
    return 'Read only when historical details are required.';
  }
  if (tags.includes('automation') || tags.includes('gesture')) {
    return 'Read when task touches automation mapping or gesture recognition.';
  }
  if (tags.includes('wasm') || tags.includes('plugin')) {
    return 'Read when task touches wasm runtime, plugin ABI, or sample behavior.';
  }
  if (role === 'workflow' || tags.includes('workflow') || tags.includes('regression')) {
    return 'Read when executing manual/regression workflows.';
  }
  return 'Read only when task keywords match this capability area.';
}

function topicFromTags(tags) {
  if (tags.includes('automation') || tags.includes('gesture')) {
    return 'automation';
  }
  if (tags.includes('wasm') || tags.includes('plugin')) {
    return 'wasm';
  }
  if (tags.includes('indicator')) {
    return 'input-indicator';
  }
  if (tags.includes('effects') || tags.includes('trail') || tags.includes('hover') || tags.includes('hold')) {
    return 'effects';
  }
  if (tags.includes('theme')) {
    return 'theme';
  }
  if (tags.includes('regression') || tags.includes('workflow')) {
    return 'workflow';
  }
  return 'general';
}

export function buildContextIndex() {
  const agentsContent = readFileOrEmpty(AGENTS_PATH);
  const parsedFirstRead = parseFirstReadOrder(agentsContent);
  const firstReadOrder = Array.from(new Set([
    ...DEFAULT_FIRST_READ_ORDER,
    ...parsedFirstRead,
  ]));
  const firstReadOrderSet = new Set(firstReadOrder);

  const markdownDocs = listMarkdownDocs(DOCS_ROOT);
  const absoluteFiles = [AGENTS_PATH, ...markdownDocs];
  const entries = [];

  for (const absolutePath of absoluteFiles) {
    const relativePath = toPosix(path.relative(REPO_ROOT, absolutePath));
    if (!relativePath || relativePath.startsWith('..')) {
      continue;
    }
    const content = readFileOrEmpty(absolutePath);
    const stat = fileStatOrNull(absolutePath);
    const lineCount = content ? content.split(/\r?\n/).length : 0;
    const bytes = Buffer.byteLength(content, 'utf8');
    const estimatedTokens = estimateTokens(bytes);
    const firstReadRank = firstReadOrder.indexOf(relativePath) + 1;
    const priority = classifyPriority(relativePath, firstReadOrderSet);
    const pathTags = extractPathTags(relativePath);
    const keywordTags = extractKeywordTags(content.toLowerCase());
    const tags = Array.from(new Set([...pathTags, ...keywordTags])).slice(0, 18);
    const role = classifyRole(relativePath, firstReadRank);

    entries.push({
      id: relativePath,
      relative_path: relativePath,
      absolute_path: toAbsolute(relativePath),
      priority,
      role,
      topic: topicFromTags(tags),
      first_read_rank: firstReadRank || 0,
      tags,
      summary: extractSummary(content, path.basename(relativePath, '.md')),
      when_to_read: composeWhenToRead(priority, role, tags),
      line_count: lineCount,
      size_bytes: bytes,
      estimated_tokens: estimatedTokens,
      max_read_tokens: computeReadBudget(priority, estimatedTokens),
      line_limit: LINE_LIMITS[relativePath] || 0,
      line_limit_exceeded: (LINE_LIMITS[relativePath] || 0) > 0
        ? lineCount > LINE_LIMITS[relativePath]
        : false,
      hash_sha1: hashText(content),
      updated_at: stat ? new Date(stat.mtimeMs).toISOString() : '',
    });
  }

  stableSortByPath(entries);

  return {
    version: 1,
    generated_at: new Date().toISOString(),
    repo_root: REPO_ROOT,
    budgets: {
      priority: PRIORITY_BUDGETS,
      line_limits: LINE_LIMITS,
      default_route_budget_tokens: 5200,
      default_route_top_k: 8,
    },
    first_read_order: firstReadOrder,
    entries,
  };
}

function renderTopicRows(entries) {
  const topicOrder = ['automation', 'wasm', 'effects', 'input-indicator', 'workflow', 'theme', 'general'];
  const byTopic = new Map();
  for (const topic of topicOrder) {
    byTopic.set(topic, []);
  }
  for (const entry of entries) {
    const topic = entry.topic || 'general';
    if (!byTopic.has(topic)) {
      byTopic.set(topic, []);
    }
    byTopic.get(topic).push(entry);
  }

  const lines = [];
  for (const topic of topicOrder) {
    const items = (byTopic.get(topic) || [])
      .filter((entry) => entry.priority !== 'P3')
      .slice(0, 4);
    if (items.length === 0) {
      continue;
    }
    lines.push(`### ${topic}`);
    for (const item of items) {
      lines.push(`- \`${item.relative_path}\` (${item.priority}, ${item.max_read_tokens} tok)`);
    }
    lines.push('');
  }
  return lines.join('\n').trim();
}

export function renderContextMap(index) {
  const firstReadLines = [];
  const firstReadEntries = index.first_read_order
    .map((pathValue, i) => {
      const entry = index.entries.find((item) => item.relative_path === pathValue);
      return { rank: i + 1, entry };
    })
    .filter((item) => item.entry);

  for (const item of firstReadEntries) {
    firstReadLines.push(`- ${item.rank}. \`${item.entry.relative_path}\` (${item.entry.max_read_tokens} tok)`);
  }

  const largest = [...index.entries]
    .sort((a, b) => b.estimated_tokens - a.estimated_tokens)
    .slice(0, 8)
    .map((entry) => `- \`${entry.relative_path}\` -> ~${entry.estimated_tokens} tok`);

  return [
    '# AI Context Map',
    '',
    `Generated: ${index.generated_at}`,
    '',
    '## Goal',
    'Load minimal docs by task keyword while keeping AGENTS + current context as mandatory baseline.',
    '',
    '## Mandatory First Read',
    ...firstReadLines,
    '',
    '## Topic Routes (Top Candidates)',
    renderTopicRows(index.entries),
    '',
    '## Commands',
    '```bash',
    './tools/docs/ai-context.sh index',
    './tools/docs/ai-context.sh route --task "automation gesture debug"',
    './tools/docs/ai-context.sh check --strict',
    './tools/docs/ai-context.sh watch',
    '```',
    '',
    '## Largest Docs (Trim Candidates)',
    ...largest,
    '',
    '## Notes',
    '- Index is machine-readable: `docs/.ai/context-index.json`.',
    '- Route output uses token budget + keyword scoring + priority gating.',
    '- `watch` rebuilds index on AGENTS/docs markdown changes.',
    '',
  ].join('\n');
}

export async function writeContextArtifacts(index) {
  await fsp.mkdir(AI_DOCS_ROOT, { recursive: true });
  await fsp.writeFile(INDEX_PATH, `${JSON.stringify(index, null, 2)}\n`, 'utf8');
  await fsp.writeFile(MAP_PATH, renderContextMap(index), 'utf8');
}

export function buildRoute(index, taskText, options = {}) {
  const task = `${taskText || ''}`.trim().toLowerCase();
  const words = task
    .split(/[\s,;:/|+]+/)
    .map((item) => item.trim())
    .filter((item) => item.length >= 2);
  const uniqueWords = Array.from(new Set(words));
  const hasArchiveIntent = uniqueWords.some((word) => word.includes('archive') || word.includes('history'));
  const routeBudget = Number.isFinite(options.budgetTokens)
    ? Math.max(500, Math.round(options.budgetTokens))
    : (index?.budgets?.default_route_budget_tokens || 5200);
  const topK = Number.isFinite(options.topK)
    ? Math.max(3, Math.round(options.topK))
    : (index?.budgets?.default_route_top_k || 8);

  const entries = Array.isArray(index?.entries) ? index.entries : [];
  const scoreEntry = (entry) => {
    let score = 0;
    if (entry.priority === 'P0') score += 200;
    else if (entry.priority === 'P1') score += 140;
    else if (entry.priority === 'P2') score += 70;
    else score += 20;

    if (entry.first_read_rank > 0) {
      score += 40;
    }
    if (task.length === 0) {
      return score;
    }

    const blob = `${entry.relative_path} ${entry.summary} ${entry.tags.join(' ')}`.toLowerCase();
    for (const word of uniqueWords) {
      if (entry.tags.includes(word)) {
        score += 18;
      }
      if (entry.relative_path.toLowerCase().includes(word)) {
        score += 12;
      }
      if (blob.includes(word)) {
        score += 6;
      }
    }
    return score;
  };

  const sorted = [...entries]
    .map((entry) => {
      const blob = `${entry.relative_path} ${entry.summary} ${entry.tags.join(' ')}`.toLowerCase();
      const matchCount = uniqueWords.reduce((count, word) => (blob.includes(word) ? count + 1 : count), 0);
      let score = scoreEntry(entry);
      if (task.length > 0 && entry.role === 'index') {
        score -= 16;
      }
      if (entry.priority === 'P3') {
        score -= 20;
      }
      return { entry, score, matchCount };
    })
    .sort((a, b) => b.score - a.score);

  const selected = [];
  const selectedSet = new Set();

  const firstRead = Array.isArray(index?.first_read_order) ? index.first_read_order : [];
  for (const baselinePath of firstRead) {
    const item = sorted.find((candidate) => candidate.entry.relative_path === baselinePath);
    if (!item) {
      continue;
    }
    if (!selectedSet.has(item.entry.relative_path)) {
      selected.push(item);
      selectedSet.add(item.entry.relative_path);
    }
  }

  const baselineCount = selected.length;
  let tokenSum = selected.reduce((sum, item) => sum + (item.entry.max_read_tokens || 0), 0);
  let addedKeywordMatch = false;
  let addedConcreteMatch = false;
  for (const item of sorted) {
    const key = item.entry.relative_path;
    if (selectedSet.has(key)) {
      continue;
    }
    if (!hasArchiveIntent && item.entry.priority === 'P3') {
      continue;
    }
    if (item.entry.role === 'issue' && task.length > 0 && item.matchCount < 2) {
      continue;
    }
    if (task.length > 0 && selected.length >= baselineCount && addedKeywordMatch && item.matchCount === 0) {
      continue;
    }
    const next = tokenSum + (item.entry.max_read_tokens || 0);
    if (selected.length >= topK) {
      break;
    }
    if (selected.length >= 3 && next > routeBudget) {
      const canOverflowForTarget = selected.length >= baselineCount
        && !addedKeywordMatch
        && item.entry.priority !== 'P3'
        && item.matchCount > 0;
      if (!canOverflowForTarget) {
        continue;
      }
    }
    if (selected.length >= 3 && next > routeBudget * 1.2) {
      continue;
    }
    selected.push(item);
    selectedSet.add(key);
    tokenSum = next;
    if (item.matchCount > 0 && item.entry.priority !== 'P3') {
      addedKeywordMatch = true;
      const isIndexLike = item.entry.role === 'index'
        || item.entry.relative_path.endsWith('/p2-capability-index.md');
      if (!isIndexLike) {
        addedConcreteMatch = true;
      }
    }
  }

  if (task.length > 0 && !addedConcreteMatch) {
    const fallback = sorted.find((item) => {
      if (selectedSet.has(item.entry.relative_path)) {
        return false;
      }
      if (item.entry.priority === 'P3' || item.matchCount === 0) {
        return false;
      }
      if (item.entry.role === 'index') {
        return false;
      }
      if (item.entry.relative_path.endsWith('/p2-capability-index.md')) {
        return false;
      }
      return true;
    });
    if (fallback) {
      const next = tokenSum + (fallback.entry.max_read_tokens || 0);
      if (next <= routeBudget * 1.15) {
        selected.push(fallback);
        selectedSet.add(fallback.entry.relative_path);
        tokenSum = next;
      }
    }
  }

  return {
    task,
    keywords: uniqueWords,
    budget_tokens: routeBudget,
    selected_count: selected.length,
    selected_tokens: tokenSum,
    files: selected.map((item) => ({
      path: item.entry.relative_path,
      priority: item.entry.priority,
      score: item.score,
      max_read_tokens: item.entry.max_read_tokens,
      reason: item.entry.when_to_read,
      tags: item.entry.tags,
    })),
  };
}

export function loadIndexOrNull() {
  try {
    const raw = fs.readFileSync(INDEX_PATH, 'utf8');
    const parsed = JSON.parse(raw);
    if (!parsed || typeof parsed !== 'object') {
      return null;
    }
    return parsed;
  } catch (_error) {
    return null;
  }
}
