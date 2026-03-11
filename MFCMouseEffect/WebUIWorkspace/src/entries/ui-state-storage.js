const UI_STATE_KEY_PREFIX = 'mfx.websettings.ui-state.';
const COOKIE_MAX_AGE_SECONDS = 60 * 60 * 24 * 365;

function storageKey(namespace) {
  return `${UI_STATE_KEY_PREFIX}${namespace}`;
}

function localStorageGet(key) {
  if (typeof window === 'undefined' || !window.localStorage) {
    return '';
  }
  try {
    return `${window.localStorage.getItem(key) || ''}`.trim();
  } catch (_error) {
    return '';
  }
}

function localStorageSet(key, value) {
  if (typeof window === 'undefined' || !window.localStorage) {
    return;
  }
  try {
    window.localStorage.setItem(key, value);
  } catch (_error) {
    // Ignore local storage failures.
  }
}

function cookieGet(key) {
  if (typeof document === 'undefined') {
    return '';
  }
  const encodedKey = encodeURIComponent(key);
  const source = `${document.cookie || ''}`.trim();
  if (!source) {
    return '';
  }
  const entries = source.split(';');
  for (const entry of entries) {
    const pair = `${entry || ''}`.trim();
    if (!pair) {
      continue;
    }
    const idx = pair.indexOf('=');
    if (idx <= 0) {
      continue;
    }
    const name = pair.slice(0, idx).trim();
    if (name !== encodedKey) {
      continue;
    }
    const rawValue = pair.slice(idx + 1).trim();
    try {
      return decodeURIComponent(rawValue);
    } catch (_error) {
      return '';
    }
  }
  return '';
}

function cookieSet(key, value) {
  if (typeof document === 'undefined') {
    return;
  }
  const encodedKey = encodeURIComponent(key);
  const encodedValue = encodeURIComponent(value);
  document.cookie = `${encodedKey}=${encodedValue}; Max-Age=${COOKIE_MAX_AGE_SECONDS}; Path=/; SameSite=Lax`;
}

function safeParseJson(raw) {
  const text = `${raw || ''}`.trim();
  if (!text) {
    return null;
  }
  try {
    const parsed = JSON.parse(text);
    if (!parsed || typeof parsed !== 'object') {
      return null;
    }
    return parsed;
  } catch (_error) {
    return null;
  }
}

function safeStringifyJson(value) {
  try {
    return JSON.stringify(value || {});
  } catch (_error) {
    return '';
  }
}

export function readUiState(namespace) {
  const key = storageKey(namespace);
  const localRaw = localStorageGet(key);
  const localParsed = safeParseJson(localRaw);
  if (localParsed) {
    return localParsed;
  }

  const cookieRaw = cookieGet(key);
  const cookieParsed = safeParseJson(cookieRaw);
  if (cookieParsed) {
    // Backfill current origin storage for faster later reads.
    localStorageSet(key, cookieRaw);
    return cookieParsed;
  }

  return null;
}

export function writeUiState(namespace, value) {
  const key = storageKey(namespace);
  const serialized = safeStringifyJson(value);
  if (!serialized) {
    return;
  }
  localStorageSet(key, serialized);
  cookieSet(key, serialized);
}
