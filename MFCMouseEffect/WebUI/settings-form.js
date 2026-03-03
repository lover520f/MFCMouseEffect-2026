(() => {
  const state = {
    schema: null,
    indicatorBound: false,
    wasmPendingRender: null,
    wasmRenderRetryTimer: 0,
  };

  function el(id) {
    return document.getElementById(id);
  }

  function inputIndicatorSection() {
    return window.MfxInputIndicatorSection || null;
  }

  function generalSection() {
    return window.MfxGeneralSection || null;
  }

  function effectsSection() {
    return window.MfxEffectsSection || null;
  }

  function textSection() {
    return window.MfxTextSection || null;
  }

  function trailSection() {
    return window.MfxTrailSection || null;
  }

  function wasmSection() {
    return window.MfxWasmSection || null;
  }

  function clearWasmRetryTimer() {
    if (!state.wasmRenderRetryTimer) {
      return;
    }
    window.clearTimeout(state.wasmRenderRetryTimer);
    state.wasmRenderRetryTimer = 0;
  }

  function scheduleWasmRenderRetry() {
    if (state.wasmRenderRetryTimer) {
      return;
    }
    state.wasmRenderRetryTimer = window.setTimeout(() => {
      state.wasmRenderRetryTimer = 0;
      const pending = state.wasmPendingRender;
      if (!pending) {
        return;
      }
      renderWasm(
        pending.schema,
        pending.appState,
        pending.texts,
        pending.wasmAction,
        pending.wasmStatus);
    }, 120);
  }

  function fillSelectByNode(selectNode, items, current) {
    if (!selectNode) {
      return;
    }
    selectNode.innerHTML = '';
    for (const item of items || []) {
      const option = document.createElement('option');
      option.value = item.value;
      option.textContent = item.label;
      selectNode.appendChild(option);
    }
    if (current) {
      selectNode.value = current;
    }
  }

  function fillSelect(id, items, current) {
    fillSelectByNode(el(id), items, current);
  }

  function effectCapabilityEnabled(schema, key) {
    return schema?.capabilities?.effects?.[key] !== false;
  }

  function applyEffectSelectCapability(id, enabled) {
    const node = el(id);
    if (!node) {
      return;
    }
    node.disabled = !enabled;
  }

  function setText(id, value) {
    const node = el(id);
    if (node) {
      node.value = value || '';
    }
  }

  function setNum(id, value) {
    const node = el(id);
    if (node) {
      node.value = (value ?? '').toString();
    }
  }

  function setChecked(id, value) {
    const node = el(id);
    if (node) {
      node.checked = !!value;
    }
  }

  function getText(id) {
    const node = el(id);
    return node ? (node.value || '') : '';
  }

  function getNum(id) {
    const node = el(id);
    return Number(node ? (node.value || 0) : 0);
  }

  function getChecked(id) {
    const node = el(id);
    return !!(node && node.checked);
  }

  function syncIndicatorPositionUi() {
    const section = inputIndicatorSection();
    if (section && typeof section.syncIndicatorPositionUi === 'function') {
      section.syncIndicatorPositionUi();
      return;
    }

    const mode = getText('ii_position_mode') || 'relative';
    const relativeRow = el('ii_offset_x')?.closest('.pair');
    const absoluteRow = el('ii_absolute_x')?.closest('.pair');
    if (relativeRow) {
      relativeRow.style.opacity = (mode === 'relative') ? '1' : '0.45';
    }
    if (absoluteRow) {
      absoluteRow.style.opacity = (mode === 'absolute') ? '1' : '0.45';
    }

    const targetMon = getText('ii_target_monitor') || 'cursor';
    const perMonitorContainer = el('ii_per_monitor_overrides');
    if (perMonitorContainer) {
      perMonitorContainer.style.display = (mode === 'absolute' && targetMon === 'custom') ? 'block' : 'none';
    }
  }

  function bindIndicatorEvents() {
    if (state.indicatorBound) {
      return;
    }
    const section = inputIndicatorSection();
    if (section) {
      state.indicatorBound = true;
      return;
    }
    state.indicatorBound = true;

    const positionMode = el('ii_position_mode');
    if (positionMode) {
      positionMode.addEventListener('change', syncIndicatorPositionUi);
    }
    const targetMonitor = el('ii_target_monitor');
    if (targetMonitor) {
      targetMonitor.addEventListener('change', syncIndicatorPositionUi);
    }
  }

  function buildPerMonitorUI(monitors, overrides, texts) {
    const container = el('ii_per_monitor_overrides');
    if (!container) {
      return;
    }
    container.innerHTML = '';

    if (!Array.isArray(monitors) || monitors.length === 0) {
      const tip = document.createElement('div');
      tip.className = 'pm-empty';
      tip.textContent = texts.pm_no_monitors || 'No monitors detected.';
      container.appendChild(tip);
      return;
    }

    const monLabel = texts.pm_monitor || 'Monitor';
    const primaryBadge = texts.pm_primary_badge || 'Primary';
    for (let idx = 0; idx < monitors.length; idx += 1) {
      const monitor = monitors[idx];
      const monitorId = monitor.id;
      const width = (monitor.right || 0) - (monitor.left || 0);
      const height = (monitor.bottom || 0) - (monitor.top || 0);
      const resolution = (width > 0 && height > 0) ? ` (${width}x${height})` : '';
      const badge = monitor.is_primary ? ` * ${primaryBadge}` : '';
      const labelText = `${monLabel} ${idx + 1}${resolution}${badge}`;

      const override = overrides ? overrides[monitorId] : null;
      const enabled = !!(override && override.enabled);
      const valueX = (override && override.absolute_x !== undefined) ? override.absolute_x : 40;
      const valueY = (override && override.absolute_y !== undefined) ? override.absolute_y : 40;

      const row = document.createElement('div');
      row.className = 'monitor-row';
      if (!enabled) {
        row.classList.add('is-disabled');
      }

      const checkbox = document.createElement('input');
      checkbox.type = 'checkbox';
      checkbox.id = `ii_ov_${monitorId}_en`;
      checkbox.className = 'monitor-toggle';
      checkbox.checked = enabled;

      const label = document.createElement('label');
      label.htmlFor = checkbox.id;
      label.className = 'monitor-name';
      label.title = labelText;
      label.textContent = labelText;

      const inputX = document.createElement('input');
      inputX.type = 'number';
      inputX.id = `ii_ov_${monitorId}_x`;
      inputX.className = 'monitor-input';
      inputX.value = valueX;
      inputX.disabled = !enabled;
      inputX.title = 'X';
      inputX.placeholder = 'X';

      const inputY = document.createElement('input');
      inputY.type = 'number';
      inputY.id = `ii_ov_${monitorId}_y`;
      inputY.className = 'monitor-input';
      inputY.value = valueY;
      inputY.disabled = !enabled;
      inputY.title = 'Y';
      inputY.placeholder = 'Y';

      checkbox.addEventListener('change', () => {
        const checked = checkbox.checked;
        inputX.disabled = !checked;
        inputY.disabled = !checked;
        row.classList.toggle('is-disabled', !checked);
      });

      row.appendChild(checkbox);
      row.appendChild(label);
      row.appendChild(inputX);
      row.appendChild(inputY);
      container.appendChild(row);
    }
  }

  function readPerMonitorUI(monitors) {
    const result = {};
    if (!Array.isArray(monitors)) {
      return result;
    }
    for (const monitor of monitors) {
      const monitorId = monitor.id;
      const checkbox = el(`ii_ov_${monitorId}_en`);
      const inputX = el(`ii_ov_${monitorId}_x`);
      const inputY = el(`ii_ov_${monitorId}_y`);
      if (!checkbox || !inputX || !inputY) {
        continue;
      }
      result[monitorId] = {
        enabled: checkbox.checked,
        absolute_x: Number(inputX.value),
        absolute_y: Number(inputY.value),
      };
    }
    return result;
  }

  function renderGeneral(schema, appState, generalAction) {
    const section = generalSection();
    if (section && typeof section.render === 'function') {
      if (typeof section.onAction === 'function') {
        section.onAction(generalAction);
      }
      section.render({ schema, state: appState, onAction: generalAction });
      return;
    }

    fillSelect('ui_language', schema.ui_languages, appState.ui_language);
    fillSelect('theme', schema.themes, appState.theme);
    fillSelect('hold_follow_mode', schema.hold_follow_modes, appState.hold_follow_mode || 'smooth');
    fillSelect(
      'hold_presenter_backend',
      schema.hold_presenter_backends,
      appState.hold_presenter_backend || 'auto');
  }

  function renderEffects(schema, appState) {
    const section = effectsSection();
    if (section && typeof section.render === 'function') {
      section.render({ schema, state: appState });
      return;
    }

    fillSelect('click', schema.effects?.click, appState.active?.click);
    fillSelect('trail', schema.effects?.trail, appState.active?.trail);
    fillSelect('scroll', schema.effects?.scroll, appState.active?.scroll);
    fillSelect('hold', schema.effects?.hold, appState.active?.hold);
    fillSelect('hover', schema.effects?.hover, appState.active?.hover);
    applyEffectSelectCapability('click', effectCapabilityEnabled(schema, 'click'));
    applyEffectSelectCapability('trail', effectCapabilityEnabled(schema, 'trail'));
    applyEffectSelectCapability('scroll', effectCapabilityEnabled(schema, 'scroll'));
    applyEffectSelectCapability('hold', effectCapabilityEnabled(schema, 'hold'));
    applyEffectSelectCapability('hover', effectCapabilityEnabled(schema, 'hover'));
  }

  function renderText(appState) {
    const section = textSection();
    if (section && typeof section.render === 'function') {
      section.render({ state: appState });
      return;
    }

    setText('text_content', appState.text_content || '');
    setNum('text_font_size', appState.text_font_size);
  }

  function renderTrail(appState) {
    const section = trailSection();
    if (section && typeof section.render === 'function') {
      section.render({ state: appState });
      return;
    }

    setText('trail_style', appState.trail_style || 'default');
    const profile = appState.trail_profiles || {};
    setNum('p_streamer_duration', profile.streamer?.duration_ms);
    setNum('p_streamer_max', profile.streamer?.max_points);
    setNum('p_electric_duration', profile.electric?.duration_ms);
    setNum('p_electric_max', profile.electric?.max_points);
    setNum('p_meteor_duration', profile.meteor?.duration_ms);
    setNum('p_meteor_max', profile.meteor?.max_points);
    setNum('p_tubes_duration', profile.tubes?.duration_ms);
    setNum('p_tubes_max', profile.tubes?.max_points);
    setNum('p_line_duration', profile.line?.duration_ms);
    setNum('p_line_max', profile.line?.max_points);

    const params = appState.trail_params || {};
    setNum('k_streamer_glow', params.streamer?.glow_width_scale);
    setNum('k_streamer_core', params.streamer?.core_width_scale);
    setNum('k_streamer_head', params.streamer?.head_power);
    setNum('k_electric_amp', params.electric?.amplitude_scale);
    setNum('k_electric_fork', params.electric?.fork_chance);
    setNum('k_meteor_rate', params.meteor?.spark_rate_scale);
    setNum('k_meteor_speed', params.meteor?.spark_speed_scale);
    setNum('k_idle_fade_start', params.idle_fade_start_ms);
    setNum('k_idle_fade_end', params.idle_fade_end_ms);
  }

  function renderInputIndicator(schema, appState, texts) {
    const indicator = appState.input_indicator || appState.mouse_indicator || {};
    const section = inputIndicatorSection();
    if (section && typeof section.render === 'function') {
      section.render({
        schema,
        indicator,
        texts,
      });
      return;
    }

    fillSelect(
      'ii_position_mode',
      schema.input_indicator_position_modes,
      indicator.position_mode || 'relative');
    fillSelect('ii_target_monitor', schema.target_monitor_options, indicator.target_monitor || 'cursor');
    fillSelect('ii_key_display_mode', schema.key_display_modes, indicator.key_display_mode || 'all');
    fillSelect(
      'ii_key_label_layout_mode',
      schema.key_label_layout_modes,
      indicator.key_label_layout_mode || 'fixed_font');

    buildPerMonitorUI(schema.monitors, indicator.per_monitor_overrides, texts);

    setChecked('ii_enabled', indicator.enabled !== false);
    setChecked('ii_keyboard_enabled', indicator.keyboard_enabled !== false);
    setNum('ii_offset_x', indicator.offset_x);
    setNum('ii_offset_y', indicator.offset_y);
    setNum('ii_absolute_x', indicator.absolute_x);
    setNum('ii_absolute_y', indicator.absolute_y);
    setNum('ii_size_px', indicator.size_px);
    setNum('ii_duration_ms', indicator.duration_ms);

    syncIndicatorPositionUi();
  }

  function renderWasm(schema, appState, texts, wasmAction, wasmStatus) {
    const section = wasmSection();
    if (!section || typeof section.render !== 'function') {
      state.wasmPendingRender = {
        schema,
        appState,
        texts,
        wasmAction,
        wasmStatus,
      };
      scheduleWasmRenderRetry();
      return;
    }
    clearWasmRetryTimer();
    state.wasmPendingRender = null;
    section.render({
      schema: schema?.wasm || {},
      state: appState?.wasm || {},
      i18n: texts,
      onAction: wasmAction,
      onStatus: wasmStatus,
    });
  }

  function render(payload) {
    const schema = payload?.schema || {};
    const appState = payload?.state || {};
    const texts = payload?.i18n || {};
    const generalAction = (typeof payload?.generalAction === 'function')
      ? payload.generalAction
      : null;
    const wasmAction = (typeof payload?.wasmAction === 'function')
      ? payload.wasmAction
      : null;
    const wasmStatus = (typeof payload?.wasmStatus === 'function')
      ? payload.wasmStatus
      : null;

    state.schema = schema;
    renderGeneral(schema, appState, generalAction);
    renderEffects(schema, appState);
    renderText(appState);
    renderTrail(appState);
    renderInputIndicator(schema, appState, texts);
    renderWasm(schema, appState, texts, wasmAction, wasmStatus);
    bindIndicatorEvents();
  }

  function read() {
    const schema = state.schema || {};
    const general = generalSection();
    const effects = effectsSection();
    const text = textSection();
    const trail = trailSection();
    const section = inputIndicatorSection();

    const generalState = (general && typeof general.read === 'function')
      ? general.read()
      : {
        ui_language: getText('ui_language'),
        theme: getText('theme'),
        theme_catalog_root_path: getText('theme_catalog_root_path'),
        overlay_target_fps: getNum('overlay_target_fps'),
        hold_follow_mode: getText('hold_follow_mode'),
        hold_presenter_backend: getText('hold_presenter_backend') || 'auto',
      };

    const effectsState = (effects && typeof effects.read === 'function')
      ? effects.read()
      : {
        click: getText('click'),
        trail: getText('trail'),
        scroll: getText('scroll'),
        hold: getText('hold'),
        hover: getText('hover'),
      };

    const textState = (text && typeof text.read === 'function')
      ? text.read()
      : {
        text_content: getText('text_content'),
        text_font_size: getNum('text_font_size'),
      };

    const trailState = (trail && typeof trail.read === 'function')
      ? trail.read()
      : {
        trail_style: getText('trail_style'),
        trail_profiles: {
          line: { duration_ms: getNum('p_line_duration'), max_points: getNum('p_line_max') },
          streamer: { duration_ms: getNum('p_streamer_duration'), max_points: getNum('p_streamer_max') },
          electric: { duration_ms: getNum('p_electric_duration'), max_points: getNum('p_electric_max') },
          meteor: { duration_ms: getNum('p_meteor_duration'), max_points: getNum('p_meteor_max') },
          tubes: { duration_ms: getNum('p_tubes_duration'), max_points: getNum('p_tubes_max') },
        },
        trail_params: {
          streamer: {
            glow_width_scale: getNum('k_streamer_glow'),
            core_width_scale: getNum('k_streamer_core'),
            head_power: getNum('k_streamer_head'),
          },
          electric: {
            amplitude_scale: getNum('k_electric_amp'),
            fork_chance: getNum('k_electric_fork'),
          },
          meteor: {
            spark_rate_scale: getNum('k_meteor_rate'),
            spark_speed_scale: getNum('k_meteor_speed'),
          },
          idle_fade_start_ms: getNum('k_idle_fade_start'),
          idle_fade_end_ms: getNum('k_idle_fade_end'),
        },
      };

    const indicatorState = (section && typeof section.read === 'function')
      ? section.read()
      : {
        enabled: getChecked('ii_enabled'),
        keyboard_enabled: getChecked('ii_keyboard_enabled'),
        position_mode: getText('ii_position_mode') || 'relative',
        offset_x: getNum('ii_offset_x'),
        offset_y: getNum('ii_offset_y'),
        absolute_x: getNum('ii_absolute_x'),
        absolute_y: getNum('ii_absolute_y'),
        target_monitor: getText('ii_target_monitor') || 'cursor',
        key_display_mode: getText('ii_key_display_mode') || 'all',
        key_label_layout_mode: getText('ii_key_label_layout_mode') || 'fixed_font',
        per_monitor_overrides: readPerMonitorUI(schema.monitors),
        size_px: getNum('ii_size_px'),
        duration_ms: getNum('ii_duration_ms'),
      };

    return {
      ui_language: generalState.ui_language,
      theme: generalState.theme,
      theme_catalog_root_path: generalState.theme_catalog_root_path || '',
      overlay_target_fps: Number(generalState.overlay_target_fps ?? 0),
      hold_follow_mode: generalState.hold_follow_mode,
      hold_presenter_backend: generalState.hold_presenter_backend || 'auto',
      active: effectsState,
      text_content: textState.text_content,
      text_font_size: textState.text_font_size,
      trail_style: trailState.trail_style,
      trail_profiles: trailState.trail_profiles,
      trail_params: trailState.trail_params,
      input_indicator: indicatorState,
    };
  }

  window.MfxSettingsForm = {
    render,
    read,
    syncIndicatorPositionUi,
  };
})();
