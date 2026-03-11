<script>
  import MappingPanel from './MappingPanel.svelte';
  import {
    DEFAULT_GESTURE_MAX_DIRECTIONS,
    DEFAULT_GESTURE_MATCH_THRESHOLD_PERCENT,
    DEFAULT_GESTURE_MIN_DISTANCE,
    DEFAULT_GESTURE_PATTERN_MODE,
    DEFAULT_GESTURE_SAMPLE_STEP,
    DEFAULT_GESTURE_TRIGGER_BUTTON,
  } from './defaults.js';
  import {
    evaluateRows,
    listTemplateOptions,
    normalizeAutomationPayload,
    parseAppScopes,
    readMappings,
    readTemplateBindings,
    sanitizeOptionValue,
    serializeAppScopes,
    serializeLegacyAppScope,
    textOf,
    upsertRowsByTrigger,
  } from './model.js';
  import { normalizeTriggerChain, serializeTriggerChain } from './trigger-chain.js';

  export let schema = {};
  export let payloadState = {};
  export let i18n = {};

  let mouseOptions = [];
  let appScopeOptions = [];
  let gestureOptions = [];
  let gestureButtonOptions = [];

  let defaultMouseTrigger = '';
  let defaultGestureTrigger = '';
  let defaultGestureButton = DEFAULT_GESTURE_TRIGGER_BUTTON;

  let automationEnabled = false;
  let gestureEnabled = false;
  let gestureMinDistance = DEFAULT_GESTURE_MIN_DISTANCE;
  let gestureSampleStep = DEFAULT_GESTURE_SAMPLE_STEP;
  let gestureMaxDirections = DEFAULT_GESTURE_MAX_DIRECTIONS;

  let mouseRows = [];
  let gestureRows = [];
  let rowSeq = 1;

  let mouseValidation = { hasMissingShortcut: false, hasDuplicateTrigger: false, hasInvalidScope: false };
  let gestureValidation = { hasMissingShortcut: false, hasDuplicateTrigger: false, hasInvalidScope: false };

  let mouseTemplate = '';
  let gestureTemplate = '';
  let mouseTemplateOptions = [];
  let gestureTemplateOptions = [];

  const AUTOMATION_DRAFT_VERSION = 1;
  const AUTOMATION_DRAFT_STORAGE_KEY = 'mfx.automation.editor.draft.v1';

  let lastIncomingSignature = '';
  let lastI18nRef = null;
  let skipDraftPersist = false;
  let uiText = {};
  let mousePanelTexts = {};
  let gesturePanelTexts = {};
  let runtimePlatform = 'windows';

  function safeStringify(value) {
    try {
      return JSON.stringify(value);
    } catch (_error) {
      return '';
    }
  }

  function normalizeDraftRows(rows) {
    const out = [];
    const sourceRows = Array.isArray(rows) ? rows : [];
    for (const row of sourceRows) {
      const source = row || {};
      out.push({
        enabled: source.enabled !== false,
        trigger: `${source.trigger || ''}`.trim(),
        triggerChain: Array.isArray(source.triggerChain) ? source.triggerChain : `${source.trigger || ''}`.trim(),
        appScopeMode: scopeMode(source.appScopeMode || source.appScopeType || 'all'),
        appScopeApps: Array.isArray(source.appScopeApps) ? source.appScopeApps : [],
        appScopeDraft: `${source.appScopeDraft || ''}`,
        triggerButton: `${source.triggerButton || ''}`.trim(),
        gesturePattern: source.gesturePattern || {},
        keys: `${source.keys || ''}`.trim(),
      });
    }
    return out;
  }

  function readDraftSnapshot() {
    if (typeof window === 'undefined' || !window.localStorage) {
      return null;
    }
    try {
      const raw = window.localStorage.getItem(AUTOMATION_DRAFT_STORAGE_KEY);
      if (!raw) {
        return null;
      }
      const parsed = JSON.parse(raw);
      if (!parsed || parsed.version !== AUTOMATION_DRAFT_VERSION) {
        return null;
      }
      return parsed;
    } catch (_error) {
      return null;
    }
  }

  function removeDraftSnapshot() {
    if (typeof window === 'undefined' || !window.localStorage) {
      return;
    }
    try {
      window.localStorage.removeItem(AUTOMATION_DRAFT_STORAGE_KEY);
    } catch (_error) {
      // Ignore local cache write failures.
    }
  }

  function writeDraftSnapshot() {
    if (typeof window === 'undefined' || !window.localStorage) {
      return;
    }
    if (!lastIncomingSignature || skipDraftPersist) {
      return;
    }
    const snapshot = {
      version: AUTOMATION_DRAFT_VERSION,
      baseSignature: lastIncomingSignature,
      platform: runtimePlatform,
      state: {
        enabled: !!automationEnabled,
        gestureEnabled: !!gestureEnabled,
        gestureMinDistance: numberOr(gestureMinDistance, DEFAULT_GESTURE_MIN_DISTANCE),
        gestureSampleStep: numberOr(gestureSampleStep, DEFAULT_GESTURE_SAMPLE_STEP),
        gestureMaxDirections: numberOr(gestureMaxDirections, DEFAULT_GESTURE_MAX_DIRECTIONS),
        mouseTemplate: `${mouseTemplate || ''}`.trim(),
        gestureTemplate: `${gestureTemplate || ''}`.trim(),
        mouseRows: normalizeDraftRows(mouseRows),
        gestureRows: normalizeDraftRows(gestureRows),
      },
    };

    try {
      window.localStorage.setItem(AUTOMATION_DRAFT_STORAGE_KEY, safeStringify(snapshot));
    } catch (_error) {
      // Ignore local cache write failures.
    }
  }

  function restoreRowsFromDraft(rows, kind) {
    const options = optionsForKind(kind);
    const fallback = defaultTriggerForKind(kind);
    const source = Array.isArray(rows) ? rows : [];
    return source.map((item) => createRow(item, fallback, options));
  }

  function tryRestoreDraftSnapshot(signature) {
    const draft = readDraftSnapshot();
    if (!draft) {
      return;
    }

    if (draft.baseSignature !== signature) {
      removeDraftSnapshot();
      return;
    }
    if (draft.platform && draft.platform !== runtimePlatform) {
      removeDraftSnapshot();
      return;
    }

    const state = draft.state || {};
    automationEnabled = state.enabled === true;
    gestureEnabled = state.gestureEnabled === true;
    gestureMinDistance = numberOr(state.gestureMinDistance, DEFAULT_GESTURE_MIN_DISTANCE);
    gestureSampleStep = numberOr(state.gestureSampleStep, DEFAULT_GESTURE_SAMPLE_STEP);
    gestureMaxDirections = numberOr(state.gestureMaxDirections, DEFAULT_GESTURE_MAX_DIRECTIONS);
    mouseRows = restoreRowsFromDraft(state.mouseRows, 'mouse');
    gestureRows = restoreRowsFromDraft(state.gestureRows, 'gesture');
    mouseTemplate = `${state.mouseTemplate || ''}`.trim();
    gestureTemplate = `${state.gestureTemplate || ''}`.trim();
  }

  function t(key, fallback) {
    return textOf(i18n, key, fallback);
  }

  function numberOr(value, fallback) {
    const parsed = Number(value);
    return Number.isFinite(parsed) ? parsed : fallback;
  }

  function resolveTemplateProvider() {
    const provider = window.MfxAutomationTemplates;
    if (!provider) {
      return null;
    }
    if (typeof provider.list !== 'function' || typeof provider.mappings !== 'function') {
      return null;
    }
    return provider;
  }

  function isTemplateValid(selected, options) {
    if (!selected) {
      return true;
    }
    return options.some((item) => item.id === selected);
  }

  function nextRowId() {
    const id = `auto_row_${rowSeq}`;
    rowSeq += 1;
    return id;
  }

  function scopeMode(value) {
    const text = `${value || ''}`.trim().toLowerCase();
    return text === 'selected' || text === 'process' ? 'selected' : 'all';
  }

  function scopeAppsFromScopes(appScopes) {
    const out = [];
    const items = Array.isArray(appScopes) ? appScopes : [];
    for (const item of items) {
      const text = `${item || ''}`.trim().toLowerCase();
      if (!text.startsWith('process:')) {
        continue;
      }
      const app = text.slice('process:'.length).trim();
      if (!app || out.includes(app)) {
        continue;
      }
      out.push(app);
    }
    return out;
  }

  function buildScopeState(mode, apps, draft) {
    const appScopeMode = scopeMode(mode);
    if (appScopeMode === 'all') {
      return {
        appScopeMode: 'all',
        appScopeApps: [],
        appScopes: ['all'],
        appScope: 'all',
        appScopeDraft: '',
      };
    }
    const appScopes = serializeAppScopes('selected', apps, runtimePlatform);
    return {
      appScopeMode: 'selected',
      appScopeApps: scopeAppsFromScopes(appScopes),
      appScopes,
      appScope: serializeLegacyAppScope('selected', apps, runtimePlatform),
      appScopeDraft: `${draft || ''}`.trim(),
    };
  }

  function createRow(binding, fallbackTrigger, options) {
    const triggerChain = normalizeTriggerChain(binding?.triggerChain || binding?.trigger, options, fallbackTrigger);
    const scope = parseAppScopes(
      binding?.app_scopes || binding?.appScopes || binding?.app_scope || binding?.appScope,
      runtimePlatform);
    const scopeState = buildScopeState(scope.mode, scope.apps, binding?.appScopeDraft || '');
    const sourceModifiers = binding?.modifiers || {};
    const sourceModifierMode = `${sourceModifiers?.mode || ''}`.trim().toLowerCase();
    const sourcePrimary = !!sourceModifiers?.primary;
    const sourceShift = !!sourceModifiers?.shift;
    const sourceAlt = !!sourceModifiers?.alt;
    const normalizedModifierMode = sourceModifierMode === 'exact'
      ? 'exact'
      : sourceModifierMode === 'none'
        ? 'none'
        : ((sourcePrimary || sourceShift || sourceAlt) ? 'exact' : 'none');
    const normalizedPrimary = normalizedModifierMode === 'exact' ? sourcePrimary : false;
    const normalizedShift = normalizedModifierMode === 'exact' ? sourceShift : false;
    const normalizedAlt = normalizedModifierMode === 'exact' ? sourceAlt : false;

    return {
      id: nextRowId(),
      enabled: binding?.enabled !== false,
      triggerChain,
      trigger: triggerChain.join('>'),
      ...scopeState,
      triggerButton: binding?.triggerButton || binding?.trigger_button || defaultGestureButton || DEFAULT_GESTURE_TRIGGER_BUTTON,
      gesturePattern: binding?.gesturePattern || binding?.gesture_pattern || {
        mode: DEFAULT_GESTURE_PATTERN_MODE,
        matchThresholdPercent: DEFAULT_GESTURE_MATCH_THRESHOLD_PERCENT,
        customPoints: [],
        customStrokes: [],
      },
      modifiers: {
        mode: normalizedModifierMode,
        primary: normalizedPrimary,
        shift: normalizedShift,
        alt: normalizedAlt,
      },
      keys: `${binding?.keys || ''}`.trim(),
      note: '',
      hasConflict: false,
    };
  }

  function rowCollection(kind) {
    return kind === 'gesture' ? gestureRows : mouseRows;
  }

  function setRowCollection(kind, rows) {
    if (kind === 'gesture') {
      gestureRows = rows;
      return;
    }
    mouseRows = rows;
  }

  function optionsForKind(kind) {
    return kind === 'gesture' ? gestureOptions : mouseOptions;
  }

  function defaultTriggerForKind(kind) {
    return kind === 'gesture' ? defaultGestureTrigger : defaultMouseTrigger;
  }

  function optionLabelKey(group, value) {
    const text = `${value || ''}`.trim();
    if (!text) return '';

    if (group === 'mouse_action') return `auto_mouse_action_${text}`;
    if (group === 'app_scope') {
      if (text === 'process') return 'auto_app_scope_selected';
      return `auto_app_scope_${text}`;
    }
    if (group === 'gesture_pattern') return `auto_gesture_pattern_${text}`;
    if (group === 'gesture_button') return `auto_gesture_button_${text}`;
    return '';
  }

  function localizeOptions(options, group) {
    const localized = (options || []).map((option) => {
      const source = option || {};
      const value = `${source.value || ''}`.trim();
      if (!value) {
        return source;
      }
      const fallback = `${source.label || value}`;
      const key = optionLabelKey(group, value);
      return {
        ...source,
        label: key ? t(key, fallback) : fallback,
      };
    });
    if (group !== 'gesture_button') {
      return localized;
    }

    const weight = {
      left: 0,
      middle: 1,
      right: 2,
    };

    return localized.slice().sort((a, b) => {
      const av = `${a?.value || ''}`.trim().toLowerCase();
      const bv = `${b?.value || ''}`.trim().toLowerCase();
      const aw = Object.prototype.hasOwnProperty.call(weight, av) ? weight[av] : Number.MAX_SAFE_INTEGER;
      const bw = Object.prototype.hasOwnProperty.call(weight, bv) ? weight[bv] : Number.MAX_SAFE_INTEGER;
      if (aw !== bw) {
        return aw - bw;
      }
      return av.localeCompare(bv);
    });
  }

  function relocalizeOptionLabels() {
    mouseOptions = localizeOptions(mouseOptions, 'mouse_action');
    appScopeOptions = localizeOptions(appScopeOptions, 'app_scope');
    gestureOptions = localizeOptions(gestureOptions, 'gesture_pattern');
    gestureButtonOptions = localizeOptions(gestureButtonOptions, 'gesture_button');
  }

  function validationMessages() {
    const isMac = runtimePlatform === 'macos';
    return {
      duplicate: t('auto_conflict_trigger', 'Duplicate trigger chain + app scope. Keep only one enabled mapping per key.'),
      invalidScope: isMac
        ? t('auto_missing_scope_app_macos', 'Please add at least one target app name when selected-app scope is enabled.')
        : t('auto_missing_scope_app', 'Please add at least one target app exe when selected-app scope is enabled.'),
    };
  }

  function runValidation(kind) {
    const result = evaluateRows(
      rowCollection(kind),
      optionsForKind(kind),
      defaultTriggerForKind(kind),
      validationMessages(),
      runtimePlatform);

    setRowCollection(kind, result.rows);
    if (kind === 'gesture') {
      gestureValidation = result;
      return;
    }
    mouseValidation = result;
  }

  function normalizeRowPatch(row, key, value, options, fallback) {
    if (key === 'triggerChain' || key === 'trigger') {
      const triggerChain = normalizeTriggerChain(value, options, fallback);
      return {
        ...row,
        triggerChain,
        trigger: serializeTriggerChain(triggerChain, options, fallback),
      };
    }
    if (key === 'keys') {
      return {
        ...row,
        keys: `${value || ''}`,
      };
    }
    if (key === 'gesturePattern') {
      return {
        ...row,
        gesturePattern: {
          ...(row.gesturePattern || {}),
          ...(value || {}),
        },
      };
    }
    if (key === 'appScopeMode') {
      const appScopeMode = scopeMode(value);
      const nextApps = appScopeMode === 'selected' ? [...(row.appScopeApps || [])] : [];
      return {
        ...row,
        ...buildScopeState(appScopeMode, nextApps, row.appScopeDraft),
      };
    }
    if (key === 'appScopeDraft') {
      return {
        ...row,
        appScopeDraft: `${value || ''}`,
      };
    }
    if (key === 'appScopeAdd') {
      const nextApps = [...(row.appScopeApps || []), `${value || ''}`];
      return {
        ...row,
        ...buildScopeState('selected', nextApps, ''),
      };
    }
    if (key === 'appScopeRemove') {
      const target = `${value || ''}`.trim().toLowerCase();
      const nextApps = (row.appScopeApps || []).filter((item) => `${item || ''}`.trim().toLowerCase() !== target);
      return {
        ...row,
        ...buildScopeState('selected', nextApps, row.appScopeDraft),
      };
    }
    return {
      ...row,
      [key]: value,
    };
  }

  function updateRow(kind, rowId, key, value) {
    const options = optionsForKind(kind);
    const fallback = defaultTriggerForKind(kind);
    const nextRows = rowCollection(kind).map((row) => {
      if (row.id !== rowId) {
        return row;
      }
      return normalizeRowPatch(row, key, value, options, fallback);
    });
    setRowCollection(kind, nextRows);
    runValidation(kind);
  }

  function addMapping(kind, binding) {
    const options = optionsForKind(kind);
    const fallback = defaultTriggerForKind(kind);
    const nextRows = rowCollection(kind).concat(createRow(binding, fallback, options));
    setRowCollection(kind, nextRows);
    runValidation(kind);
  }

  function removeMapping(kind, rowId) {
    const nextRows = rowCollection(kind).filter((row) => row.id !== rowId);
    setRowCollection(kind, nextRows);
    runValidation(kind);
  }

  function syncTemplateOptions() {
    const provider = resolveTemplateProvider();
    const translate = (key, fallback) => t(key, fallback);

    mouseTemplateOptions = listTemplateOptions(provider, 'mouse', translate, runtimePlatform);
    gestureTemplateOptions = listTemplateOptions(provider, 'gesture', translate, runtimePlatform);

    if (!isTemplateValid(mouseTemplate, mouseTemplateOptions)) {
      mouseTemplate = '';
    }
    if (!isTemplateValid(gestureTemplate, gestureTemplateOptions)) {
      gestureTemplate = '';
    }
  }

  function applyTemplate(kind) {
    const provider = resolveTemplateProvider();
    const selected = kind === 'gesture' ? gestureTemplate : mouseTemplate;
    if (!selected || !provider) {
      return;
    }

    const options = optionsForKind(kind);
    const fallback = defaultTriggerForKind(kind);
    const bindings = readTemplateBindings(
      provider,
      kind,
      selected,
      options,
      fallback,
      runtimePlatform,
      defaultGestureButton || DEFAULT_GESTURE_TRIGGER_BUTTON);
    if (bindings.length === 0) {
      return;
    }

    const nextRows = upsertRowsByTrigger(
      rowCollection(kind),
      bindings,
      options,
      fallback,
      (binding) => createRow(binding, fallback, options),
      runtimePlatform);

    setRowCollection(kind, nextRows);
    runValidation(kind);
  }

  function buildIncomingSignature(normalized) {
    return safeStringify({
      platform: normalized.platform || 'windows',
      enabled: !!normalized.enabled,
      gestureEnabled: !!normalized.gestureEnabled,
      gestureMinDistance: numberOr(normalized.gestureMinDistance, DEFAULT_GESTURE_MIN_DISTANCE),
      gestureSampleStep: numberOr(normalized.gestureSampleStep, DEFAULT_GESTURE_SAMPLE_STEP),
      gestureMaxDirections: numberOr(normalized.gestureMaxDirections, DEFAULT_GESTURE_MAX_DIRECTIONS),
      defaultMouseTrigger: normalized.defaultMouseTrigger || '',
      defaultGestureTrigger: normalized.defaultGestureTrigger || '',
      defaultGestureButton: normalized.defaultGestureButton || DEFAULT_GESTURE_TRIGGER_BUTTON,
      mouseOptions: normalized.mouseOptions || [],
      appScopeOptions: normalized.appScopeOptions || [],
      gestureOptions: normalized.gestureOptions || [],
      gestureButtonOptions: normalized.gestureButtonOptions || [],
      mouseMappings: normalized.mouseMappings || [],
      gestureMappings: normalized.gestureMappings || [],
    });
  }

  function hydrateFromPayload(normalized, signature) {
    skipDraftPersist = true;
    rowSeq = 1;
    mouseTemplate = '';
    gestureTemplate = '';

    runtimePlatform = normalized.platform || 'windows';
    mouseOptions = normalized.mouseOptions;
    appScopeOptions = normalized.appScopeOptions;
    gestureOptions = normalized.gestureOptions;
    gestureButtonOptions = normalized.gestureButtonOptions;
    defaultMouseTrigger = normalized.defaultMouseTrigger;
    defaultGestureTrigger = normalized.defaultGestureTrigger;
    defaultGestureButton = normalized.defaultGestureButton;

    relocalizeOptionLabels();

    automationEnabled = normalized.enabled;
    gestureEnabled = normalized.gestureEnabled;
    gestureMinDistance = normalized.gestureMinDistance;
    gestureSampleStep = normalized.gestureSampleStep;
    gestureMaxDirections = normalized.gestureMaxDirections;

    mouseRows = normalized.mouseMappings.map((item) =>
      createRow(item, defaultMouseTrigger, mouseOptions));
    gestureRows = normalized.gestureMappings.map((item) =>
      createRow(item, defaultGestureTrigger, gestureOptions));

    tryRestoreDraftSnapshot(signature);
    runValidation('mouse');
    runValidation('gesture');
    syncTemplateOptions();
    lastIncomingSignature = signature;
    skipDraftPersist = false;
  }

  function panelTextsForKind(kind) {
    const isMac = runtimePlatform === 'macos';
    return {
      empty: kind === 'gesture'
        ? t('auto_empty_gesture', 'No gesture mappings yet. Click "Add mapping".')
        : t('auto_empty_mouse', 'No mouse mappings yet. Click "Add mapping".'),
      enabledTitle: t('label_auto_mapping_enabled', 'Enabled'),
      shortcutPlaceholder: isMac
        ? t('placeholder_shortcut_macos', 'Cmd+Shift+S')
        : t('placeholder_shortcut', 'Ctrl+Shift+S'),
      shortcutLabel: t('label_auto_output_shortcut', 'Output shortcut'),
      record: t('btn_record_shortcut', 'Record'),
      recordStop: t('btn_record_stop_save', 'Stop / Save'),
      recording: t('btn_recording', 'Press keys...'),
      captureHint: t(
        'hint_shortcut_capture',
        'Manual mode: type shortcut text directly. Auto mode: click "Record" to start native capture, then press combo.'),
      captureHintActive: t(
        'hint_shortcut_capture_active',
        'Recording mode: native capture is active (page shortcuts are blocked). Press combo, Esc to cancel, Backspace to clear.'),
      captureHintModifierActive: t(
        'hint_shortcut_capture_active_modifier',
        'Recording gesture trigger modifiers. Press Ctrl/Cmd/Shift/Alt(Option), Esc to cancel, Backspace to clear to no modifier.'),
      gestureTriggerShortcutLabel: t('label_auto_gesture_trigger_shortcut', 'Gesture trigger modifiers'),
      gestureTriggerShortcutPlaceholder: isMac
        ? t('placeholder_auto_gesture_trigger_shortcut_macos', 'Cmd / Cmd+Shift')
        : t('placeholder_auto_gesture_trigger_shortcut', 'Ctrl / Ctrl+Shift'),
      gestureTriggerModifiersAny: t('text_auto_gesture_trigger_any', 'Any modifier'),
      gestureTriggerModifiersNonePlaceholder: t(
        'text_auto_gesture_trigger_none_placeholder',
        'Optional (empty means no modifier)'),
      gestureTriggerModifiersNone: t('text_auto_gesture_trigger_none', 'No modifier'),
      gestureOutputShortcutLabel: t('label_auto_output_shortcut', 'Output shortcut'),
      modifierPrimary: t(
        isMac ? 'auto_modifier_primary_short_macos' : 'auto_modifier_primary_windows',
        isMac ? 'Cmd' : 'Ctrl'),
      modifierShift: t('auto_modifier_shift', 'Shift'),
      modifierAlt: t(
        isMac ? 'auto_modifier_alt_macos' : 'auto_modifier_alt_windows',
        isMac ? 'Option' : 'Alt'),
      scopeRefreshCatalog: t('btn_scope_refresh_catalog', 'Refresh app list'),
      scopeRefreshingCatalog: t('btn_scope_refreshing_catalog', 'Refreshing...'),
      scopePickFromFile: isMac
        ? t('btn_scope_pick_from_file_app', 'Pick app file')
        : t('btn_scope_pick_from_file', 'Pick exe file'),
      scopeSearchPlaceholder: t(
        isMac ? 'placeholder_scope_search_app' : 'placeholder_scope_search',
        isMac
          ? 'Search app name/app, or type manually (for example code.app)'
          : 'Search app name/exe, or type manually (for example code.exe)'),
      scopeCatalogLoading: t('text_scope_catalog_loading', 'Scanning app list...'),
      scopeCatalogEmpty: t(
        isMac ? 'text_scope_catalog_empty_app' : 'text_scope_catalog_empty',
        isMac
          ? 'No matching app. Keep typing to search, or add app manually.'
          : 'No matching app. Keep typing to search, or add exe manually.'),
      scopeCatalogLoadFailed: t(
        isMac ? 'text_scope_catalog_error_app' : 'text_scope_catalog_error',
        isMac
          ? 'Failed to scan app list. You can still add app manually or pick a file.'
          : 'Failed to scan app list. You can still add exe manually or pick a file.'),
      scopeAppPlaceholder: t(
        isMac ? 'placeholder_scope_process_app' : 'placeholder_scope_process',
        isMac ? 'for example code.app' : 'for example code.exe'),
      scopeAllLabel: t('auto_app_scope_all', 'All Apps'),
      scopeSelectedEmpty: t('text_scope_selected_empty', 'No app selected'),
      shortcutEmpty: t('text_shortcut_empty', 'No shortcut'),
      expand: t('btn_expand_mapping', 'Expand'),
      collapse: t('btn_collapse_mapping', 'Collapse'),
      remove: t('btn_remove_mapping', 'Remove'),
      add: t('btn_add_mapping', 'Add mapping'),
      addChainNode: t('btn_add_chain_node', 'Add chain node'),
      removeChainNode: t('btn_remove_chain_node', 'Remove node'),
      chainJoiner: t('label_chain_joiner', 'Then'),
      gestureTriggerButtonLabel: t('label_auto_gesture_trigger_button', 'Gesture trigger button'),
      gestureModePreset: t('auto_gesture_mode_preset', 'Preset'),
      gestureModeCustom: t('auto_gesture_mode_custom', 'Custom Draw'),
      gesturePatternTitle: t('label_auto_gesture_pattern', 'Gesture shape'),
      gesturePatternPreset: t('label_auto_gesture_pattern_preset', 'Preset gesture'),
      gesturePatternCustom: t('label_auto_gesture_pattern_custom', 'Custom drawing'),
      gestureThreshold: t('label_auto_gesture_threshold', 'Similarity threshold'),
      gestureCanvasEmpty: t('text_auto_gesture_canvas_empty', 'No custom stroke yet'),
      gestureCanvasClear: t('btn_auto_gesture_canvas_clear', 'Clear'),
      gestureCanvasUndo: t('btn_auto_gesture_canvas_undo', 'Undo'),
      gestureCanvasGuide: t('hint_auto_gesture_canvas', 'Press and drag to draw a custom gesture template.'),
      gestureCanvasLimitHint: t('hint_auto_gesture_canvas_max_strokes', 'Supports up to 4 strokes. Each stroke keeps index and direction.'),
      gestureCanvasLimitBadge: t('text_auto_gesture_canvas_limit_badge', '!'),
      gestureCanvasLimitReached: t('text_auto_gesture_canvas_limit_reached', 'Stroke limit reached (max 4).'),
      gestureCanvasStrokeCount: t('label_auto_gesture_canvas_strokes', 'Strokes'),
      gestureCanvasPointUnit: t('text_auto_gesture_canvas_point_unit', 'pt'),
      gestureCanvasNoDirection: t('text_auto_gesture_canvas_no_direction', 'No direction'),
      templateNone: t('auto_template_none', 'Select quick template'),
      templateTitle: kind === 'gesture'
        ? t('label_auto_gesture_template', 'Gesture quick template')
        : t('label_auto_mouse_template', 'Mouse quick template'),
      applyTemplate: t('btn_apply_template', 'Apply template'),
    };
  }

  function refreshLocalizedText() {
    uiText = {
      autoEnabled: t('label_auto_enabled', 'Enable automation'),
      mouseMappings: t('label_auto_mouse_mappings', 'Mouse action mappings'),
      gestureEnabled: t('label_auto_gesture_enabled', 'Enable gesture mapping'),
      gestureMinDistance: t('label_auto_gesture_min_distance', 'Min stroke distance (px)'),
      gestureSampleStep: t('label_auto_gesture_sample_step', 'Sampling step (px)'),
      gestureMaxDirections: t('label_auto_gesture_max_dirs', 'Max direction segments'),
      gestureMappings: t('label_auto_gesture_mappings', 'Gesture mappings'),
      gestureSetup: t('label_auto_gesture_setup', 'Gesture recognizer'),
      gestureDrawHint: t('hint_auto_gesture_draw', 'Each gesture can use a preset shape or your own drawing. Custom drawings will later match by similarity threshold.'),
      hint: t(
        'hint_automation',
        'Action chain trigger format: action1>action2 (for example left_click>scroll_down).'),
    };
    mousePanelTexts = panelTextsForKind('mouse');
    gesturePanelTexts = panelTextsForKind('gesture');
  }

  function onPanelRowChange(event) {
    const detail = event?.detail || {};
    updateRow(detail.kind, detail.rowId, detail.key, detail.value);
  }

  function onPanelRemove(event) {
    const detail = event?.detail || {};
    removeMapping(detail.kind, detail.rowId);
  }

  function onPanelAdd(event) {
    const detail = event?.detail || {};
    addMapping(detail.kind, {});
  }

  function onPanelTemplateChange(event) {
    const detail = event?.detail || {};
    if (detail.kind === 'gesture') {
      gestureTemplate = detail.value || '';
      return;
    }
    mouseTemplate = detail.value || '';
  }

  function onPanelApplyTemplate(event) {
    const detail = event?.detail || {};
    applyTemplate(detail.kind);
  }

  export function read() {
    return {
      enabled: !!automationEnabled,
      mouse_mappings: readMappings(mouseRows, mouseOptions, defaultMouseTrigger, runtimePlatform),
      gesture: {
        enabled: !!gestureEnabled,
        trigger_button: defaultGestureButton || DEFAULT_GESTURE_TRIGGER_BUTTON,
        min_stroke_distance_px: numberOr(gestureMinDistance, DEFAULT_GESTURE_MIN_DISTANCE),
        sample_step_px: numberOr(gestureSampleStep, DEFAULT_GESTURE_SAMPLE_STEP),
        max_directions: numberOr(gestureMaxDirections, DEFAULT_GESTURE_MAX_DIRECTIONS),
        mappings: readMappings(gestureRows, gestureOptions, defaultGestureTrigger, runtimePlatform),
      },
    };
  }

  export function validate() {
    runValidation('mouse');
    runValidation('gesture');
    const isMac = runtimePlatform === 'macos';

    if (!automationEnabled) {
      return { ok: true };
    }
    if (mouseValidation.hasInvalidScope) {
      return {
        ok: false,
        message: isMac
          ? t('auto_validation_missing_scope_app_macos', 'At least one enabled mapping uses selected-app scope but has no app name.')
          : t('auto_validation_missing_scope_app', 'At least one enabled mapping uses selected-app scope but has no app exe name.'),
      };
    }
    if (mouseValidation.hasDuplicateTrigger) {
      return {
        ok: false,
        message: t('auto_validation_mouse_duplicate', 'Mouse mappings contain duplicate trigger chain + app scope keys.'),
      };
    }
    if (!gestureEnabled) {
      return { ok: true };
    }
    if (gestureValidation.hasInvalidScope) {
      return {
        ok: false,
        message: isMac
          ? t('auto_validation_missing_scope_app_macos', 'At least one enabled mapping uses selected-app scope but has no app name.')
          : t('auto_validation_missing_scope_app', 'At least one enabled mapping uses selected-app scope but has no app exe name.'),
      };
    }
    if (gestureValidation.hasDuplicateTrigger) {
      return {
        ok: false,
        message: t('auto_validation_gesture_duplicate', 'Gesture mappings contain duplicate trigger chain + app scope keys.'),
      };
    }
    return { ok: true };
  }

  refreshLocalizedText();

  $: {
    const normalized = normalizeAutomationPayload(schema, payloadState);
    const signature = buildIncomingSignature(normalized);
    if (signature && signature !== lastIncomingSignature) {
      hydrateFromPayload(normalized, signature);
    }
  }

  $: if (i18n !== lastI18nRef) {
    lastI18nRef = i18n;
    refreshLocalizedText();
    relocalizeOptionLabels();
    runValidation('mouse');
    runValidation('gesture');
    syncTemplateOptions();
  }

  $: if (!skipDraftPersist && lastIncomingSignature) {
    writeDraftSnapshot();
  }
