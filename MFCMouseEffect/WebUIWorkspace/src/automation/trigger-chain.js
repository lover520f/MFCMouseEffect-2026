function asArray(value) {
  return Array.isArray(value) ? value : [];
}

function asText(value) {
  return `${value || ''}`.trim();
}

function toNodeArray(value) {
  if (Array.isArray(value)) {
    return value;
  }
  if (!value || typeof value === 'string') {
    return [];
  }
  if (typeof value.length === 'number' && value.length >= 0) {
    try {
      return Array.from(value);
    } catch (_error) {
      return [];
    }
  }
  if (typeof value[Symbol.iterator] === 'function') {
    try {
      return Array.from(value);
    } catch (_error) {
      return [];
    }
  }
  return [];
}

function firstOptionValue(options, fallback) {
  const list = asArray(options);
  for (const option of list) {
    const value = asText(option?.value);
    if (value) {
      return value;
    }
  }
  return asText(fallback);
}

function sanitizeNode(value, options, fallback) {
  const text = asText(value);
  const list = asArray(options);
  for (const option of list) {
    if (asText(option?.value) === text) {
      return text;
    }
  }
  return asText(fallback);
}

export function splitTriggerChainText(value) {
  const text = asText(value);
  if (!text) {
    return [];
  }
  return text
    .split('>')
    .map((token) => asText(token))
    .filter((token) => token.length > 0);
}

export function normalizeTriggerChain(value, options, fallback) {
  const fallbackValue = firstOptionValue(options, fallback);
  const fromList = toNodeArray(value);
  const source = fromList.length > 0 ? fromList : splitTriggerChainText(value);
  const out = [];

  for (const item of source) {
    const sanitized = sanitizeNode(item, options, fallbackValue);
    if (!sanitized) {
      continue;
    }
    out.push(sanitized);
  }

  if (out.length > 0) {
    return out;
  }
  return fallbackValue ? [fallbackValue] : [];
}

export function serializeTriggerChain(value, options, fallback) {
  return normalizeTriggerChain(value, options, fallback).join('>');
}
