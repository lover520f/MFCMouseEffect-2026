<script>
  import MappingPanel from './MappingPanel.svelte';
  import {
    DEFAULT_GESTURE_MAX_DIRECTIONS,
    DEFAULT_GESTURE_MIN_DISTANCE,
    DEFAULT_GESTURE_SAMPLE_STEP,
    DEFAULT_GESTURE_TRIGGER_BUTTON,
  } from './defaults.js';
  import {
    evaluateRows,
    listTemplateOptions,
    normalizeAutomationPayload,
    readMappings,
    readTemplateBindings,
    sanitizeOptionValue,
    textOf,
    upsertRowsByTrigger,
  } from './model.js';
  import { normalizeTriggerChain, serializeTriggerChain } from './trigger-chain.js';

  export let schema = {};
  export let payloadState = {};
  export let i18n = {};

  let mouseOptions = [];
  let gestureOptions = [];
  let gestureButtonOptions = [];

  let defaultMouseTrigger = '';
  let defaultGestureTrigger = '';
  let defaultGestureButton = DEFAULT_GESTURE_TRIGGER_BUTTON;

  let automationEnabled = false;
  let gestureEnabled = false;
  let gestureTriggerButton = DEFAULT_GESTURE_TRIGGER_BUTTON;
  let gestureMinDistance = DEFAULT_GESTURE_MIN_DISTANCE;
  let gestureSampleStep = DEFAULT_GESTURE_SAMPLE_STEP;
  let gestureMaxDirections = DEFAULT_GESTURE_MAX_DIRECTIONS;

  let mouseRows = [];
  let gestureRows = [];
  let rowSeq = 1;

  let mouseValidation = { hasMissingShortcut: false, hasDuplicateTrigger: false };
  let gestureValidation = { hasMissingShortcut: false, hasDuplicateTrigger: false };

  let mouseTemplate = '';
  let gestureTemplate = '';
  let mouseTemplateOptions = [];
  let gestureTemplateOptions = [];

  let lastSchemaRef = null;
  let lastPayloadRef = null;
  let lastI18nRef = null;
  let uiText = {};
  let mousePanelTexts = {};
  let gesturePanelTexts = {};

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

  function createRow(binding, fallbackTrigger, options) {
    const triggerChain = normalizeTriggerChain(binding?.triggerChain || binding?.trigger, options, fallbackTrigger);
    return {
      id: nextRowId(),
      enabled: binding?.enabled !== false,
      triggerChain,
      trigger: triggerChain.join('>'),
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
    if (group === 'gesture_pattern') return `auto_gesture_pattern_${text}`;
    if (group === 'gesture_button') return `auto_gesture_button_${text}`;
    return '';
  }

  function localizeOptions(options, group) {
    return (options || []).map((option) => {
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
  }

  function relocalizeOptionLabels() {
    mouseOptions = localizeOptions(mouseOptions, 'mouse_action');
    gestureOptions = localizeOptions(gestureOptions, 'gesture_pattern');
    gestureButtonOptions = localizeOptions(gestureButtonOptions, 'gesture_button');
  }

  function validationMessages() {
    return {
      missing: t('auto_missing_shortcut', 'Shortcut is required for enabled mapping.'),
      duplicate: t('auto_conflict_trigger', 'Duplicate trigger chain. Keep only one enabled mapping per trigger chain.'),
    };
  }

  function runValidation(kind) {
    const result = evaluateRows(
      rowCollection(kind),
      optionsForKind(kind),
      defaultTriggerForKind(kind),
      validationMessages());

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

    mouseTemplateOptions = listTemplateOptions(provider, 'mouse', translate);
    gestureTemplateOptions = listTemplateOptions(provider, 'gesture', translate);

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
    const bindings = readTemplateBindings(provider, kind, selected, options, fallback);
    if (bindings.length === 0) {
      return;
    }

    const nextRows = upsertRowsByTrigger(
      rowCollection(kind),
      bindings,
      options,
      fallback,
      (binding) => createRow(binding, fallback, options));

    setRowCollection(kind, nextRows);
    runValidation(kind);
  }

  function hydrateFromPayload() {
    rowSeq = 1;
    mouseTemplate = '';
    gestureTemplate = '';

    const normalized = normalizeAutomationPayload(schema, payloadState);
    mouseOptions = normalized.mouseOptions;
    gestureOptions = normalized.gestureOptions;
    gestureButtonOptions = normalized.gestureButtonOptions;
    defaultMouseTrigger = normalized.defaultMouseTrigger;
    defaultGestureTrigger = normalized.defaultGestureTrigger;
    defaultGestureButton = normalized.defaultGestureButton;

    relocalizeOptionLabels();

    automationEnabled = normalized.enabled;
    gestureEnabled = normalized.gestureEnabled;
    gestureTriggerButton = normalized.gestureTriggerButton;
    gestureMinDistance = normalized.gestureMinDistance;
    gestureSampleStep = normalized.gestureSampleStep;
    gestureMaxDirections = normalized.gestureMaxDirections;

    mouseRows = normalized.mouseMappings.map((item) =>
      createRow(item, defaultMouseTrigger, mouseOptions));
    gestureRows = normalized.gestureMappings.map((item) =>
      createRow(item, defaultGestureTrigger, gestureOptions));

    runValidation('mouse');
    runValidation('gesture');
    syncTemplateOptions();
  }

  function panelTextsForKind(kind) {
    return {
      empty: kind === 'gesture'
        ? t('auto_empty_gesture', 'No gesture mappings yet. Click "Add mapping".')
        : t('auto_empty_mouse', 'No mouse mappings yet. Click "Add mapping".'),
      enabledTitle: t('label_auto_mapping_enabled', 'Enabled'),
      shortcutPlaceholder: t('placeholder_shortcut', 'Ctrl+Shift+S'),
      record: t('btn_record_shortcut', 'Record'),
      recordStop: t('btn_record_stop_save', 'Stop / Save'),
      recording: t('btn_recording', 'Press keys...'),
      captureHint: t(
        'hint_shortcut_capture',
        'Manual mode: type shortcut text directly. Auto mode: click "Record" to start native capture, then press combo.'),
      captureHintActive: t(
        'hint_shortcut_capture_active',
        'Recording mode: native capture is active (page shortcuts are blocked). Press combo, Esc to cancel, Backspace to clear.'),
      remove: t('btn_remove_mapping', 'Remove'),
      add: t('btn_add_mapping', 'Add mapping'),
      addChainNode: t('btn_add_chain_node', 'Add chain node'),
      removeChainNode: t('btn_remove_chain_node', 'Remove node'),
      chainJoiner: t('label_chain_joiner', 'Then'),
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
      gestureTriggerButton: t('label_auto_gesture_trigger_button', 'Gesture trigger button'),
      gestureMinDistance: t('label_auto_gesture_min_distance', 'Min stroke distance (px)'),
      gestureSampleStep: t('label_auto_gesture_sample_step', 'Sampling step (px)'),
      gestureMaxDirections: t('label_auto_gesture_max_dirs', 'Max direction segments'),
      gestureMappings: t('label_auto_gesture_mappings', 'Gesture mappings'),
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
      mouse_mappings: readMappings(mouseRows, mouseOptions, defaultMouseTrigger),
      gesture: {
        enabled: !!gestureEnabled,
        trigger_button: sanitizeOptionValue(
          gestureTriggerButton,
          gestureButtonOptions,
          defaultGestureButton || DEFAULT_GESTURE_TRIGGER_BUTTON),
        min_stroke_distance_px: numberOr(gestureMinDistance, DEFAULT_GESTURE_MIN_DISTANCE),
        sample_step_px: numberOr(gestureSampleStep, DEFAULT_GESTURE_SAMPLE_STEP),
        max_directions: numberOr(gestureMaxDirections, DEFAULT_GESTURE_MAX_DIRECTIONS),
        mappings: readMappings(gestureRows, gestureOptions, defaultGestureTrigger),
      },
    };
  }

  export function validate() {
    runValidation('mouse');
    runValidation('gesture');

    if (!automationEnabled) {
      return { ok: true };
    }
    if (mouseValidation.hasMissingShortcut) {
      return {
        ok: false,
        message: t('auto_validation_missing_shortcut', 'At least one enabled mapping has empty shortcut text.'),
      };
    }
    if (mouseValidation.hasDuplicateTrigger) {
      return {
        ok: false,
        message: t('auto_validation_mouse_duplicate', 'Mouse mappings contain duplicate trigger chains.'),
      };
    }
    if (!gestureEnabled) {
      return { ok: true };
    }
    if (gestureValidation.hasMissingShortcut) {
      return {
        ok: false,
        message: t('auto_validation_missing_shortcut', 'At least one enabled mapping has empty shortcut text.'),
      };
    }
    if (gestureValidation.hasDuplicateTrigger) {
      return {
        ok: false,
        message: t('auto_validation_gesture_duplicate', 'Gesture mappings contain duplicate trigger chains.'),
      };
    }
    return { ok: true };
  }

  refreshLocalizedText();

  $: if (schema !== lastSchemaRef || payloadState !== lastPayloadRef) {
    lastSchemaRef = schema;
    lastPayloadRef = payloadState;
    hydrateFromPayload();
  }

  $: if (i18n !== lastI18nRef) {
    lastI18nRef = i18n;
    refreshLocalizedText();
    relocalizeOptionLabels();
    runValidation('mouse');
    runValidation('gesture');
    syncTemplateOptions();
  }
</script>

<div class="grid automation-grid">
  <label for="auto_enabled">{uiText.autoEnabled}</label>
  <input id="auto_enabled" type="checkbox" bind:checked={automationEnabled} />

  <div class="automation-field-label">{uiText.mouseMappings}</div>
  <MappingPanel
    kind="mouse"
    rows={mouseRows}
    options={mouseOptions}
    templateValue={mouseTemplate}
    templateOptions={mouseTemplateOptions}
    texts={mousePanelTexts}
    on:rowchange={onPanelRowChange}
    on:remove={onPanelRemove}
    on:add={onPanelAdd}
    on:templatechange={onPanelTemplateChange}
    on:applytemplate={onPanelApplyTemplate}
  />

  <label for="auto_gesture_enabled">{uiText.gestureEnabled}</label>
  <input id="auto_gesture_enabled" type="checkbox" bind:checked={gestureEnabled} />

  <label for="auto_gesture_trigger_button">{uiText.gestureTriggerButton}</label>
  <select id="auto_gesture_trigger_button" bind:value={gestureTriggerButton}>
    {#each gestureButtonOptions as option (option.value)}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  <label for="auto_gesture_min_distance">{uiText.gestureMinDistance}</label>
  <input id="auto_gesture_min_distance" type="number" min="10" max="4000" bind:value={gestureMinDistance} />

  <label for="auto_gesture_sample_step">{uiText.gestureSampleStep}</label>
  <input id="auto_gesture_sample_step" type="number" min="2" max="256" bind:value={gestureSampleStep} />

  <label for="auto_gesture_max_dirs">{uiText.gestureMaxDirections}</label>
  <input id="auto_gesture_max_dirs" type="number" min="1" max="8" bind:value={gestureMaxDirections} />

  <div class="automation-field-label">{uiText.gestureMappings}</div>
  <MappingPanel
    kind="gesture"
    rows={gestureRows}
    options={gestureOptions}
    templateValue={gestureTemplate}
    templateOptions={gestureTemplateOptions}
    texts={gesturePanelTexts}
    on:rowchange={onPanelRowChange}
    on:remove={onPanelRemove}
    on:add={onPanelAdd}
    on:templatechange={onPanelTemplateChange}
    on:applytemplate={onPanelApplyTemplate}
  />

  <div class="hint span2">
    {uiText.hint}
  </div>
</div>
