(() => {
  const token = new URL(location.href).searchParams.get('token') || '';
  const el = (id) => document.getElementById(id);
  let connectionState = 'unknown';
  let hasRenderedSettings = false;
  let isReloading = false;
  let reloadRetryTimer = 0;
  let cachedSchema = null;
  let cachedSchemaLang = '';
  let cachedState = null;
  let actionHandlers = { general: null, wasm: null };
  let gestureDebug = null;

  const I18N = window.MfxWebI18n || {};

  function shellUi() {
    return window.MfxWebShell || null;
  }

  function createI18nRuntime() {
    if (!window.MfxI18nRuntime || typeof window.MfxI18nRuntime.create !== 'function') {
      return null;
    }
    function safeSync(fn, text) {
      if (typeof fn !== 'function') return;
      try {
        fn(text);
      } catch (_error) {
        // Do not block settings reload because of i18n consumer sync failures.
      }
    }

    return window.MfxI18nRuntime.create({
      catalog: I18N,
      getElement: el,
      syncConsumers: (text) => {
        safeSync(window.MfxAutomationUi?.syncI18n, text);
        safeSync(window.MfxSectionWorkspace?.syncI18n, text);
        safeSync(window.MfxWasmSection?.syncI18n, text);
      },
    });
  }

  const i18nRuntime = createI18nRuntime();

  function ensureI18nRuntime() {
    if (!i18nRuntime) {
      throw new Error('i18n-runtime unavailable');
    }
    return i18nRuntime;
  }

  function applyI18n(lang) {
    return ensureI18nRuntime().apply(lang);
  }

  function setStatus(msg, tone) {
    const shell = shellUi();
    if (shell && typeof shell.setStatus === 'function') {
      shell.setStatus(msg, tone);
      return;
    }
    const statusEl = document.getElementById('status');
    if (!statusEl) return;
    if (!msg) {
      statusEl.textContent = '';
      statusEl.className = 'status';
      return;
    }
    statusEl.textContent = msg;
    let cls = 'status show';
    if (tone === 'warn') cls += ' warn';
    if (tone === 'ok') cls += ' ok';
    if (tone === 'offline') cls += ' offline';
    statusEl.className = cls;
  }

  function currentText() {
    return ensureI18nRuntime().currentText();
  }

  function statusText(key, fallback) {
    const t = currentText();
    return (t && t[key]) || fallback;
  }

  function statusError(prefixKey, fallbackPrefix, error) {
    const msg = (error && error.message) ? error.message : String(error || '');
    return statusText(prefixKey, fallbackPrefix) + msg;
  }

  function pickRuntimeNotice(stateSnapshot) {
    const st = stateSnapshot || {};
    const inputNotice = st.input_capture_notice;
    if (inputNotice && inputNotice.message) {
      return inputNotice;
    }
    const routeNotice = st.gpu_route_notice;
    if (routeNotice && routeNotice.message) {
      return routeNotice;
    }
    return null;
  }

  function normalizeManifestPathForCompare(path) {
    const value = `${path || ''}`.trim();
    if (!value) {
      return '';
    }
    return value
      .replace(/\\/g, '/')
      .replace(/\/+/g, '/')
      .toLowerCase();
  }

  function mergeInputIndicatorState(dirtyState, freshState) {
    const dirtyIndicator = (dirtyState && (dirtyState.input_indicator || dirtyState.mouse_indicator)) || null;
    const freshIndicator = (freshState && (freshState.input_indicator || freshState.mouse_indicator)) || null;
    if (!dirtyIndicator) {
      return freshIndicator;
    }
    if (!freshIndicator) {
      return dirtyIndicator;
    }
    const merged = { ...dirtyIndicator };
    if (Object.prototype.hasOwnProperty.call(freshIndicator, 'wasm_manifest_path')) {
      merged.wasm_manifest_path = freshIndicator.wasm_manifest_path;
    }
    return merged;
  }

  async function ensureInputIndicatorRuntimeManifestSynced(stateSnapshot) {
    const current = stateSnapshot || {};
    const indicator = current.input_indicator || current.mouse_indicator || {};
    const renderMode = `${indicator.render_mode || ''}`.trim().toLowerCase();
    const configuredManifestPath = `${indicator.wasm_manifest_path || ''}`.trim();
    if (renderMode !== 'wasm' || !configuredManifestPath) {
      return { state: current, attempted: false, ok: true };
    }

    const activeManifestPath = `${current?.wasm?.indicator_active_manifest_path || ''}`.trim();
    if (normalizeManifestPathForCompare(configuredManifestPath) === normalizeManifestPathForCompare(activeManifestPath)) {
      return { state: current, attempted: false, ok: true };
    }

    const result = await apiPost('/api/wasm/load-manifest', {
      surface: 'indicator',
      manifest_path: configuredManifestPath,
    });
    const refreshed = await apiGet('/api/state');
    return {
      state: refreshed,
      attempted: true,
      ok: !!result?.ok,
      error: `${result?.error || ''}`.trim(),
      errorCode: `${result?.error_code || ''}`.trim(),
    };
  }

  function isObject(value) {
    return !!value && typeof value === 'object';
  }

  function currentRuntimePlatform() {
    return `${cachedSchema?.capabilities?.platform || ''}`.trim();
  }

  function setActionHandlers(handlers) {
    actionHandlers = {
      general: handlers?.general || null,
      wasm: handlers?.wasm || null,
    };
  }

  function setGestureDebug(handler) {
    gestureDebug = handler || null;
  }

  function setActionButtonsEnabled(enabled) {
    const shell = shellUi();
    if (shell && typeof shell.setActionsEnabled === 'function') {
      shell.setActionsEnabled(enabled);
      return;
    }
    ['btnReload', 'btnReset', 'btnStop', 'btnSave'].forEach((id) => {
      const node = el(id);
      if (node) node.disabled = !enabled;
    });
  }

  function blockActionWhenDisconnected() {
    const t = currentText();
    if (connectionState === 'online' || connectionState === 'unknown') return false;
    const showBlockedDialog = (message) => {
      if (window.MfxDialog && typeof window.MfxDialog.showNotice === 'function') {
        window.MfxDialog.showNotice({
          title: t.dialog_title_notice || 'Connection lost',
          message,
          okText: t.dialog_btn_ok || 'OK'
        });
        return;
      }
      alert(message);
    };
    if (connectionState === 'unauthorized') {
      showBlockedDialog(t.unauthorized_blocked || t.unauthorized_hint || 'Unauthorized.');
      return true;
    }
    if (connectionState === 'stopped') {
      showBlockedDialog(t.stopped_blocked || t.stopped_hint || 'Server stopped.');
      return true;
    }
    showBlockedDialog(t.disconnected_blocked || t.disconnected_hint || 'Disconnected from server.');
    return true;
  }

  function markConnection(next, force) {
    if (!force && connectionState === next) return;
    connectionState = next;
    const t = currentText();
    if (next === 'online') {
      setActionButtonsEnabled(true);
      if (!hasRenderedSettings && !isReloading) {
        reload().catch((e) => {
          handleReloadFailure(e);
        });
        return;
      }
      if (!hasRenderedSettings) {
        // Keep current loading text before first successful render to avoid
        // early "Ready" text in the wrong language.
        return;
      }
      setStatus(t.status_ready || 'Ready.', 'ok');
      if (gestureDebug && typeof gestureDebug.refresh === 'function') {
        gestureDebug.refresh();
      }
      return;
    }
    if (next === 'unauthorized') {
      setActionButtonsEnabled(true);
      setStatus(t.unauthorized_hint || 'Unauthorized.', 'warn');
      if (gestureDebug && typeof gestureDebug.clear === 'function') {
        gestureDebug.clear();
      }
      return;
    }
    if (next === 'stopped') {
      setActionButtonsEnabled(true);
      setStatus(t.stopped_hint || 'Server stopped.', 'offline');
      if (gestureDebug && typeof gestureDebug.clear === 'function') {
        gestureDebug.clear();
      }
      return;
    }
    setActionButtonsEnabled(true);
    setStatus(t.disconnected_hint || 'Disconnected from server.', 'offline');
    if (gestureDebug && typeof gestureDebug.clear === 'function') {
      gestureDebug.clear();
    }
  }

  function clearReloadRetryTimer() {
    if (!reloadRetryTimer) return;
    window.clearTimeout(reloadRetryTimer);
    reloadRetryTimer = 0;
  }

  function scheduleReloadRetry() {
    if (reloadRetryTimer) return;
    if (isReloading) return;
    reloadRetryTimer = window.setTimeout(() => {
      reloadRetryTimer = 0;
      reload().catch((e) => {
        handleReloadFailure(e);
      });
    }, 1200);
  }

  function handleReloadFailure(e) {
    if (e && e.code === 'unauthorized') return;
    hasRenderedSettings = false;
    markConnection('offline');
    setStatus(statusError('status_reload_failed', 'Reload failed: ', e), 'warn');
    scheduleReloadRetry();
  }

  function pickLang() {
    return ensureI18nRuntime().pickLang();
  }

  function showUnauthorized() {
    markConnection('unauthorized');
  }

  function createWebApiClient() {
    if (!window.MfxWebApi || typeof window.MfxWebApi.create !== 'function') {
      return null;
    }
    return window.MfxWebApi.create({
      token,
      healthCheckMs: 3000,
      onUnauthorized: showUnauthorized,
      onConnectionState: markConnection,
    });
  }

  const webApi = createWebApiClient();

  function ensureWebApi() {
    if (!webApi) {
      throw new Error('web-api unavailable');
    }
    return webApi;
  }

  async function apiGet(path) {
    return ensureWebApi().apiGet(path);
  }

  async function apiPost(path, obj) {
    return ensureWebApi().apiPost(path, obj);
  }

  function startHealthCheck() {
    ensureWebApi().startHealthCheck();
  }

  function settingsForm() {
    return window.MfxSettingsForm || null;
  }

  function renderSettingsSnapshot(schema, st) {
    const uiLang = st.ui_language || 'en-US';
    const i18n = I18N[uiLang] || I18N['en-US'] || {};
    cachedSchema = schema;
    cachedSchemaLang = uiLang;
    cachedState = st;

    try {
      applyI18n(uiLang);
    } catch (_error) {
      // Keep settings rendering alive even if a consumer throws during i18n sync.
    }
    const form = settingsForm();
    if (form && typeof form.render === 'function') {
      form.render({
        schema,
        state: st,
        i18n,
        generalAction: actionHandlers.general,
        wasmAction: actionHandlers.wasm,
      });
    }

    if (window.MfxAutomationUi && typeof window.MfxAutomationUi.render === 'function') {
      const automationState = {
        ...(st.automation || {}),
        input_automation_gesture_route_status: st.input_automation_gesture_route_status || null,
      };
      window.MfxAutomationUi.render({
        schema,
        state: automationState,
        i18n,
      });
      if (typeof window.MfxAutomationUi.syncI18n === 'function') {
        try {
          window.MfxAutomationUi.syncI18n(i18n);
        } catch (_error) {
          // Keep reload resilient if automation i18n sync fails.
        }
      }
    }
    if (gestureDebug && typeof gestureDebug.onStateRendered === 'function') {
      gestureDebug.onStateRendered(st);
    } else if (window.MfxSectionWorkspace && typeof window.MfxSectionWorkspace.refresh === 'function') {
      window.MfxSectionWorkspace.refresh();
    }

    // Re-apply i18n after sections mount/update so newly inserted nodes
    // (especially Svelte section roots) do not keep fallback English text.
    try {
      applyI18n(uiLang);
    } catch (_error) {
      // Ignore post-render i18n sync failures to keep page usable.
    }

    hasRenderedSettings = true;
    clearReloadRetryTimer();
    markConnection('online', true);
    const runtimeNotice = pickRuntimeNotice(st);
    if (runtimeNotice) {
      setStatus(runtimeNotice.message, runtimeNotice.level === 'warn' ? 'warn' : 'ok');
    }
  }

  async function refreshStateSnapshot(useCachedSchema) {
    let st = null;
    let schema = null;

    if (!cachedSchema) {
      const results = await Promise.all([
        apiGet('/api/state'),
        apiGet('/api/schema'),
      ]);
      st = results[0];
      schema = results[1];
    } else {
      st = await apiGet('/api/state');
      const uiLang = st.ui_language || 'en-US';
      const canReuseSchema = !!cachedSchema && cachedSchemaLang === uiLang;
      schema = (useCachedSchema && canReuseSchema)
        ? cachedSchema
        : await apiGet('/api/schema');
    }

    renderSettingsSnapshot(schema, st);
    return st;
  }

  async function refreshAfterWasmAction() {
    try {
      const dirtyState = buildState();
      const st = await apiGet('/api/state');
      const mergedIndicator = mergeInputIndicatorState(dirtyState, st);
      const mergedState = {
        ...st,
        ...dirtyState,
        wasm: st.wasm, // Prefer fresh diagnostics and plugin states from backend
        input_indicator_wasm_route_status: st.input_indicator_wasm_route_status,
        input_automation_gesture_route_status: st.input_automation_gesture_route_status,
        input_capture_notice: st.input_capture_notice,
        gpu_route_notice: st.gpu_route_notice,
      };

      if (mergedIndicator) {
        mergedState.input_indicator = mergedIndicator;
      }

      const uiLang = mergedState.ui_language || 'en-US';
      const canReuseSchema = !!cachedSchema && cachedSchemaLang === uiLang;
      const schema = canReuseSchema ? cachedSchema : await apiGet('/api/schema');
      renderSettingsSnapshot(schema, mergedState);
    } catch (e) {
      if (e && e.code === 'unauthorized') {
        throw e;
      }
      await reload();
    }
  }

  async function reload() {
    if (isReloading) return;
    isReloading = true;
    setStatus(statusText('status_loading', 'Loading...'));
    try {
      await refreshStateSnapshot(false);
    } finally {
      isReloading = false;
    }
  }

  function getCachedState() {
    return cachedState;
  }

  function setCachedState(state) {
    cachedState = state;
  }

  function mergeCachedState(patch) {
    cachedState = {
      ...(isObject(cachedState) ? cachedState : {}),
      ...(isObject(patch) ? patch : {}),
    };
  }

  function getCachedSchema() {
    return cachedSchema;
  }

  function getConnectionState() {
    return connectionState;
  }

  function getHasRenderedSettings() {
    return hasRenderedSettings;
  }

  window.MfxAppCore = {
    apiGet,
    apiPost,
    applyI18n,
    blockActionWhenDisconnected,
    currentRuntimePlatform,
    currentText,
    ensureInputIndicatorRuntimeManifestSynced,
    getCachedSchema,
    getCachedState,
    getConnectionState,
    getHasRenderedSettings,
    isObject,
    markConnection,
    mergeCachedState,
    pickLang,
    pickRuntimeNotice,
    reload,
    renderSettingsSnapshot,
    refreshAfterWasmAction,
    refreshStateSnapshot,
    setActionHandlers,
    setGestureDebug,
    setStatus,
    settingsForm,
    startHealthCheck,
    statusError,
    statusText,
    handleReloadFailure,
  };
})();
