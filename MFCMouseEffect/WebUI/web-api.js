(() => {
  function create(options) {
    const opts = options || {};
    const token = opts.token || '';
    const healthCheckMs = typeof opts.healthCheckMs === 'number' ? opts.healthCheckMs : 3000;
    const onUnauthorized = typeof opts.onUnauthorized === 'function' ? opts.onUnauthorized : () => {};
    const onConnectionState = typeof opts.onConnectionState === 'function' ? opts.onConnectionState : () => {};
    let healthTimer = 0;
    let visibilityBound = false;

    function authHeaders(extraHeaders) {
      const headers = Object.assign({}, extraHeaders || {});
      headers['X-MFCMouseEffect-Token'] = token;
      return headers;
    }

    async function parseErrorResponse(response) {
      try {
        const text = await response.text();
        return text || `HTTP ${response.status}`;
      } catch (_error) {
        return `HTTP ${response.status}`;
      }
    }

    function unauthorizedError() {
      const err = new Error('unauthorized');
      err.code = 'unauthorized';
      return err;
    }

    async function apiGet(path) {
      const response = await fetch(path, { headers: authHeaders() });
      if (!response.ok) {
        if (response.status === 401) {
          onUnauthorized();
          throw unauthorizedError();
        }
        throw new Error(await parseErrorResponse(response));
      }
      return await response.json();
    }

    async function apiPost(path, payload) {
      const response = await fetch(path, {
        method: 'POST',
        headers: authHeaders({ 'Content-Type': 'application/json' }),
        body: JSON.stringify(payload || {}),
      });
      if (!response.ok) {
        if (response.status === 401) {
          onUnauthorized();
          throw unauthorizedError();
        }
        throw new Error(await parseErrorResponse(response));
      }
      return await response.json();
    }

    async function probeConnection() {
      try {
        const response = await fetch('/api/state', {
          headers: authHeaders(),
          cache: 'no-store',
        });
        if (response.status === 401) {
          onConnectionState('unauthorized');
          return false;
        }
        if (!response.ok) {
          onConnectionState('offline');
          return false;
        }
        onConnectionState('online');
        return true;
      } catch (_error) {
        onConnectionState('offline');
        return false;
      }
    }

    function startHealthCheck() {
      if (healthTimer) return;
      healthTimer = window.setInterval(() => {
        probeConnection();
      }, healthCheckMs);

      if (!visibilityBound) {
        document.addEventListener('visibilitychange', () => {
          if (!document.hidden) {
            probeConnection();
          }
        });
        visibilityBound = true;
      }
    }

    function stopHealthCheck() {
      if (!healthTimer) return;
      window.clearInterval(healthTimer);
      healthTimer = 0;
    }

    return {
      apiGet,
      apiPost,
      probeConnection,
      startHealthCheck,
      stopHealthCheck,
    };
  }

  window.MfxWebApi = {
    create,
  };
})();
