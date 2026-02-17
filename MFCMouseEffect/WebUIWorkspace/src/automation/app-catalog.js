function asText(value) {
  return `${value || ''}`.trim();
}

function normalizeProcessName(value) {
  const trimmed = asText(value).toLowerCase().replace(/\\/g, '/');
  if (!trimmed) {
    return '';
  }
  const parts = trimmed.split('/');
  const base = `${parts[parts.length - 1] || ''}`.trim();
  if (!base) {
    return '';
  }
  if (base.includes('.')) {
    return base;
  }
  return `${base}.exe`;
}

function normalizeLabel(value, fallback) {
  const label = asText(value);
  return label || fallback;
}

function normalizeSource(value) {
  return asText(value).toLowerCase();
}

function scoreEntry(entry, query) {
  const keyword = asText(query).toLowerCase();
  if (!keyword) {
    return 0;
  }

  const exe = entry.exe;
  const label = entry.labelLower;
  if (exe === keyword) {
    return 0;
  }
  if (label === keyword) {
    return 1;
  }
  if (exe.startsWith(keyword)) {
    return 2;
  }
  if (label.startsWith(keyword)) {
    return 3;
  }
  if (exe.includes(keyword)) {
    return 4;
  }
  if (label.includes(keyword)) {
    return 5;
  }
  return 999;
}

export function normalizeCatalogEntries(items) {
  const source = Array.isArray(items) ? items : [];
  const out = [];
  const seen = new Set();

  for (const item of source) {
    const raw = item && typeof item === 'object' ? item : { exe: item };
    const exe = normalizeProcessName(raw.exe || raw.process || raw.value || raw.path || raw.name || '');
    if (!exe || seen.has(exe)) {
      continue;
    }
    seen.add(exe);

    const label = normalizeLabel(raw.label || raw.name || raw.title, exe);
    out.push({
      exe,
      label,
      labelLower: label.toLowerCase(),
      source: normalizeSource(raw.source),
    });
  }

  out.sort((left, right) => {
    const byLabel = left.labelLower.localeCompare(right.labelLower);
    if (byLabel !== 0) {
      return byLabel;
    }
    return left.exe.localeCompare(right.exe);
  });
  return out;
}

export function filterCatalogEntries(items, query, excludedExeNames, limit = 10) {
  const rows = Array.isArray(items) ? items : [];
  const keyword = asText(query);
  const excluded = new Set(
    (Array.isArray(excludedExeNames) ? excludedExeNames : [])
      .map((item) => normalizeProcessName(item))
      .filter(Boolean),
  );

  const scored = [];
  for (const entry of rows) {
    if (!entry || !entry.exe || excluded.has(entry.exe)) {
      continue;
    }
    const score = scoreEntry(entry, keyword);
    if (score >= 999) {
      continue;
    }
    scored.push({ entry, score });
  }

  scored.sort((left, right) => {
    if (left.score !== right.score) {
      return left.score - right.score;
    }
    const byLabel = left.entry.labelLower.localeCompare(right.entry.labelLower);
    if (byLabel !== 0) {
      return byLabel;
    }
    return left.entry.exe.localeCompare(right.entry.exe);
  });

  const safeLimit = Number.isFinite(limit) && limit > 0 ? Math.floor(limit) : 10;
  return scored.slice(0, safeLimit).map((item) => item.entry);
}

