(() => {
  const token = new URL(location.href).searchParams.get('token') || '';
  const el = (id) => document.getElementById(id);
  const statusEl = document.getElementById('status');
  const healthCheckMs = 3000;
  let healthTimer = 0;
  let connectionState = 'unknown';

  const I18N = {
    "en-US": {
      title: "MFCMouseEffect Settings",
      subtitle: "Local UI · instant apply · saved to config.json",
      hint_apply: "Change values then click Apply.",
      btn_star: "Star",
      btn_reload: "Reload",
      btn_reset: "Reset",
      btn_stop: "Stop",
      btn_apply: "Apply",
      tip_star: "Open project page",
      tip_reload: "Reload config.json from disk",
      tip_reset: "Reset all settings to defaults",
      tip_stop: "Stop local server (reopen from tray)",
      tip_apply: "Apply current form values",
      confirm_reset: "Reset to defaults? This cannot be undone.",
      stopped_hint: "Server stopped. Reopen from tray to continue.",
      disconnected_hint: "Disconnected from local server. Reopen from tray to continue.",
      unauthorized_hint: "Token expired. Reopen settings from the tray.",
      disconnected_blocked: "Disconnected. This action is unavailable. Reopen settings from the tray.",
      stopped_blocked: "Server is stopped. This action is unavailable. Reopen settings from the tray.",
      unauthorized_blocked: "Token expired. This action is unavailable. Reopen settings from the tray.",
      dialog_title_notice: "Connection lost",
      dialog_title_confirm: "Please confirm",
      dialog_btn_ok: "Got it",
      dialog_btn_cancel: "Cancel",
      dialog_btn_confirm_reset: "Reset",
      status_loading: "Loading...",
      status_reloading: "Reloading config...",
      status_applying: "Applying...",
      status_applied: "Applied.",
      status_resetting: "Resetting...",
      status_reload_failed: "Reload failed: ",
      status_save_failed: "Save failed: ",
      status_reset_failed: "Reset failed: ",
      status_stop_failed: "Stop failed: ",
      status_load_failed: "Load failed: ",
      status_ready: "Ready.",
      section_general: "General",
      section_effects: "Active Effects",
      section_text: "Text Content (Click/Text)",
      section_trail_tuning: "Trail Tuning",
      label_language: "Language",
      label_theme: "Theme",
      label_click: "Click",
      label_trail: "Trail",
      label_scroll: "Scroll",
      label_hold: "Hold",
      label_hover: "Hover",
      label_texts: "Comma separated",
      placeholder_texts: "happy,healthy",
      hint_texts: "Use English comma \",\" to separate words.",
      label_style_preset: "Style preset",
      hint_trail_preset: "Preset name only; values below are what actually apply.",
      label_idle_fade: "Idle fade start/end (ms)",
      hint_idle_fade: "Controls how fast the trail converges after the mouse stops (0 = default).",
      label_streamer_profile: "streamer duration/max",
      label_electric_profile: "electric duration/max",
      label_meteor_profile: "meteor duration/max",
      label_tubes_profile: "tubes duration/max",
      label_line_profile: "line duration/max",
      label_streamer_params: "streamer glow/core/head",
      label_electric_params: "electric amp/fork",
      label_meteor_params: "meteor rate/speed",
      hint_clamp: "Values are clamped to safe ranges when applied.",
      style_default: "Default",
      style_snappy: "Snappy",
      style_long: "Long",
      style_cinematic: "Cinematic",
      style_custom: "Custom"
    },
    "zh-CN": {
      title: "MFCMouseEffect \u8bbe\u7f6e",
      subtitle: "\u672c\u5730\u8bbe\u7f6e\u9875 \u00b7 \u4fee\u6539\u5373\u65f6\u751f\u6548 \u00b7 \u4fdd\u5b58\u5230 config.json",
      hint_apply: "\u4fee\u6539\u5b8c\u540e\u70b9\u51fb\u201c\u5e94\u7528\u201d\u3002",
      btn_star: "Star \u9879\u76ee",
      btn_reload: "\u91cd\u8f7d",
      btn_reset: "\u6062\u590d\u9ed8\u8ba4",
      btn_stop: "\u5173\u95ed\u76d1\u542c",
      btn_apply: "\u5e94\u7528",
      tip_star: "\u6253\u5f00\u9879\u76ee\u4e3b\u9875",
      tip_reload: "\u4ece\u78c1\u76d8\u91cd\u8f7d config.json",
      tip_reset: "\u5c06\u6240\u6709\u8bbe\u7f6e\u6062\u590d\u4e3a\u9ed8\u8ba4",
      tip_stop: "\u5173\u95ed\u672c\u5730\u76d1\u542c\uff08\u91cd\u65b0\u4ece\u6258\u76d8\u6253\u5f00\uff09",
      tip_apply: "\u5e94\u7528\u5f53\u524d\u503c",
      confirm_reset: "\u786e\u5b9a\u6062\u590d\u9ed8\u8ba4\u5417\uff1f\u65e0\u6cd5\u64a4\u9500\u3002",
      stopped_hint: "\u670d\u52a1\u5df2\u5173\u95ed\uff0c\u8bf7\u4ece\u6258\u76d8\u91cd\u65b0\u6253\u5f00\u3002",
      disconnected_hint: "\u5df2\u4e0e\u672c\u5730\u670d\u52a1\u65ad\u5f00\u8fde\u63a5\uff0c\u8bf7\u4ece\u6258\u76d8\u91cd\u65b0\u6253\u5f00\u3002",
      unauthorized_hint: "Token \u5df2\u5931\u6548\uff0c\u8bf7\u4ece\u6258\u76d8\u91cd\u65b0\u6253\u5f00\u8bbe\u7f6e\u3002",
      disconnected_blocked: "\u5f53\u524d\u5df2\u65ad\u5f00\u8fde\u63a5\uff0c\u65e0\u6cd5\u6267\u884c\u8be5\u64cd\u4f5c\uff0c\u8bf7\u4ece\u6258\u76d8\u91cd\u65b0\u6253\u5f00\u3002",
      stopped_blocked: "\u670d\u52a1\u5df2\u5173\u95ed\uff0c\u65e0\u6cd5\u6267\u884c\u8be5\u64cd\u4f5c\uff0c\u8bf7\u4ece\u6258\u76d8\u91cd\u65b0\u6253\u5f00\u3002",
      unauthorized_blocked: "Token \u5df2\u5931\u6548\uff0c\u65e0\u6cd5\u6267\u884c\u8be5\u64cd\u4f5c\uff0c\u8bf7\u4ece\u6258\u76d8\u91cd\u65b0\u6253\u5f00\u3002",
      dialog_title_notice: "\u8fde\u63a5\u5f02\u5e38",
      dialog_title_confirm: "\u8bf7\u786e\u8ba4",
      dialog_btn_ok: "\u6211\u77e5\u9053\u4e86",
      dialog_btn_cancel: "\u53d6\u6d88",
      dialog_btn_confirm_reset: "\u786e\u8ba4\u91cd\u7f6e",
      status_loading: "\u6b63\u5728\u52a0\u8f7d...",
      status_reloading: "\u6b63\u5728\u91cd\u8f7d\u914d\u7f6e...",
      status_applying: "\u6b63\u5728\u5e94\u7528...",
      status_applied: "\u5df2\u5e94\u7528\u3002",
      status_resetting: "\u6b63\u5728\u6062\u590d\u9ed8\u8ba4...",
      status_reload_failed: "\u91cd\u8f7d\u5931\u8d25\uff1a",
      status_save_failed: "\u5e94\u7528\u5931\u8d25\uff1a",
      status_reset_failed: "\u6062\u590d\u5931\u8d25\uff1a",
      status_stop_failed: "\u5173\u95ed\u76d1\u542c\u5931\u8d25\uff1a",
      status_load_failed: "\u52a0\u8f7d\u5931\u8d25\uff1a",
      status_ready: "\u5c31\u7eea\u3002",
      section_general: "\u4e00\u822c",
      section_effects: "\u7279\u6548\u9009\u62e9",
      section_text: "\u6587\u5b57\u5185\u5bb9\uff08\u70b9\u51fb/\u6587\u5b57\uff09",
      section_trail_tuning: "\u62d6\u5c3e\u8c03\u53c2",
      label_language: "\u8bed\u8a00",
      label_theme: "\u4e3b\u9898",
      label_click: "\u70b9\u51fb",
      label_trail: "\u62d6\u5c3e",
      label_scroll: "\u6eda\u8f6e",
      label_hold: "\u957f\u6309",
      label_hover: "\u60ac\u505c",
      label_texts: "\u9017\u53f7\u5206\u9694",
      placeholder_texts: "\u7f8e\u4e3d,\u5065\u5eb7,\u5e78\u798f",
      hint_texts: "\u8bf7\u4f7f\u7528\u82f1\u6587\u9017\u53f7 \",\" \u5206\u9694\u3002",
      label_style_preset: "\u9884\u8bbe\u98ce\u683c",
      hint_trail_preset: "\u9884\u8bbe\u53ea\u662f\u540d\u79f0\uff0c\u4ee5\u4e0b\u6570\u503c\u624d\u662f\u5b9e\u9645\u751f\u6548\u3002",
      label_idle_fade: "\u505c\u7559\u6de1\u51fa \u5f00\u59cb/\u7ed3\u675f(ms)",
      hint_idle_fade: "\u63a7\u5236\u9f20\u6807\u505c\u4f4f\u540e\u7684\u62d6\u5c3e\u6536\u655b\u901f\u5ea6\uff080 \u4e3a\u9ed8\u8ba4\uff09\u3002",
      label_streamer_profile: "\u9713\u8679 \u65f6\u957f/\u70b9\u6570",
      label_electric_profile: "\u7535\u5f27 \u65f6\u957f/\u70b9\u6570",
      label_meteor_profile: "\u6d41\u661f \u65f6\u957f/\u70b9\u6570",
      label_tubes_profile: "\u7ba1\u9053 \u65f6\u957f/\u70b9\u6570",
      label_line_profile: "\u7ebf\u6761 \u65f6\u957f/\u70b9\u6570",
      label_streamer_params: "\u9713\u8679 \u5149\u666f/\u6838\u5fc3/\u5934\u90e8",
      label_electric_params: "\u7535\u5f27 \u632f\u5e45/\u5206\u53c9",
      label_meteor_params: "\u6d41\u661f \u9891\u7387/\u901f\u5ea6",
      hint_clamp: "\u6570\u503c\u4f1a\u88ab\u5b89\u5168\u533a\u95f4\u8fdb\u884c\u88c1\u526a\u3002",
      style_default: "\u9ed8\u8ba4",
      style_snappy: "\u7d27\u81f4",
      style_long: "\u5ef6\u957f",
      style_cinematic: "\u5267\u60c5",
      style_custom: "\u81ea\u5b9a\u4e49"
    }
  };

  function applyI18n(lang){
    const t = I18N[lang] || I18N["en-US"];
    document.title = t.title;
    document.querySelectorAll('[data-i18n]').forEach(node => {
      const key = node.getAttribute('data-i18n');
      if (key && t[key]) node.textContent = t[key];
    });
    document.querySelectorAll('[data-i18n-title]').forEach(node => {
      const key = node.getAttribute('data-i18n-title');
      if (key && t[key]) node.setAttribute('title', t[key]);
    });
    document.querySelectorAll('[data-i18n-placeholder]').forEach(node => {
      const key = node.getAttribute('data-i18n-placeholder');
      if (key && t[key]) node.setAttribute('placeholder', t[key]);
    });

    const styleMap = {
      "default": t.style_default || "default",
      "snappy": t.style_snappy || "snappy",
      "long": t.style_long || "long",
      "cinematic": t.style_cinematic || "cinematic",
      "custom": t.style_custom || "custom"
    };
    const styleSelect = el('trail_style');
    if (styleSelect) {
      Array.from(styleSelect.options).forEach(opt => {
        const key = opt.value;
        if (styleMap[key]) opt.textContent = styleMap[key];
      });
    }
  }

  function setStatus(msg, tone){
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

  function currentText(){
    return I18N[pickLang()] || I18N["en-US"];
  }

  function statusText(key, fallback){
    const t = currentText();
    return t[key] || fallback;
  }

  function statusError(prefixKey, fallbackPrefix, error){
    const msg = (error && error.message) ? error.message : String(error || '');
    return statusText(prefixKey, fallbackPrefix) + msg;
  }

  function setActionButtonsEnabled(enabled){
    ['btnReload', 'btnReset', 'btnStop', 'btnSave'].forEach((id) => {
      const node = el(id);
      if (node) node.disabled = !enabled;
    });
  }

  function blockActionWhenDisconnected(){
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

  function markConnection(next, force){
    if (!force && connectionState === next) return;
    connectionState = next;
    const t = currentText();
    if (next === 'online') {
      setActionButtonsEnabled(true);
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

  function pickLang(){
    const sel = el('ui_language');
    const val = sel ? sel.value : '';
    if (val) return val;
    const nav = (navigator.language || '').toLowerCase();
    if (nav.startsWith('zh')) return 'zh-CN';
    return 'en-US';
  }

  function showUnauthorized(){
    markConnection('unauthorized');
  }

  async function probeConnection(){
    try{
      const r = await fetch('/api/state', {
        headers: {'X-MFCMouseEffect-Token': token},
        cache: 'no-store'
      });
      if (r.status === 401) {
        markConnection('unauthorized');
        return false;
      }
      if (!r.ok) {
        markConnection('offline');
        return false;
      }
      markConnection('online');
      return true;
    }catch(_e){
      markConnection('offline');
      return false;
    }
  }

  function startHealthCheck(){
    if (healthTimer) return;
    healthTimer = window.setInterval(() => { probeConnection(); }, healthCheckMs);
    document.addEventListener('visibilitychange', () => {
      if (!document.hidden) probeConnection();
    });
  }

  async function apiGet(path){
    const r = await fetch(path, {headers: {'X-MFCMouseEffect-Token': token}});
    if(!r.ok) {
      if (r.status === 401) {
        showUnauthorized();
        const err = new Error('unauthorized');
        err.code = 'unauthorized';
        throw err;
      }
      throw new Error(await r.text());
    }
    return await r.json();
  }
  async function apiPost(path, obj){
    const r = await fetch(path, {
      method:'POST',
      headers:{'Content-Type':'application/json','X-MFCMouseEffect-Token': token},
      body: JSON.stringify(obj || {})
    });
    if(!r.ok) {
      if (r.status === 401) {
        showUnauthorized();
        const err = new Error('unauthorized');
        err.code = 'unauthorized';
        throw err;
      }
      throw new Error(await r.text());
    }
    return await r.json();
  }

  function fillSelect(sel, items, current){
    sel.innerHTML = '';
    for(const it of items || []){
      const o = document.createElement('option');
      o.value = it.value;
      o.textContent = it.label;
      sel.appendChild(o);
    }
    if(current) sel.value = current;
  }

  function num(id, v){ el(id).value = (v ?? '').toString(); }
  function getNum(id){ return Number(el(id).value || 0); }

  function scrollToHash(){
    const h = (location.hash || '').replace('#','');
    if(!h) return;
    const node = document.getElementById(h);
    if(node) node.scrollIntoView({behavior:'smooth', block:'start'});
  }

  async function reload(){
    setStatus(statusText('status_loading', 'Loading...'));
    const schema = await apiGet('/api/schema');
    const st = await apiGet('/api/state');

    applyI18n(st.ui_language || 'en-US');

    fillSelect(el('ui_language'), schema.ui_languages, st.ui_language);
    fillSelect(el('theme'), schema.themes, st.theme);
    fillSelect(el('click'), schema.effects?.click, st.active?.click);
    fillSelect(el('trail'), schema.effects?.trail, st.active?.trail);
    fillSelect(el('scroll'), schema.effects?.scroll, st.active?.scroll);
    fillSelect(el('hold'), schema.effects?.hold, st.active?.hold);
    fillSelect(el('hover'), schema.effects?.hover, st.active?.hover);

    el('text_content').value = st.text_content || '';

    el('trail_style').value = st.trail_style || 'default';
    const p = st.trail_profiles || {};
    num('p_streamer_duration', p.streamer?.duration_ms); num('p_streamer_max', p.streamer?.max_points);
    num('p_electric_duration', p.electric?.duration_ms); num('p_electric_max', p.electric?.max_points);
    num('p_meteor_duration', p.meteor?.duration_ms);     num('p_meteor_max', p.meteor?.max_points);
    num('p_tubes_duration', p.tubes?.duration_ms);       num('p_tubes_max', p.tubes?.max_points);
    num('p_line_duration', p.line?.duration_ms);         num('p_line_max', p.line?.max_points);

    const k = st.trail_params || {};
    num('k_streamer_glow', k.streamer?.glow_width_scale);
    num('k_streamer_core', k.streamer?.core_width_scale);
    num('k_streamer_head', k.streamer?.head_power);
    num('k_electric_amp', k.electric?.amplitude_scale);
    num('k_electric_fork', k.electric?.fork_chance);
    num('k_meteor_rate', k.meteor?.spark_rate_scale);
    num('k_meteor_speed', k.meteor?.spark_speed_scale);
    num('k_idle_fade_start', k.idle_fade_start_ms);
    num('k_idle_fade_end', k.idle_fade_end_ms);

    markConnection('online');
  }

  function buildState(){
    return {
      ui_language: el('ui_language').value,
      theme: el('theme').value,
      active: {
        click: el('click').value,
        trail: el('trail').value,
        scroll: el('scroll').value,
        hold: el('hold').value,
        hover: el('hover').value,
      },
      text_content: el('text_content').value || '',
      trail_style: el('trail_style').value,
      trail_profiles: {
        line:     {duration_ms: getNum('p_line_duration'),     max_points: getNum('p_line_max')},
        streamer: {duration_ms: getNum('p_streamer_duration'), max_points: getNum('p_streamer_max')},
        electric: {duration_ms: getNum('p_electric_duration'), max_points: getNum('p_electric_max')},
        meteor:   {duration_ms: getNum('p_meteor_duration'),   max_points: getNum('p_meteor_max')},
        tubes:    {duration_ms: getNum('p_tubes_duration'),    max_points: getNum('p_tubes_max')},
      },
      trail_params: {
        streamer: {
          glow_width_scale: getNum('k_streamer_glow'),
          core_width_scale: getNum('k_streamer_core'),
          head_power: getNum('k_streamer_head')
        },
        electric: {
          amplitude_scale: getNum('k_electric_amp'),
          fork_chance: getNum('k_electric_fork')
        },
        meteor: {
          spark_rate_scale: getNum('k_meteor_rate'),
          spark_speed_scale: getNum('k_meteor_speed')
        },
        idle_fade_start_ms: getNum('k_idle_fade_start'),
        idle_fade_end_ms: getNum('k_idle_fade_end'),
      }
    };
  }

  el('btnReload').addEventListener('click', async () => {
    try{
      if (blockActionWhenDisconnected()) return;
      setStatus(statusText('status_reloading', 'Reloading config...'));
      await apiPost('/api/reload', {});
      await reload();
      scrollToHash();
    }catch(e){
      if (e && e.code === 'unauthorized') return;
      setStatus(statusError('status_reload_failed', 'Reload failed: ', e), 'warn');
    }
  });

  el('btnSave').addEventListener('click', async () => {
    try{
      if (blockActionWhenDisconnected()) return;
      setStatus(statusText('status_applying', 'Applying...'));
      const st = buildState();
      const res = await apiPost('/api/state', st);
      if (res.ok) {
        setStatus(statusText('status_applied', 'Applied.'), 'ok');
      } else {
        setStatus('Failed: ' + (res.error || ''), 'warn');
      }
    }catch(e){
      if (e && e.code === 'unauthorized') return;
      setStatus(statusError('status_save_failed', 'Save failed: ', e), 'warn');
    }
  });

  el('btnReset').addEventListener('click', async () => {
    try{
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
    }catch(e){
      if (e && e.code === 'unauthorized') return;
      setStatus(statusError('status_reset_failed', 'Reset failed: ', e), 'warn');
    }
  });

  el('btnStop').addEventListener('click', async () => {
    try{
      if (blockActionWhenDisconnected()) return;
      const res = await apiPost('/api/stop', {});
      if (!res.ok) throw new Error(res.error || 'stop failed');
      markConnection('stopped');
    }catch(e){
      if (e && e.code === 'unauthorized') return;
      setStatus(statusError('status_stop_failed', 'Stop failed: ', e), 'warn');
    }
  });

  el('ui_language').addEventListener('change', () => {
    applyI18n(el('ui_language').value);
    if (connectionState !== 'unknown') markConnection(connectionState, true);
  });

  startHealthCheck();
  reload().then(() => {
    scrollToHash();
  }).catch(e => {
    if (e && e.code === 'unauthorized') return;
    markConnection('offline');
  });
})();
