function authToken() {
  return new URL(window.location.href).searchParams.get('token') || '';
}

async function postJson(path, payload) {
  const headers = { 'Content-Type': 'application/json' };
  const token = authToken();
  if (token) {
    headers['X-MFCMouseEffect-Token'] = token;
  }

  const response = await fetch(path, {
    method: 'POST',
    headers,
    body: JSON.stringify(payload || {}),
  });

  if (!response.ok) {
    const text = await response.text();
    throw new Error(text || `HTTP ${response.status}`);
  }

  const body = await response.json();
  if (!body || body.ok === false) {
    throw new Error((body && body.error) || 'capture api failed');
  }
  return body;
}

export async function startShortcutCapture(timeoutMs = 10000) {
  const result = await postJson('/api/automation/shortcut-capture/start', { timeout_ms: timeoutMs });
  return `${result.session || ''}`;
}

export async function pollShortcutCapture(sessionId) {
  if (!sessionId) {
    return { status: 'invalid', shortcut: '' };
  }

  const result = await postJson('/api/automation/shortcut-capture/poll', { session: sessionId });
  return {
    status: `${result.status || 'invalid'}`,
    shortcut: `${result.shortcut || ''}`,
  };
}

export async function stopShortcutCapture(sessionId) {
  if (!sessionId) {
    return;
  }
  await postJson('/api/automation/shortcut-capture/stop', { session: sessionId });
}
