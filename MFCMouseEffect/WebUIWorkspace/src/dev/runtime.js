const DEV_RUNTIME_ROUTE = '/__mfx/dev-runtime';

function trimText(value) {
  return `${value || ''}`.trim();
}

function updateUrlToken(token) {
  const nextToken = trimText(token);
  if (!nextToken) {
    return;
  }
  const currentUrl = new URL(window.location.href);
  if (trimText(currentUrl.searchParams.get('token')) === nextToken) {
    return;
  }
  currentUrl.searchParams.set('token', nextToken);
  window.history.replaceState(null, '', currentUrl.toString());
}

export async function loadDevRuntime() {
  const response = await fetch(DEV_RUNTIME_ROUTE, { cache: 'no-store' });
  if (!response.ok) {
    throw new Error(`dev runtime request failed: ${response.status}`);
  }

  const runtime = await response.json();
  if (!runtime || typeof runtime !== 'object') {
    throw new Error('dev runtime payload invalid');
  }

  updateUrlToken(runtime.token);
  window.__MFX_DEV_RUNTIME__ = runtime;
  return runtime;
}

export function renderDevRuntimeError(error, runtime) {
  const body = document.body;
  if (!body) {
    return;
  }

  const message = error instanceof Error ? error.message : String(error || 'unknown error');
  body.innerHTML = `
    <main style="font-family: ui-sans-serif, system-ui, sans-serif; padding: 32px; color: #17324d;">
      <section style="max-width: 780px; margin: 0 auto; border: 1px solid rgba(191,211,235,0.92); border-radius: 20px; padding: 24px; background: linear-gradient(180deg, rgba(250,252,255,0.98), rgba(241,247,255,0.98)); box-shadow: 0 18px 50px rgba(50,84,122,0.12);">
        <div style="font-size: 13px; font-weight: 700; color: #2f6fb8; letter-spacing: 0.04em; text-transform: uppercase;">WebUI Dev</div>
        <h1 style="margin: 10px 0 0; font-size: 28px; line-height: 1.2;">Runtime unavailable</h1>
        <p style="margin: 12px 0 0; font-size: 15px; line-height: 1.7; color: #3e5c79;">
          Start the host with <code>./mfx fast --debug</code> or <code>./mfx start --debug</code>,
          or provide
          <code>MFX_WEBUI_DEV_BASE_URL</code> and <code>MFX_WEBUI_DEV_TOKEN</code> before <code>pnpm run dev</code>.
        </p>
        <div style="margin-top: 16px; border-radius: 14px; background: rgba(255,255,255,0.9); border: 1px solid rgba(205,221,240,0.96); padding: 14px 16px; font-size: 14px; line-height: 1.6;">
          <div><strong>Probe file:</strong> <code>${trimText(runtime?.probeFile) || '(not set)'}</code></div>
          <div><strong>Backend:</strong> <code>${trimText(runtime?.baseUrl) || '(not found)'}</code></div>
          <div><strong>Error:</strong> <code>${message}</code></div>
        </div>
      </section>
    </main>
  `;
}
