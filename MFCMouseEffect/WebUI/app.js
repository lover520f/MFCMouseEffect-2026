(() => {
  const token = new URL(location.href).searchParams.get('token') || '';
  const el = (id) => document.getElementById(id);
  let connectionState = 'unknown';
  let hasRenderedSettings = false;
  let isReloading = false;
  let reloadRetryTimer = 0;
  let cachedSchema = null;
  let cachedSchemaLang = '';

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
      return;
    }
    if (next === 'unauthorized') {
      setActionButtonsEnabled(true);
      setStatus(t.unauthorized_hint || 'Unauthorized.', 'warn');
      return;
    }
    if (next === 'stopped') {
      setActionButtonsEnabled(true);
      setStatus(t.stopped_hint || 'Server stopped.', 'offline');
      return;
    }
    setActionButtonsEnabled(true);
    setStatus(t.disconnected_hint || 'Disconnected from server.', 'offline');
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
        wasmAction: handleWasmAction,
      });
    }

    if (window.MfxAutomationUi && typeof window.MfxAutomationUi.render === 'function') {
      window.MfxAutomationUi.render({
        schema,
        state: st.automation || {},
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
    if (window.MfxSectionWorkspace && typeof window.MfxSectionWorkspace.refresh === 'function') {
      window.MfxSectionWorkspace.refresh();
    }

    hasRenderedSettings = true;
    clearReloadRetryTimer();
    markConnection('online', true);
    const routeNotice = st.gpu_route_notice;
    if (routeNotice && routeNotice.message) {
      setStatus(routeNotice.message, routeNotice.level === 'warn' ? 'warn' : 'ok');
    }
  }

  async function refreshStateSnapshot(useCachedSchema) {
    const st = await apiGet('/api/state');
    const uiLang = st.ui_language || 'en-US';
    const canReuseSchema = !!cachedSchema && cachedSchemaLang === uiLang;
    const schema = (useCachedSchema && canReuseSchema)
      ? cachedSchema
      : await apiGet('/api/schema');
    renderSettingsSnapshot(schema, st);
    return st;
  }

  async function refreshAfterWasmAction() {
    try {
      await refreshStateSnapshot(true);
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

  async function handleWasmAction(action, payload) {
    try {
      if (blockActionWhenDisconnected()) {
        return { ok: false, error: 'blocked' };
      }
      const body = payload || {};
      if (action === 'catalog') {
        return await apiPost('/api/wasm/catalog', body);
      }
      if (action === 'enable') {
        setStatus(statusText('status_wasm_enabling', 'Enabling WASM plugin...'));
        const result = await apiPost('/api/wasm/enable', {});
        await refreshAfterWasmAction();
        return result;
      }
      if (action === 'disable') {
        setStatus(statusText('status_wasm_disabling', 'Disabling WASM plugin...'));
        const result = await apiPost('/api/wasm/disable', {});
        await refreshAfterWasmAction();
        return result;
      }
      if (action === 'reload') {
        setStatus(statusText('status_wasm_reloading', 'Reloading WASM plugin...'));
        const result = await apiPost('/api/wasm/reload', {});
        await refreshAfterWasmAction();
        return result;
      }
      if (action === 'loadManifest') {
        setStatus(statusText('status_wasm_loading_manifest', 'Loading WASM manifest...'));
        const result = await apiPost('/api/wasm/load-manifest', body);
        await refreshAfterWasmAction();
        return result;
      }
      if (action === 'setPolicy') {
        setStatus(statusText('status_wasm_updating_policy', 'Updating WASM policy...'));
        const result = await apiPost('/api/wasm/policy', body);
        await refreshAfterWasmAction();
        return result;
      }
      return { ok: false, error: 'unsupported action' };
    } catch (e) {
      if (e && e.code === 'unauthorized') {
        return { ok: false, error: 'unauthorized' };
      }
      setStatus(statusError('status_wasm_action_failed', 'WASM action failed: ', e), 'warn');
      return {
        ok: false,
        error: (e && e.message) ? e.message : String(e || ''),
      };
    }
  }

  function buildState() {
    const automation = (window.MfxAutomationUi && typeof window.MfxAutomationUi.read === 'function')
      ? window.MfxAutomationUi.read()
      : {
        enabled: false,
        mouse_mappings: [],
        gesture: {
          enabled: false,
          trigger_button: 'right',
          min_stroke_distance_px: 80,
          sample_step_px: 10,
          max_directions: 4,
          mappings: [],
        },
      };
    const form = settingsForm();
    const baseState = (form && typeof form.read === 'function')
      ? form.read()
      : {};

    return {
      ...baseState,
      automation,
    };
  }

  async function handleReloadAction() {
    try {
      if (blockActionWhenDisconnected()) return;
      setStatus(statusText('status_reloading', 'Reloading config...'));
      await apiPost('/api/reload', {});
      await reload();
    } catch (e) {
      if (e && e.code === 'unauthorized') return;
      setStatus(statusError('status_reload_failed', 'Reload failed: ', e), 'warn');
    }
  }

  async function handleSaveAction() {
    try {
      if (blockActionWhenDisconnected()) return;
      const automationValidation = (window.MfxAutomationUi && typeof window.MfxAutomationUi.validate === 'function')
        ? window.MfxAutomationUi.validate()
        : { ok: true };
      if (!automationValidation.ok) {
        setStatus(automationValidation.message || 'Please resolve mapping validation errors before applying.', 'warn');
        return;
      }
      setStatus(statusText('status_applying', 'Applying...'));
      const st = buildState();
      const res = await apiPost('/api/state', st);
      if (res.ok) {
        const latest = await apiGet('/api/state');
        const routeNotice = latest.gpu_route_notice;
        if (routeNotice && routeNotice.message) {
          setStatus(routeNotice.message, routeNotice.level === 'warn' ? 'warn' : 'ok');
        } else {
          setStatus(statusText('status_applied', 'Applied.'), 'ok');
        }
      } else {
        setStatus('Failed: ' + (res.error || ''), 'warn');
      }
    } catch (e) {
      if (e && e.code === 'unauthorized') return;
      setStatus(statusError('status_save_failed', 'Save failed: ', e), 'warn');
    }
  }

  async function handleResetAction() {
    try {
      if (blockActionWhenDisconnected()) return;
      const t = currentText();
      let confirmed = false;
      if (window.MfxDialog && typeof window.MfxDialog.showConfirm === 'function') {
        confirmed = await window.MfxDialog.showConfirm({
          title: t.dialog_title_confirm || 'Please confirm',
          message: t.confirm_reset,
          okText: t.dialog_btn_confirm_reset || 'Reset',
          cancelText: t.dialog_btn_cancel || 'Cancel'
        });
      } else {
        confirmed = confirm(t.confirm_reset);
      }
      if (!confirmed) return;
      setStatus(statusText('status_resetting', 'Resetting...'));
      const res = await apiPost('/api/reset', {});
      if (!res.ok) throw new Error(res.error || 'reset failed');
      await reload();
    } catch (e) {
      if (e && e.code === 'unauthorized') return;
      setStatus(statusError('status_reset_failed', 'Reset failed: ', e), 'warn');
    }
  }

  async function handleStopAction() {
    try {
      if (blockActionWhenDisconnected()) return;
      const res = await apiPost('/api/stop', {});
      if (!res.ok) throw new Error(res.error || 'stop failed');
      markConnection('stopped');
    } catch (e) {
      if (e && e.code === 'unauthorized') return;
      setStatus(statusError('status_stop_failed', 'Stop failed: ', e), 'warn');
    }
  }

  function handleLanguageChange() {
    const langEl = el('ui_language');
    if (!langEl) return;
    try {
      applyI18n(langEl.value);
    } catch (_error) {
      // Ignore i18n runtime errors on language switch to keep UI usable.
    }
    if (connectionState !== 'unknown') markConnection(connectionState, true);
  }

  function bindLegacyActionButtons() {
    const reloadBtn = el('btnReload');
    if (reloadBtn) reloadBtn.addEventListener('click', () => { handleReloadAction(); });

    const saveBtn = el('btnSave');
    if (saveBtn) saveBtn.addEventListener('click', () => { handleSaveAction(); });

    const resetBtn = el('btnReset');
    if (resetBtn) resetBtn.addEventListener('click', () => { handleResetAction(); });

    const stopBtn = el('btnStop');
    if (stopBtn) stopBtn.addEventListener('click', () => { handleStopAction(); });
  }

  function bindShellActions() {
    const shell = shellUi();
    if (!shell || typeof shell.onAction !== 'function') {
      bindLegacyActionButtons();
      return;
    }
    shell.onAction((type) => {
      if (type === 'reload') {
        handleReloadAction();
        return;
      }
      if (type === 'save') {
        handleSaveAction();
        return;
      }
      if (type === 'reset') {
        handleResetAction();
        return;
      }
      if (type === 'stop') {
        handleStopAction();
      }
    });
  }

  function bindLanguageChange() {
    const langEl = el('ui_language');
    if (!langEl) return;
    langEl.addEventListener('change', handleLanguageChange);
  }

  bindShellActions();
  bindLanguageChange();

  if (window.MfxSectionWorkspace && typeof window.MfxSectionWorkspace.init === 'function') {
    window.MfxSectionWorkspace.init();
  }

  startHealthCheck();
  reload().then(() => {
    if (window.MfxSectionWorkspace && typeof window.MfxSectionWorkspace.refresh === 'function') {
      window.MfxSectionWorkspace.refresh();
    }
  }).catch(e => {
    handleReloadFailure(e);
  });
})();

