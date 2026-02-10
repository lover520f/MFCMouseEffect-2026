(() => {
  const token = new URL(location.href).searchParams.get('token') || '';
  const diagMode = (new URL(location.href)).searchParams.get('diag') === '1';
  const el = (id) => document.getElementById(id);
  const statusEl = document.getElementById('status');
  const gpuBannerEl = document.getElementById('gpuBanner');
  const gpuBannerTextEl = document.getElementById('gpuBannerText');
  const btnGpuActionEl = document.getElementById('btnGpuAction');
  const btnGpuProbeEl = document.getElementById('btnGpuProbe');
  const diagPanelEl = document.getElementById('diagPanel');
  const diagLogEl = document.getElementById('diagLog');
  const btnDiagSymbolEl = document.getElementById('btnDiagSymbol');
  const btnDiagCopyEl = document.getElementById('btnDiagCopy');
  const btnDiagClearEl = document.getElementById('btnDiagClear');
  const healthCheckMs = 3000;
  const diagPollMs = 500;
  let healthTimer = 0;
  let diagTimer = 0;
  let connectionState = 'unknown';
  let latestState = null;
  let lastStateSyncAtMs = 0;
  const diagMaxLines = 280;
  let diagLines = [];
  let lastDiagSignature = '';

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
      label_render_backend: "Acceleration Mode",
      label_gpu_bridge_mode: "Compatibility Mode",
      label_hold_follow_mode: "Hold Tracking",
      tip_hold_follow_mode: "Choose by feel and CPU budget: Precise (lowest latency, best for aiming/drawing), Smooth (recommended default, balanced response and stability), Performance First (reduced update rate for heavy effects or weaker CPUs).",
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
      btn_gpu_action_probe: "Refresh",
      btn_gpu_action_enable_dawn: "Switch to GPU",
      btn_gpu_action_switch_stable: "Use Stable Mode",
      btn_probe_gpu: "Refresh Acceleration",
      status_gpu_probing: "Refreshing acceleration status...",
      status_gpu_probe_done: "Acceleration status updated.",
      status_gpu_probe_failed: "Refresh failed: ",
      status_gpu_action_running: "Applying optimization suggestion...",
      status_gpu_action_done: "Optimization action completed.",
      status_gpu_action_failed: "Action failed: ",
      status_bridge_switching: "Switching compatibility mode...",
      status_bridge_switched: "Compatibility mode switched.",
      status_bridge_switch_failed: "Compatibility mode switch failed: ",
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
      label_render_backend: "\u52a0\u901f\u6a21\u5f0f",
      label_gpu_bridge_mode: "\u517c\u5bb9\u7b56\u7565",
      label_hold_follow_mode: "\u957f\u6309\u8ddf\u968f\u6a21\u5f0f",
      tip_hold_follow_mode: "\u6309\u4f53\u611f\u548c CPU \u9884\u7b97\u9009\u62e9\uff1a\u7cbe\u51c6\u8ddf\u968f\uff08\u5ef6\u8fdf\u6700\u4f4e\uff0c\u9002\u5408\u6e38\u620f/\u7ed8\u56fe\uff09\uff1b\u5e73\u6ed1\u8ddf\u968f\uff08\u9ed8\u8ba4\u63a8\u8350\uff0c\u8ddf\u624b\u4e0e\u7a33\u5b9a\u66f4\u5747\u8861\uff09\uff1b\u6027\u80fd\u4f18\u5148\uff08\u964d\u4f4e\u66f4\u65b0\u9891\u7387\uff0c\u9002\u5408\u7279\u6548\u8f83\u91cd\u6216 CPU \u7d27\u5f20\u573a\u666f\uff09\u3002",
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
      btn_gpu_action_probe: "\u5237\u65b0\u72b6\u6001",
      btn_gpu_action_enable_dawn: "\u5207\u5230 GPU",
      btn_gpu_action_switch_stable: "\u5207\u5230\u7a33\u5b9a\u6a21\u5f0f",
      btn_probe_gpu: "\u5237\u65b0\u52a0\u901f\u72b6\u6001",
      status_gpu_probing: "\u6b63\u5728\u5237\u65b0\u52a0\u901f\u72b6\u6001...",
      status_gpu_probe_done: "\u52a0\u901f\u72b6\u6001\u5df2\u66f4\u65b0\u3002",
      status_gpu_probe_failed: "\u5237\u65b0\u5931\u8d25\uff1a",
      status_gpu_action_running: "\u6b63\u5728\u6267\u884c\u4f18\u5316\u5efa\u8bae...",
      status_gpu_action_done: "\u4f18\u5316\u64cd\u4f5c\u5df2\u5b8c\u6210\u3002",
      status_gpu_action_failed: "\u64cd\u4f5c\u5931\u8d25\uff1a",
      status_bridge_switching: "\u6b63\u5728\u5207\u6362\u517c\u5bb9\u7b56\u7565...",
      status_bridge_switched: "\u517c\u5bb9\u7b56\u7565\u5df2\u5207\u6362\u3002",
      status_bridge_switch_failed: "\u517c\u5bb9\u7b56\u7565\u5207\u6362\u5931\u8d25\uff1a",
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

  function formatDiagLine(st){
    if (!st) return '';
    const now = new Date();
    const hh = String(now.getHours()).padStart(2, '0');
    const mm = String(now.getMinutes()).padStart(2, '0');
    const ss = String(now.getSeconds()).padStart(2, '0');
    const ms = String(now.getMilliseconds()).padStart(3, '0');
    const ts = `${hh}:${mm}:${ss}.${ms}`;
    const cs = st.gpu_command_stream || {};
    const cc = st.dawn_command_consumer || {};
    const ds = st.dawn_status || {};
    const cmd = `${cs.command_count || 0}/${cs.trail_commands || 0}/${cs.ripple_commands || 0}/${cs.particle_commands || 0}`;
    const queue = `${ds.queue_ready ? 'Q1' : 'Q0'}:${ds.command_encoder_ready ? 'E1' : 'E0'}:${ds.modern_abi_detected ? 'M1' : 'M0'}:${ds.modern_abi_native_ready ? 'N1' : 'N0'}`;
    const strategy = ds.modern_abi_strategy || 'n/a';
    const nativeDetail = ds.modern_abi_native_detail || 'n/a';
    const prime = ds.modern_abi_prime_detail || 'n/a';
    const submit = `${cc.noop_submit_success || 0}/${cc.noop_submit_attempts || 0}`;
    const cmdSubmit = `${cc.empty_command_submit_success || 0}/${cc.empty_command_submit_attempts || 0}`;
    const detail = cc.detail || 'unknown';
    return `${ts} | backend=${st.render_backend_active || 'cpu'} | cmd=${cmd} | ${queue} | strategy=${strategy} | native=${nativeDetail} | prime=${prime} | submit=${submit} | cmdbuf=${cmdSubmit} | ${detail}`;
  }

  function appendDiagLine(st){
    if (!diagMode || !diagLogEl || !diagPanelEl) return;
    const sig = JSON.stringify({
      b: st && st.render_backend_active,
      c: st && st.gpu_command_stream ? st.gpu_command_stream.command_count : 0,
      t: st && st.gpu_command_stream ? st.gpu_command_stream.trail_commands : 0,
      r: st && st.gpu_command_stream ? st.gpu_command_stream.ripple_commands : 0,
      p: st && st.gpu_command_stream ? st.gpu_command_stream.particle_commands : 0,
      q: st && st.dawn_status ? st.dawn_status.queue_ready : false,
      e: st && st.dawn_status ? st.dawn_status.command_encoder_ready : false,
      m: st && st.dawn_status ? st.dawn_status.modern_abi_detected : false,
      n: st && st.dawn_status ? st.dawn_status.modern_abi_native_ready : false,
      y: st && st.dawn_status ? st.dawn_status.modern_abi_strategy : '',
      z: st && st.dawn_status ? st.dawn_status.modern_abi_native_detail : '',
      d: st && st.dawn_command_consumer ? st.dawn_command_consumer.detail : ''
    });
    if (sig === lastDiagSignature) return;
    lastDiagSignature = sig;
    const line = formatDiagLine(st);
    if (!line) return;
    diagLines.push(line);
    if (diagLines.length > diagMaxLines) {
      diagLines.splice(0, diagLines.length - diagMaxLines);
    }
    diagLogEl.value = diagLines.join('\n');
    diagLogEl.scrollTop = diagLogEl.scrollHeight;
  }

  function appendDiagTextLine(line){
    if (!diagMode || !diagLogEl || !diagPanelEl || !line) return;
    const now = new Date();
    const hh = String(now.getHours()).padStart(2, '0');
    const mm = String(now.getMinutes()).padStart(2, '0');
    const ss = String(now.getSeconds()).padStart(2, '0');
    const ms = String(now.getMilliseconds()).padStart(3, '0');
    diagLines.push(`${hh}:${mm}:${ss}.${ms} | ${line}`);
    if (diagLines.length > diagMaxLines) {
      diagLines.splice(0, diagLines.length - diagMaxLines);
    }
    diagLogEl.value = diagLines.join('\n');
    diagLogEl.scrollTop = diagLogEl.scrollHeight;
  }

  function pipelineLabel(mode, lang){
    const zh = {
      cpu_layered: 'CPU \u5206\u5c42\u6e32\u67d3',
      dawn_host_compat_layered: 'GPU \u5408\u6210\u8def\u5f84\uff08\u5bbf\u4e3b\u517c\u5bb9\u6a21\u5f0f\uff09',
      dawn_compositor: 'GPU \u5408\u6210\u8def\u5f84\uff08Compositor \u6a21\u5f0f\uff09'
    };
    const en = {
      cpu_layered: 'CPU layered rendering',
      dawn_host_compat_layered: 'GPU compositor path (host-compatible mode)',
      dawn_compositor: 'GPU compositor path (compositor mode)'
    };
    if (!mode) return '';
    if (lang === 'zh-CN') return zh[mode] || mode;
    return en[mode] || mode;
  }

  function gpuSummaryText(st, banner, stateCode, lang){
    const gpuInUse = !!(st && st.gpu_in_use);
    if (gpuInUse) {
      return (lang === 'zh-CN')
        ? '\u5df2\u542f\u7528 GPU \u540e\u7aef\u3002'
        : 'GPU backend is active.';
    }

    switch (stateCode) {
      case 'loader_missing':
        return (lang === 'zh-CN')
          ? '\u5f53\u524d\u4f7f\u7528 CPU\uff1a\u672a\u627e\u5230 GPU \u8fd0\u884c\u5e93\u3002'
          : 'CPU mode: GPU runtime was not found.';
      case 'handshake_skipped_debugger':
        return (lang === 'zh-CN')
          ? '\u5f53\u524d\u4f7f\u7528 CPU\uff1a\u8c03\u8bd5\u5668\u9644\u52a0\u65f6\u4f1a\u8df3\u8fc7 GPU \u63a2\u6d4b\u3002'
          : 'CPU mode: GPU probing is skipped while a debugger is attached.';
      case 'request_adapter_exception':
      case 'request_adapter_failed':
      case 'request_adapter_timeout':
      case 'request_device_exception':
      case 'request_device_failed':
      case 'request_device_timeout':
        return (lang === 'zh-CN')
          ? '\u5f53\u524d\u4f7f\u7528 CPU\uff1aGPU \u521d\u59cb\u5316\u5931\u8d25\u3002'
          : 'CPU mode: GPU initialization failed.';
      case 'symbols_missing':
      case 'symbols_partial':
        return (lang === 'zh-CN')
          ? '\u5f53\u524d\u4f7f\u7528 CPU\uff1aGPU \u8fd0\u884c\u5e93\u7248\u672c\u4e0e\u5f53\u524d\u7a0b\u5e8f\u4e0d\u517c\u5bb9\u3002'
          : 'CPU mode: GPU runtime version is not compatible with current app.';
      case 'build_disabled':
        return (lang === 'zh-CN')
          ? '\u5f53\u524d\u4f7f\u7528 CPU\uff1a\u5f53\u524d\u7248\u672c\u672a\u542f\u7528 GPU \u6a21\u5757\u3002'
          : 'CPU mode: this build does not include GPU backend.';
      case 'no_display_adapter':
        return (lang === 'zh-CN')
          ? '\u5f53\u524d\u4f7f\u7528 CPU\uff1a\u672a\u68c0\u6d4b\u5230\u53ef\u7528\u663e\u793a\u8bbe\u5907\u3002'
          : 'CPU mode: no usable desktop display adapter was detected.';
      case 'modern_abi_bridge_pending':
      case 'device_ready_cpu_bridge_pending':
      case 'ready_for_device_stage':
        return (lang === 'zh-CN')
          ? '\u5f53\u524d\u4f7f\u7528 CPU\uff1aGPU \u8fd0\u884c\u5e93\u53ef\u7528\uff0c\u4f46\u5c1a\u672a\u5b8c\u6210\u5207\u6362\u3002'
          : 'CPU mode: GPU runtime is available, but backend switch is not completed yet.';
      case 'overlay_bridge_ready_modern_abi_queue_ready':
        return (lang === 'zh-CN')
          ? '\u5f53\u524d\u4f7f\u7528 CPU\uff1aGPU \u8fd0\u884c\u65f6\u4e0e queue \u63e1\u624b\u5df2\u5c31\u7eea\uff0c\u53ef\u5207\u6362\u5230 GPU \u8def\u5f84\u3002'
          : 'CPU mode: GPU runtime and queue handshake are ready; you can switch to GPU path now.';
      case 'queue_not_ready':
        return (lang === 'zh-CN')
          ? '\u5f53\u524d\u4f7f\u7528 CPU\uff1aGPU \u8fd0\u884c\u5e93\u5df2\u5c31\u7eea\uff0c\u4f46\u547d\u4ee4\u961f\u5217\u63e1\u624b\u5c1a\u672a\u5b8c\u6210\u3002'
          : 'CPU mode: GPU runtime is detected, but command queue handshake is not completed yet.';
      default:
        return (lang === 'zh-CN')
          ? ((banner && (banner.text_zh || banner.text_en)) || '\u5f53\u524d\u4e3a CPU \u515c\u5e95\u6a21\u5f0f\u3002')
          : ((banner && (banner.text_en || banner.text_zh)) || 'CPU fallback mode is active.');
    }
  }

  function accelerationLabelText(st, accel, lang){
    const gpuInUse = !!(st && st.gpu_in_use);
    if (!gpuInUse) {
      return (lang === 'zh-CN') ? 'CPU 兜底' : 'CPU fallback';
    }

    const mode = (st && st.render_pipeline_mode) ? st.render_pipeline_mode : '';
    if (mode === 'dawn_compositor' || mode === 'dawn_host_compat_layered') {
      return (lang === 'zh-CN')
        ? 'GPU \u5408\u6210\u5df2\u542f\u7528'
        : 'GPU compositor active';
    }

    if (accel) {
      if (lang === 'zh-CN' && accel.label_zh) return accel.label_zh;
      if (lang !== 'zh-CN' && accel.label_en) return accel.label_en;
      if (accel.label_en) return accel.label_en;
      if (accel.label_zh) return accel.label_zh;
    }
    return (lang === 'zh-CN') ? '部分 GPU' : 'Partial GPU';
  }

  function gpuHintText(stateCode, actionCode, actionTextRaw, lang){
    if (actionCode === 'trigger_probe_now') {
      return '';
    }
    switch (stateCode) {
      case 'loader_missing':
        return (lang === 'zh-CN')
          ? '\u53ef\u628a webgpu_dawn.dll \u653e\u5230 EXE \u540c\u76ee\u5f55\uff0c\u6216\u9879\u76ee Runtime/Dawn \u76ee\u5f55\u3002'
          : 'Put webgpu_dawn.dll next to the exe, or under Runtime/Dawn in this project.';
      case 'handshake_skipped_debugger':
        return (lang === 'zh-CN')
          ? '\u8bf7\u4f7f\u7528 Ctrl+F5 \u6216\u76f4\u63a5\u53cc\u51fb EXE \u8fd0\u884c\u3002'
          : 'Use Ctrl+F5 or run the exe directly.';
      case 'request_adapter_exception':
      case 'request_adapter_failed':
      case 'request_adapter_timeout':
      case 'request_device_exception':
      case 'request_device_failed':
      case 'request_device_timeout':
        return (lang === 'zh-CN')
          ? '\u8bf7\u5148\u66f4\u65b0\u663e\u5361\u9a71\u52a8\uff0c\u6216\u66f4\u6362\u5339\u914d\u7684 Dawn \u8fd0\u884c\u5e93\u7248\u672c\u3002'
          : 'Try updating GPU drivers or using a Dawn runtime version that matches this app.';
      case 'modern_abi_bridge_pending':
      case 'device_ready_cpu_bridge_pending':
        return (lang === 'zh-CN')
          ? '\u53ef\u5c1d\u8bd5\u5207\u6362\u5230 GPU \u6a21\u5f0f\u5e76\u91cd\u65b0\u63a2\u6d4b\u3002'
          : 'Try switching to GPU mode and rechecking.';
      case 'overlay_bridge_ready_modern_abi_queue_ready':
        return (lang === 'zh-CN')
          ? '\u5df2\u53ef\u8fdb\u5165 GPU \u63d0\u4ea4\u9636\u6bb5\uff0c\u5982\u9700\u53ef\u91cd\u65b0\u63a2\u6d4b\u5237\u65b0\u72b6\u6001\u3002'
          : 'Ready for GPU submission stage; re-probe if you want to refresh diagnostics.';
      case 'queue_not_ready':
        return (lang === 'zh-CN')
          ? '\u8fd9\u8868\u793a modern ABI \u4e0b\u7684 queue \u63e1\u624b\u8fd8\u6ca1\u63a5\u901a\uff0c\u5f53\u524d\u4ecd\u7531 CPU \u515c\u5e95\u3002'
          : 'This means modern-ABI queue wiring is not completed yet; CPU fallback remains active.';
      case 'overlay_bridge_ready_modern_abi':
        return (lang === 'zh-CN')
          ? '\u68c0\u6d4b\u5230 modern ABI\uff0clegacy \u56de\u8c03\u9884\u70ed\u5df2\u8df3\u8fc7\uff0c\u5f53\u524d\u4fdd\u6301 CPU \u515c\u5e95\u3002'
          : 'Modern ABI detected; legacy callback priming is skipped, CPU fallback remains active.';
      default:
        if (actionCode === 'switch_bridge_host_compat') {
          return (lang === 'zh-CN')
            ? '\u5982\u679c\u60f3\u8981\u66f4\u7a33\u5b9a\uff0c\u53ef\u5207\u6362\u5230\u7a33\u5b9a\u6a21\u5f0f\u3002'
            : 'Switch to Stable mode if you prefer reliability.';
        }
        return actionTextRaw || '';
    }
  }

  function renderGpuBanner(st){
    if (!gpuBannerEl) return;
    if (!st || !st.gpu_status_banner) {
      gpuBannerEl.className = 'gpu-banner hidden';
      if (gpuBannerTextEl) gpuBannerTextEl.textContent = '';
      if (diagPanelEl) diagPanelEl.classList.toggle('hidden', !diagMode);
      return;
    }
    if (diagPanelEl) diagPanelEl.classList.toggle('hidden', !diagMode);
    const banner = st.gpu_status_banner || {};
    const action = banner.action || {};
    const actionCode = action.action_code || '';
    const lang = pickLang();
    const actionTextRaw = (lang === 'zh-CN') ? (action.action_text_zh || action.action_text_en || '') : (action.action_text_en || action.action_text_zh || '');
    const stateCode = banner.state_code || '';
    const summaryText = gpuSummaryText(st, banner, stateCode, lang);
    const hintText = gpuHintText(stateCode, actionCode, actionTextRaw, lang);
    let finalText = hintText ? `${summaryText} ${hintText}` : summaryText;
    const accel = banner.acceleration || st.gpu_acceleration || {};
    if (accel && accel.level) {
      const accelLabel = accelerationLabelText(st, accel, lang);
      const accelText = (lang === 'zh-CN')
        ? `加速级别: ${accelLabel}`
        : `Acceleration: ${accelLabel}`;
      finalText = `${finalText} ${accelText}`;
    }
    if (st && st.dawn_status && st.gpu_in_use && !st.dawn_status.queue_ready) {
      const queueHint = (lang === 'zh-CN')
        ? 'GPU\u547d\u4ee4\u961f\u5217\u672a\u5c31\u7eea\uff0c\u5f53\u524d\u4f7f\u7528\u964d\u7ea7\u8def\u5f84\u3002'
        : 'GPU command queue is not ready; fallback path is active.';
      finalText = `${finalText} ${queueHint}`;
    }
    if (st && st.render_pipeline_mode) {
      const pipelineLabelText = pipelineLabel(st.render_pipeline_mode, lang);
      const pipelineText = (lang === 'zh-CN')
        ? `渲染路径: ${pipelineLabelText}`
        : `Rendering path: ${pipelineLabelText}`;
      finalText = `${finalText} ${pipelineText}`;
    }
    const bridge = st.dawn_overlay_bridge || {};
    if (bridge && bridge.mode && bridge.mode !== 'none') {
      const modeLabel = (lang === 'zh-CN')
        ? (bridge.mode_label_zh || bridge.mode || '')
        : (bridge.mode_label_en || bridge.mode || '');
      const bridgeMode = (lang === 'zh-CN')
        ? `兼容策略: ${modeLabel}`
        : `Compatibility mode: ${modeLabel}`;
      finalText = `${finalText} ${bridgeMode}`;
    }
    if (bridge && bridge.requested_mode && bridge.mode && bridge.requested_mode !== bridge.mode) {
      const fallbackText = (lang === 'zh-CN')
        ? '\u4e3a\u4fdd\u8bc1\u7a33\u5b9a\u6027\uff0c\u5df2\u81ea\u52a8\u4f7f\u7528\u66f4\u7a33\u5b9a\u7684\u517c\u5bb9\u7b56\u7565\u3002'
        : 'For stability, a safer compatibility mode was selected automatically.';
      finalText = `${finalText} ${fallbackText}`;
    }
    if (bridge && bridge.mode && bridge.mode !== 'none') {
      const compReady = !!bridge.compositor_apis_ready;
      const compText = (lang === 'zh-CN')
        ? (compReady ? '\u9ad8\u7ea7\u5408\u6210\u80fd\u529b: \u53ef\u7528' : '\u9ad8\u7ea7\u5408\u6210\u80fd\u529b: \u672a\u5c31\u7eea')
        : (compReady ? 'Advanced compositor: Ready' : 'Advanced compositor: Not ready');
      finalText = `${finalText} ${compText}`;
    }
    const showDiagCode = diagMode;
    if (showDiagCode && st && st.gpu_command_stream) {
      const cs = st.gpu_command_stream;
      const diagText = (lang === 'zh-CN')
        ? `命令流: ${cs.command_count || 0} (trail ${cs.trail_commands || 0}, ripple ${cs.ripple_commands || 0}, particle ${cs.particle_commands || 0}, tick ${cs.frame_tick_ms || 0})`
        : `Command stream: ${cs.command_count || 0} (trail ${cs.trail_commands || 0}, ripple ${cs.ripple_commands || 0}, particle ${cs.particle_commands || 0}, tick ${cs.frame_tick_ms || 0})`;
      finalText = `${finalText} ${diagText}`;
    }
    if (showDiagCode) {
      const now = Date.now();
      const ageMs = (lastStateSyncAtMs > 0 && now >= lastStateSyncAtMs) ? (now - lastStateSyncAtMs) : 0;
      const refreshText = (lang === 'zh-CN')
        ? `刷新: ${ageMs}ms前`
        : `Refresh: ${ageMs}ms ago`;
      finalText = `${finalText} ${refreshText}`;
    }
    if (showDiagCode && st && st.dawn_status) {
      const ds = st.dawn_status;
      const runtimeReadyText = (lang === 'zh-CN')
        ? `Runtime就绪: queue ${ds.queue_ready ? 'ready' : 'not_ready'}, encoder ${ds.command_encoder_ready ? 'ready' : 'not_ready'}, modernABI ${ds.modern_abi_detected ? 'yes' : 'no'}, native ${ds.modern_abi_native_ready ? 'yes' : 'no'}, nativeDetail ${ds.modern_abi_native_detail || 'n/a'}, strategy ${ds.modern_abi_strategy || 'n/a'}, prime ${ds.modern_abi_prime_detail || 'n/a'}`
        : `Runtime: queue ${ds.queue_ready ? 'ready' : 'not_ready'}, encoder ${ds.command_encoder_ready ? 'ready' : 'not_ready'}, modernABI ${ds.modern_abi_detected ? 'yes' : 'no'}, native ${ds.modern_abi_native_ready ? 'yes' : 'no'}, nativeDetail ${ds.modern_abi_native_detail || 'n/a'}, strategy ${ds.modern_abi_strategy || 'n/a'}, prime ${ds.modern_abi_prime_detail || 'n/a'}`;
      finalText = `${finalText} ${runtimeReadyText}`;
    }
    if (showDiagCode && st && st.dawn_command_consumer) {
      const cc = st.dawn_command_consumer;
      const consumerText = (lang === 'zh-CN')
        ? `消费: ${cc.accepted ? '已接收' : '未接收'} (${cc.detail || 'unknown'})`
        : `Consumer: ${cc.accepted ? 'accepted' : 'rejected'} (${cc.detail || 'unknown'})`;
      const prepText = (lang === 'zh-CN')
        ? `Trail预处理: b${cc.prepared_trail_batches || 0} v${cc.prepared_trail_vertices || 0} s${cc.prepared_trail_segments || 0} t${cc.prepared_trail_triangles || 0} u${cc.prepared_upload_bytes || 0}`
        : `Trail prep: b${cc.prepared_trail_batches || 0} v${cc.prepared_trail_vertices || 0} s${cc.prepared_trail_segments || 0} t${cc.prepared_trail_triangles || 0} u${cc.prepared_upload_bytes || 0}`;
      const ripplePrepText = (lang === 'zh-CN')
        ? `涟漪预处理: b${cc.prepared_ripple_batches || 0} p${cc.prepared_ripple_pulses || 0} t${cc.prepared_ripple_triangles || 0} u${cc.prepared_ripple_upload_bytes || 0}`
        : `Ripple prep: b${cc.prepared_ripple_batches || 0} p${cc.prepared_ripple_pulses || 0} t${cc.prepared_ripple_triangles || 0} u${cc.prepared_ripple_upload_bytes || 0}`;
      const particlePrepText = (lang === 'zh-CN')
        ? `粒子预处理: b${cc.prepared_particle_batches || 0} p${cc.prepared_particle_sprites || 0} u${cc.prepared_particle_upload_bytes || 0}`
        : `Particle prep: b${cc.prepared_particle_batches || 0} p${cc.prepared_particle_sprites || 0} u${cc.prepared_particle_upload_bytes || 0}`;
      const prepModeText = (lang === 'zh-CN')
        ? `预处理并行: ${cc.preprocess_parallel ? '是' : '否'} (w${cc.preprocess_workers || 1})`
        : `Prep parallel: ${cc.preprocess_parallel ? 'yes' : 'no'} (w${cc.preprocess_workers || 1})`;
      const submitText = (lang === 'zh-CN')
        ? `提交: ${cc.noop_submit_success || 0}/${cc.noop_submit_attempts || 0}`
        : `Submit: ${cc.noop_submit_success || 0}/${cc.noop_submit_attempts || 0}`;
      const cmdSubmitText = (lang === 'zh-CN')
        ? `命令缓冲提交: ${cc.empty_command_submit_success || 0}/${cc.empty_command_submit_attempts || 0}`
        : `CmdBuffer submit: ${cc.empty_command_submit_success || 0}/${cc.empty_command_submit_attempts || 0}`;
      finalText = `${finalText} ${consumerText} ${prepText} ${ripplePrepText} ${particlePrepText} ${prepModeText} ${submitText} ${cmdSubmitText}`;
    }
    const prefix = st.gpu_in_use ? '[GPU] ' : '[CPU] ';
    if (gpuBannerTextEl) {
      gpuBannerTextEl.textContent = (showDiagCode && stateCode) ? `${prefix}${finalText} [${stateCode}]` : `${prefix}${finalText}`;
    }
    renderGpuActionButton(actionCode);
    const tone = banner.tone || (st.gpu_in_use ? 'ok' : 'warn');
    gpuBannerEl.className = `gpu-banner ${tone}`;
    appendDiagLine(st);
  }

  function isGpuActionSupported(actionCode){
    return actionCode === 'trigger_probe_now' ||
      actionCode === 'wire_device_stage' ||
      actionCode === 'wire_overlay_gpu_bridge' ||
      actionCode === 'switch_bridge_host_compat' ||
      actionCode === 'enable_dawn_backend' ||
      actionCode === 'run_without_debugger_for_gpu_probe' ||
      actionCode === 'review_logs' ||
      actionCode === 'check_driver_and_backend' ||
      actionCode === 'validate_runtime_abi';
  }

  function gpuActionLabel(actionCode){
    if (actionCode === 'switch_bridge_host_compat') {
      return statusText('btn_gpu_action_switch_stable', 'Use Stable Bridge');
    }
    if (actionCode === 'enable_dawn_backend') {
      return statusText('btn_gpu_action_enable_dawn', 'Enable Dawn');
    }
    return statusText('btn_gpu_action_probe', 'Probe now');
  }

  function renderGpuActionButton(actionCode){
    if (!btnGpuActionEl) return;
    if (!actionCode || !isGpuActionSupported(actionCode)) {
      btnGpuActionEl.classList.add('hidden');
      btnGpuActionEl.dataset.actionCode = '';
      return;
    }
    btnGpuActionEl.dataset.actionCode = actionCode;
    btnGpuActionEl.textContent = gpuActionLabel(actionCode);
    btnGpuActionEl.classList.remove('hidden');
  }

  async function refreshStateFromServer(){
    const st = await apiGet('/api/state');
    latestState = st;
    lastStateSyncAtMs = Date.now();
    renderGpuBanner(st);
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
    ['btnReload', 'btnReset', 'btnStop', 'btnSave', 'btnGpuProbe'].forEach((id) => {
      const node = el(id);
      if (node) node.disabled = !enabled;
    });
  }

  function mergeGpuStatePatch(patch){
    latestState = Object.assign({}, latestState || {}, patch || {});
    return latestState;
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
      renderGpuBanner(latestState);
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
      try {
        const st = await r.json();
        latestState = st;
        lastStateSyncAtMs = Date.now();
        renderGpuBanner(st);
      } catch(_e) {}
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
    if (diagMode && !diagTimer) {
      diagTimer = window.setInterval(async () => {
        if (document.hidden) return;
        try {
          await refreshStateFromServer();
          if (connectionState !== 'online') markConnection('online');
        } catch (e) {
          if (e && e.code === 'unauthorized') return;
        }
      }, diagPollMs);
    }
  }

  async function apiGet(path){
    const r = await fetch(path, {
      headers: {'X-MFCMouseEffect-Token': token},
      cache: 'no-store'
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

  async function probeGpuNow(){
    if (blockActionWhenDisconnected()) return;
    setStatus(statusText('status_gpu_probing', 'Rechecking GPU runtime...'));
    try {
      const res = await apiPost('/api/gpu/probe_now', {refresh: true});
      const merged = mergeGpuStatePatch(res);
      renderGpuBanner(merged);
      setStatus(statusText('status_gpu_probe_done', 'GPU runtime status updated.'), 'ok');
    } catch (e) {
      if (e && e.code === 'unauthorized') return;
      setStatus(statusError('status_gpu_probe_failed', 'GPU probe failed: ', e), 'warn');
    }
  }

  async function runGpuSuggestedAction(){
    if (!btnGpuActionEl) return;
    const actionCode = btnGpuActionEl.dataset.actionCode || '';
    if (!actionCode || !isGpuActionSupported(actionCode)) return;
    if (blockActionWhenDisconnected()) return;

    setStatus(statusText('status_gpu_action_running', 'Executing suggested action...'));
    try {
      if (actionCode === 'switch_bridge_host_compat') {
        const res = await apiPost('/api/gpu/bridge_mode', {mode: 'host_compat'});
        const merged = mergeGpuStatePatch(res);
        if (merged) {
          merged.gpu_bridge_mode_request = 'host_compat';
          renderGpuBanner(merged);
        } else {
          await refreshStateFromServer();
        }
        const bridgeModeSelect = el('gpu_bridge_mode_request');
        if (bridgeModeSelect) bridgeModeSelect.value = 'host_compat';
      } else if (actionCode === 'enable_dawn_backend') {
        await apiPost('/api/state', {render_backend: 'dawn'});
        await refreshStateFromServer();
      } else {
        await probeGpuNow();
      }
      setStatus(statusText('status_gpu_action_done', 'Suggested action completed.'), 'ok');
    } catch (e) {
      if (e && e.code === 'unauthorized') return;
      setStatus(statusError('status_gpu_action_failed', 'Suggested action failed: ', e), 'warn');
    }
  }

  async function switchBridgeMode(mode){
    if (blockActionWhenDisconnected()) return;
    setStatus(statusText('status_bridge_switching', 'Switching GPU bridge mode...'));
    try {
      const res = await apiPost('/api/gpu/bridge_mode', {mode});
      const merged = mergeGpuStatePatch(res);
      if (merged && typeof merged.gpu_bridge_mode_request !== 'string') {
        merged.gpu_bridge_mode_request = mode;
      }
      renderGpuBanner(merged);
      setStatus(statusText('status_bridge_switched', 'GPU bridge mode switched.'), 'ok');
    } catch (e) {
      if (e && e.code === 'unauthorized') return;
      setStatus(statusError('status_bridge_switch_failed', 'GPU bridge mode switch failed: ', e), 'warn');
    }
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
    latestState = st;

    applyI18n(st.ui_language || 'en-US');

    fillSelect(el('ui_language'), schema.ui_languages, st.ui_language);
    fillSelect(el('theme'), schema.themes, st.theme);
    fillSelect(el('render_backend'), schema.render_backends, st.render_backend || 'gdi_cpu');
    fillSelect(el('gpu_bridge_mode_request'), schema.gpu_bridge_modes, st.gpu_bridge_mode_request || 'host_compat');
    fillSelect(el('hold_follow_mode'), schema.hold_follow_modes, st.hold_follow_mode || 'smooth');
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
    renderGpuBanner(st);
  }

  if (diagMode && btnDiagClearEl && diagLogEl) {
    btnDiagClearEl.addEventListener('click', () => {
      diagLines = [];
      lastDiagSignature = '';
      diagLogEl.value = '';
    });
  }

  if (diagMode && btnDiagCopyEl && diagLogEl) {
    btnDiagCopyEl.addEventListener('click', async () => {
      const text = diagLogEl.value || '';
      if (!text) return;
      try {
        if (navigator.clipboard && navigator.clipboard.writeText) {
          await navigator.clipboard.writeText(text);
          setStatus('Diag copied.', 'ok');
          return;
        }
      } catch(_e) {}
      diagLogEl.focus();
      diagLogEl.select();
      try { document.execCommand('copy'); } catch(_e) {}
      setStatus('Diag copied.', 'ok');
    });
  }

  if (diagMode && btnDiagSymbolEl) {
    btnDiagSymbolEl.addEventListener('click', async () => {
      try {
        const res = await apiPost('/api/gpu/runtime_symbol_check', {});
        const s = res && res.dawn_runtime_symbol ? res.dawn_runtime_symbol : null;
        if (!s) {
          appendDiagTextLine('symbol-check: invalid_response');
          return;
        }
        appendDiagTextLine(
          `symbol-check: summary=${s.summary || 'unknown'} mod=${s.module_name || 'n/a'} modernAdapter=${s.has_request_adapter_modern ? 1 : 0} modernDevice=${s.has_request_device_modern ? 1 : 0} legacyAdapter=${s.has_request_adapter_legacy ? 1 : 0} legacyDevice=${s.has_request_device_legacy ? 1 : 0} waitAny=${s.has_wait_any ? 1 : 0} processEvents=${s.has_instance_process_events ? 1 : 0}`
        );
      } catch (e) {
        appendDiagTextLine(`symbol-check: failed ${(e && e.message) ? e.message : String(e || '')}`);
      }
    });
  }

  function buildState(){
    return {
      ui_language: el('ui_language').value,
      theme: el('theme').value,
      render_backend: el('render_backend').value,
      gpu_bridge_mode_request: el('gpu_bridge_mode_request').value,
      hold_follow_mode: el('hold_follow_mode').value,
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
    renderGpuBanner(latestState);
    if (connectionState !== 'unknown') markConnection(connectionState, true);
  });

  if (btnGpuProbeEl) {
    btnGpuProbeEl.addEventListener('click', () => { probeGpuNow(); });
  }
  if (btnGpuActionEl) {
    btnGpuActionEl.addEventListener('click', () => { runGpuSuggestedAction(); });
  }

  const bridgeModeSelect = el('gpu_bridge_mode_request');
  if (bridgeModeSelect) {
    bridgeModeSelect.addEventListener('change', () => {
      switchBridgeMode(bridgeModeSelect.value || 'host_compat');
    });
  }

  startHealthCheck();
  reload().then(() => {
    scrollToHash();
  }).catch(e => {
    if (e && e.code === 'unauthorized') return;
    markConnection('offline');
  });
})();