</script>

<div class="automation-flow">
  <section class="automation-card automation-card--master">
    <div class="automation-card-head">
      <h3 class="automation-card-title">{uiText.autoEnabled}</h3>
      <label class="automation-inline-toggle" for="auto_enabled">
        <span>{automationEnabled ? 'ON' : 'OFF'}</span>
        <input id="auto_enabled" type="checkbox" bind:checked={automationEnabled} />
      </label>
    </div>
    <p class="automation-card-hint">{uiText.hint}</p>
  </section>

  <section class="automation-card automation-card--mouse">
    <div class="automation-card-head">
      <h3 class="automation-card-title">{uiText.mouseMappings}</h3>
    </div>
    <div class="automation-card-body">
      <div class="automation-mapping-shell automation-mapping-shell--mouse">
        <MappingPanel
          kind="mouse"
          platform={runtimePlatform}
          rows={mouseRows}
          options={mouseOptions}
          scopeOptions={appScopeOptions}
          templateValue={mouseTemplate}
          templateOptions={mouseTemplateOptions}
          texts={mousePanelTexts}
          on:rowchange={onPanelRowChange}
          on:remove={onPanelRemove}
          on:add={onPanelAdd}
          on:templatechange={onPanelTemplateChange}
          on:applytemplate={onPanelApplyTemplate}
        />
      </div>
    </div>
  </section>

  <section class="automation-card automation-card--gesture-map">
    <div class="automation-card-head">
      <h3 class="automation-card-title">{uiText.gestureMappings}</h3>
      <label class="automation-inline-toggle" for="auto_gesture_enabled">
        <span>{gestureEnabled ? 'ON' : 'OFF'}</span>
        <input id="auto_gesture_enabled" type="checkbox" bind:checked={gestureEnabled} />
      </label>
    </div>
    <div class="automation-card-body">
      <div class="automation-gesture-shell">
        <div class="automation-gesture-setup-title">{uiText.gestureSetup}</div>
        <div class="automation-gesture-intro">{uiText.gestureDrawHint}</div>
        <div class="automation-gesture-settings">
          <label for="auto_gesture_min_distance">{uiText.gestureMinDistance}</label>
          <input id="auto_gesture_min_distance" type="number" min="10" max="4000" bind:value={gestureMinDistance} />

          <label for="auto_gesture_sample_step">{uiText.gestureSampleStep}</label>
          <input id="auto_gesture_sample_step" type="number" min="2" max="256" bind:value={gestureSampleStep} />

          <label for="auto_gesture_max_dirs">{uiText.gestureMaxDirections}</label>
          <input id="auto_gesture_max_dirs" type="number" min="1" max="8" bind:value={gestureMaxDirections} />
        </div>
      </div>
      <div class="automation-mapping-shell automation-mapping-shell--gesture">
        <MappingPanel
          kind="gesture"
          platform={runtimePlatform}
          rows={gestureRows}
          options={gestureOptions}
          scopeOptions={appScopeOptions}
          gestureButtonOptions={gestureButtonOptions}
          templateValue={gestureTemplate}
          templateOptions={gestureTemplateOptions}
          texts={gesturePanelTexts}
          on:rowchange={onPanelRowChange}
          on:remove={onPanelRemove}
          on:add={onPanelAdd}
          on:templatechange={onPanelTemplateChange}
          on:applytemplate={onPanelApplyTemplate}
        />
      </div>
    </div>
  </section>
</div>
