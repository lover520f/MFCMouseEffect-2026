function authToken() {
  try {
    return new URL(window.location.href).searchParams.get('token') || '';
  } catch (_error) {
    return '';
  }
}

function authHeaders(extraHeaders) {
  const headers = {
    ...(extraHeaders || {}),
  };
  const token = authToken();
  if (token) {
    headers['X-MFCMouseEffect-Token'] = token;
  }
  return headers;
}

function endpointForAction(action) {
  if (action === 'catalog') return '/api/wasm/catalog';
  if (action === 'enable') return '/api/wasm/enable';
  if (action === 'disable') return '/api/wasm/disable';
  if (action === 'reload') return '/api/wasm/reload';
  if (action === 'loadManifest') return '/api/wasm/load-manifest';
  if (action === 'importSelected') return '/api/wasm/import-selected';
  if (action === 'importFromFolderDialog') return '/api/wasm/import-from-folder-dialog';
  if (action === 'exportAll') return '/api/wasm/export-all';
  if (action === 'setPolicy') return '/api/wasm/policy';
  return '';
}

export async function runRemoteWasmAction(action, payload) {
  const endpoint = endpointForAction(action);
  if (!endpoint) {
    return { ok: false, error: `unsupported action: ${action}` };
  }

  try {
    const response = await fetch(endpoint, {
      method: 'POST',
      headers: authHeaders({ 'Content-Type': 'application/json' }),
      body: JSON.stringify(payload || {}),
    });
    const text = await response.text();
    const result = text ? JSON.parse(text) : {};
    if (!response.ok) {
      return {
        ok: false,
        error: `${result?.error || text || `HTTP ${response.status}`}`.trim(),
        error_code: `${result?.error_code || ''}`.trim(),
      };
    }
    return result;
  } catch (error) {
    return {
      ok: false,
      error: error instanceof Error ? error.message : String(error || 'unknown error'),
    };
  }
}
