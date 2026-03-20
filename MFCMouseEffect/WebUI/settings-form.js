(() => {
  const state = {
    schema: null,
    wasmPendingRender: null,
    wasmRenderRetryTimer: 0,
  };

  function el(id) {
    return document.getElementById(id);
  }

  function generalSection() {
    return window.MfxGeneralSection || null;
  }

  function effectsSection() {
    return window.MfxEffectsSection || null;
  }

  function mouseCompanionSection() {
    return window.MfxMouseCompanionSection || null;
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

  function inputIndicatorModule() {
    return window.MfxSettingsInputIndicator || null;
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

  function clampScalePercent(value) {
    const parsed = Number(value);
    const safe = Number.isFinite(parsed) ? Math.round(parsed) : 100;
    return Math.min(200, Math.max(50, safe));
  }

  function normalizeEffectSizeScalesPayload(value) {
    const source = value || {};
    return {
      click: clampScalePercent(source.click),
      trail: clampScalePercent(source.trail),
      scroll: clampScalePercent(source.scroll),
      hold: clampScalePercent(source.hold),
      hover: clampScalePercent(source.hover),
    };
  }

  function normalizeEffectConflictPolicyPayload(value) {
    const source = value || {};
    return {
      hold_move_policy: (source.hold_move_policy || source.hold_move || 'hold_only').toString(),
    };
  }

  function normalizeEffectsBlacklistAppsPayload(value) {
    const source = Array.isArray(value) ? value : [];
    const out = [];
    for (const item of source) {
      const normalized = `${item || ''}`.trim().toLowerCase();
      if (!normalized || out.includes(normalized)) {
        continue;
      }
      out.push(normalized);
    }
    return out;
  }

  function normalizeMouseCompanionPayload(value) {
    const source = value || {};
    const parseIntSafe = (input, fallback) => {
      const parsed = Number.parseInt(`${input ?? ''}`, 10);
      return Number.isFinite(parsed) ? parsed : fallback;
    };
    const parseFloatSafe = (input, fallback) => {
      const parsed = Number.parseFloat(`${input ?? ''}`);
      return Number.isFinite(parsed) ? parsed : fallback;
    };
    const clamp = (input, min, max, fallback) => {
      const valueInt = parseIntSafe(input, fallback);
      return Math.min(max, Math.max(min, valueInt));
    };
    const clampFloat = (input, min, max, fallback) => {
      const valueFloat = parseFloatSafe(input, fallback);
      return Math.min(max, Math.max(min, valueFloat));
    };
    const modelPath = `${source.model_path || ''}`.trim();
    const actionLibraryPath = `${source.action_library_path || ''}`.trim();
    const appearanceProfilePath = `${source.appearance_profile_path || ''}`.trim();
    const positionMode = `${source.position_mode || ''}`.trim().toLowerCase();
    const edgeClampMode = `${source.edge_clamp_mode || ''}`.trim().toLowerCase();
    const headTintPerClick = clampFloat(source.head_tint_per_click, 0.01, 1.0, 0.11);
    const testHeadTintPerClick = clampFloat(source.test_head_tint_per_click, 0.01, 1.0, 0.2);
    return {
      enabled: !!source.enabled,
      model_path: modelPath || 'MFCMouseEffect/Assets/Pet3D/source/pet-main.glb',
      action_library_path: actionLibraryPath || 'MFCMouseEffect/Assets/Pet3D/source/pet-actions.json',
      appearance_profile_path: appearanceProfilePath || 'MFCMouseEffect/Assets/Pet3D/source/pet-appearance.json',
      position_mode:
        positionMode === 'relative' ||
        positionMode === 'absolute' ||
        positionMode === 'follow' ||
        positionMode === 'fixed_bottom_left'
          ? positionMode
          : 'fixed_bottom_left',
      edge_clamp_mode:
        edgeClampMode === 'strict' || edgeClampMode === 'soft' || edgeClampMode === 'free'
          ? edgeClampMode
          : 'soft',
      size_px: clamp(source.size_px, 48, 360, 112),
      offset_x: clamp(source.offset_x, -1200, 1200, 18),
      offset_y: clamp(source.offset_y, -1200, 1200, 26),
      absolute_x: clamp(source.absolute_x, -20000, 20000, 40),
      absolute_y: clamp(source.absolute_y, -20000, 20000, 40),
      target_monitor: `${source.target_monitor || ''}`.trim().toLowerCase() || 'cursor',
      press_lift_px: clamp(source.press_lift_px, 0, 240, 24),
      smoothing_percent: clamp(source.smoothing_percent, 0, 95, 68),
      follow_threshold_px: clamp(source.follow_threshold_px, 0, 32, 2),
      release_hold_ms: clamp(source.release_hold_ms, 0, 800, 120),
      face_pointer_enabled: !!source.face_pointer_enabled,
      click_streak_break_ms: clamp(source.click_streak_break_ms, 120, 3000, 650),
      head_tint_per_click: headTintPerClick,
      head_tint_max: clampFloat(source.head_tint_max, headTintPerClick, 1.0, 0.7),
      head_tint_decay_per_second: clampFloat(source.head_tint_decay_per_second, 0.05, 4.0, 0.36),
      use_test_profile: !!source.use_test_profile,
      test_press_lift_px: clamp(source.test_press_lift_px, 0, 320, 48),
      test_smoothing_percent: clamp(source.test_smoothing_percent, 0, 95, 32),
      test_click_streak_break_ms: clamp(source.test_click_streak_break_ms, 120, 3000, 1200),
      test_head_tint_per_click: testHeadTintPerClick,
      test_head_tint_max: clampFloat(source.test_head_tint_max, testHeadTintPerClick, 1.0, 0.8),
      test_head_tint_decay_per_second: clampFloat(source.test_head_tint_decay_per_second, 0.05, 4.0, 0.15),
    };
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
    setChecked('launch_at_startup', !!appState.launch_at_startup);
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
    const companion = mouseCompanionSection();
    if (companion && typeof companion.render === 'function') {
      companion.render({
        schema: schema,
        state: appState,
      });
    }
    renderEffects(schema, appState);
    renderText(appState);
    renderTrail(appState);
    const indicator = inputIndicatorModule();
    if (indicator && typeof indicator.renderInputIndicator === 'function') {
      indicator.renderInputIndicator(schema, appState, texts, wasmAction, {
        fillSelect,
        setChecked,
        setNum,
      });
    }
    renderWasm(schema, appState, texts, wasmAction, wasmStatus);
    if (indicator && typeof indicator.bindIndicatorEvents === 'function') {
      indicator.bindIndicatorEvents();
    }
  }

  function read() {
    const schema = state.schema || {};
    const general = generalSection();
    const effects = effectsSection();
    const text = textSection();
    const trail = trailSection();
    const companion = mouseCompanionSection();
    const generalState = (general && typeof general.read === 'function')
      ? general.read()
      : {
        ui_language: getText('ui_language'),
        theme: getText('theme'),
        theme_catalog_root_path: getText('theme_catalog_root_path'),
        overlay_target_fps: getNum('overlay_target_fps'),
        launch_at_startup: getChecked('launch_at_startup'),
        hold_follow_mode: getText('hold_follow_mode'),
        hold_presenter_backend: getText('hold_presenter_backend') || 'auto',
      };

    const effectsState = (effects && typeof effects.read === 'function')
      ? effects.read()
      : {
        active: {
          click: getText('click'),
          trail: getText('trail'),
          scroll: getText('scroll'),
          hold: getText('hold'),
          hover: getText('hover'),
        },
        effect_size_scales: normalizeEffectSizeScalesPayload({}),
        effect_conflict_policy: normalizeEffectConflictPolicyPayload({}),
      };
    const activeEffectsState = effectsState?.active || effectsState || {};
    const effectSizeScales = normalizeEffectSizeScalesPayload(effectsState?.effect_size_scales || {});
    const effectConflictPolicy = normalizeEffectConflictPolicyPayload(
      effectsState?.effect_conflict_policy || {},
    );
    const effectsBlacklistApps = normalizeEffectsBlacklistAppsPayload(
      effectsState?.effects_blacklist_apps || [],
    );

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

    const indicator = inputIndicatorModule();
    const indicatorState = (indicator && typeof indicator.readInputIndicatorState === 'function')
      ? indicator.readInputIndicatorState(schema, {
        getChecked,
        getNum,
        getText,
      })
      : {
        enabled: getChecked('ii_enabled'),
        keyboard_enabled: getChecked('ii_keyboard_enabled'),
        render_mode: getText('ii_render_mode') || 'native',
        wasm_fallback_to_native: getChecked('ii_wasm_fallback'),
        wasm_manifest_path: '',
        position_mode: getText('ii_position_mode') || 'relative',
        offset_x: getNum('ii_offset_x'),
        offset_y: getNum('ii_offset_y'),
        absolute_x: getNum('ii_absolute_x'),
        absolute_y: getNum('ii_absolute_y'),
        target_monitor: getText('ii_target_monitor') || 'cursor',
        key_display_mode: getText('ii_key_display_mode') || 'all',
        key_label_layout_mode: getText('ii_key_label_layout_mode') || 'fixed_font',
        per_monitor_overrides: {},
        size_px: getNum('ii_size_px'),
        duration_ms: getNum('ii_duration_ms'),
      };
    const mouseCompanionState = (companion && typeof companion.read === 'function')
      ? companion.read()
      : {
        enabled: getChecked('mc_enabled'),
        model_path: getText('mc_model_path'),
        action_library_path: getText('mc_action_library_path'),
        appearance_profile_path: getText('mc_appearance_profile_path'),
        position_mode: getText('mc_position_mode') || 'fixed_bottom_left',
        edge_clamp_mode: getText('mc_edge_clamp_mode') || 'soft',
        size_px: getNum('mc_size_px'),
        offset_x: getNum('mc_offset_x'),
        offset_y: getNum('mc_offset_y'),
        absolute_x: getNum('mc_absolute_x'),
        absolute_y: getNum('mc_absolute_y'),
        target_monitor: getText('mc_target_monitor') || 'cursor',
        press_lift_px: getNum('mc_press_lift_px'),
        smoothing_percent: getNum('mc_smoothing_percent'),
        follow_threshold_px: getNum('mc_follow_threshold_px'),
        release_hold_ms: getNum('mc_release_hold_ms'),
        face_pointer_enabled: getChecked('mc_face_pointer_enabled'),
        click_streak_break_ms: getNum('mc_click_streak_break_ms'),
        head_tint_per_click: getNum('mc_head_tint_per_click'),
        head_tint_max: getNum('mc_head_tint_max'),
        head_tint_decay_per_second: getNum('mc_head_tint_decay_per_second'),
        use_test_profile: getChecked('mc_use_test_profile'),
        test_press_lift_px: getNum('mc_test_press_lift_px'),
        test_smoothing_percent: getNum('mc_test_smoothing_percent'),
        test_click_streak_break_ms: getNum('mc_test_click_streak_break_ms'),
        test_head_tint_per_click: getNum('mc_test_head_tint_per_click'),
        test_head_tint_max: getNum('mc_test_head_tint_max'),
        test_head_tint_decay_per_second: getNum('mc_test_head_tint_decay_per_second'),
      };

    return {
      ui_language: generalState.ui_language,
      theme: generalState.theme,
      theme_catalog_root_path: generalState.theme_catalog_root_path || '',
      overlay_target_fps: Number(generalState.overlay_target_fps ?? 0),
      launch_at_startup: !!generalState.launch_at_startup,
      hold_follow_mode: generalState.hold_follow_mode,
      hold_presenter_backend: generalState.hold_presenter_backend || 'auto',
      active: {
        click: activeEffectsState.click || '',
        trail: activeEffectsState.trail || '',
        scroll: activeEffectsState.scroll || '',
        hold: activeEffectsState.hold || '',
        hover: activeEffectsState.hover || '',
      },
      effect_size_scales: effectSizeScales,
      effect_conflict_policy: effectConflictPolicy,
      effects_blacklist_apps: effectsBlacklistApps,
      text_content: textState.text_content,
      text_font_size: textState.text_font_size,
      trail_style: trailState.trail_style,
      trail_profiles: trailState.trail_profiles,
      trail_params: trailState.trail_params,
      mouse_companion: normalizeMouseCompanionPayload(mouseCompanionState),
      input_indicator: indicatorState,
    };
  }

  window.MfxSettingsForm = {
    render,
    read,
    syncIndicatorPositionUi: () => {
      const indicator = inputIndicatorModule();
      if (indicator && typeof indicator.syncIndicatorPositionUi === 'function') {
        indicator.syncIndicatorPositionUi();
      }
    },
  };
})();
