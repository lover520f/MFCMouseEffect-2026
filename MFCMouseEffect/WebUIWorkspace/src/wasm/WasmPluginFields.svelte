<script>
  import {
    clampFloatByRange,
    clampIntByRange,
    normalizePolicyRanges,
    resolvePolicyInputValue,
  } from './policy-model.js';
  import {
    buildWasmBudgetFlagsText,
    buildWasmCallMetricsText,
    buildWasmLifetimeInvokeText,
    buildWasmLifetimeRenderText,
    hasWasmDiagnosticWarning,
  } from './diagnostics-model.js';
  import {
    normalizeActionErrorCode,
    resolveWasmActionErrorMessage,
  } from './action-error-model.js';
  import { normalizeWasmState } from './state-model.js';

  export let schemaState = {};
  export let payloadState = {};
  export let cursorDecoration = {};
  export let cursorDecorationOptions = [];
  export let i18n = {};
  export let onAction = null;
  export let onCursorDecorationChange = null;

  function text(key, fallback) {
    const value = i18n || {};
    return value[key] || fallback;
  }

  function toNumber(value, fallback) {
    const parsed = Number(value);
    return Number.isFinite(parsed) ? parsed : fallback;
  }

  function normalizeCursorDecoration(input) {
    const value = input || {};
    return {
      enabled: value.enabled === true,
      plugin_id: `${value.plugin_id || 'ring'}`.trim() || 'ring',
      color_hex: `${value.color_hex || '#ff5a5a'}`.trim() || '#ff5a5a',
      size_px: toNumber(value.size_px, 22),
      alpha_percent: toNumber(value.alpha_percent, 82),
    };
  }

  function sameCursorDecoration(left, right) {
    const a = normalizeCursorDecoration(left);
    const b = normalizeCursorDecoration(right);
    return a.enabled === b.enabled
      && a.plugin_id === b.plugin_id
      && a.color_hex === b.color_hex
      && a.size_px === b.size_px
      && a.alpha_percent === b.alpha_percent;
  }

  function resolveCursorDecorationNoneLabel(options) {
    const source = Array.isArray(options) ? options : [];
    const hasNonAsciiLabel = source.some((item) => /[^\x00-\x7F]/.test(`${item?.label || ''}`));
    return hasNonAsciiLabel ? '无' : 'None';
  }

  function buildCursorDecorationOptions(options) {
    const source = Array.isArray(options) ? options : [];
    const normalized = source
      .filter((item) => {
        const value = `${item?.value || ''}`.trim();
        return value.length > 0 && value !== '__disabled__';
      })
      .map((item) => ({
        value: `${item?.value || ''}`.trim(),
        label: `${item?.label || item?.value || ''}`.trim(),
      }));
    normalized.push({
      value: '__disabled__',
      label: resolveCursorDecorationNoneLabel(source),
    });
    return normalized;
  }

  function decorationSelectValue(value) {
    const normalized = normalizeCursorDecoration(value);
    return normalized.enabled ? normalized.plugin_id : '__disabled__';
  }

  function normalizeCatalogItems(input) {
    const source = Array.isArray(input) ? input : [];
    const out = [];
    for (const item of source) {
      const value = item || {};
      const manifestPath = `${value.manifest_path || ''}`.trim();
      if (!manifestPath || !supportsEffectsCatalogItem(value.input_kinds, value.surfaces, value.has_explicit_surfaces)) {
        continue;
      }
      out.push({
        id: `${value.id || ''}`.trim(),
        name: `${value.name || ''}`.trim(),
        version: `${value.version || ''}`.trim(),
        api_version: Number(value.api_version) || 0,
        surfaces: Array.isArray(value.surfaces)
          ? value.surfaces.map((item) => `${item || ''}`.trim()).filter((item) => item.length > 0)
          : [],
        has_explicit_surfaces: !!value.has_explicit_surfaces,
        input_kinds: Array.isArray(value.input_kinds)
          ? value.input_kinds.map((item) => `${item || ''}`.trim()).filter((item) => item.length > 0)
          : [],
        enable_frame_tick: value.enable_frame_tick !== false,
        manifest_path: manifestPath,
        wasm_path: `${value.wasm_path || ''}`.trim(),
      });
    }
    return out;
  }

  function supportsEffectsCatalogItem(inputKinds, surfaces, hasExplicitSurfaces) {
    const normalizedSurfaces = Array.isArray(surfaces)
      ? surfaces.map((entry) => `${entry || ''}`.trim()).filter((entry) => entry.length > 0)
      : [];
    if (hasExplicitSurfaces) {
      return normalizedSurfaces.includes('effects');
    }
    const kinds = Array.isArray(inputKinds)
      ? inputKinds.map((entry) => `${entry || ''}`.trim()).filter((entry) => entry.length > 0)
      : [];
    if (kinds.length === 0) {
      return true;
    }
    return kinds.some((entry) => !entry.startsWith('indicator_'));
  }

  function supportsEffectsChannelKinds(channelId, inputKinds) {
    const id = sanitizeChannelId(channelId);
    const kinds = Array.isArray(inputKinds)
      ? inputKinds.map((entry) => `${entry || ''}`.trim().toLowerCase()).filter((entry) => entry.length > 0)
      : [];
    if (kinds.length === 0) {
      return true;
    }
    if (id === 'click') {
      return kinds.includes('click');
    }
    if (id === 'trail') {
      return kinds.includes('move') || kinds.includes('trail');
    }
    if (id === 'scroll') {
      return kinds.includes('scroll');
    }
    if (id === 'hold') {
      return kinds.includes('hold') || kinds.some((entry) => entry.startsWith('hold_'));
    }
    if (id === 'hover') {
      return kinds.includes('hover') || kinds.some((entry) => entry.startsWith('hover_'));
    }
    return true;
  }

  function buildChannelCatalogMap(items) {
    const source = Array.isArray(items) ? items : [];
    const grouped = createEmptyChannelCatalogMap();
    for (const channelId of EFFECT_CHANNEL_IDS) {
      const matches = source.filter((item) => supportsEffectsChannelKinds(channelId, item?.input_kinds));
      grouped[channelId] = matches.length > 0 ? matches : source;
    }
    return grouped;
  }

  function boolText(value) {
    return value ? text('wasm_text_yes', 'Yes') : text('wasm_text_no', 'No');
  }

  function pluginLabel(plugin) {
    const id = `${plugin?.id || ''}`.trim();
    const name = `${plugin?.name || ''}`.trim();
    const version = `${plugin?.version || ''}`.trim();
    const title = name || id || text('wasm_text_unknown_plugin', 'Unknown plugin');
    if (!version) {
      return `${title}${pluginRouteLabel(plugin)}`;
    }
    return `${title} (${version})${pluginRouteLabel(plugin)}`;
  }

  function pluginRouteLabel(plugin) {
    const surfaces = Array.isArray(plugin?.surfaces) && plugin.surfaces.length > 0
      ? plugin.surfaces.join('/')
      : (plugin?.has_explicit_surfaces ? 'none' : 'auto');
    const kinds = Array.isArray(plugin?.input_kinds) && plugin.input_kinds.length > 0
      ? plugin.input_kinds.join('/')
      : 'all';
    const frame = plugin?.enable_frame_tick === false ? 'frame:off' : 'frame:on';
    return ` [surface:${surfaces}; ${kinds}; ${frame}]`;
  }

  function normalizeManifestPathForCompare(path) {
    const textValue = `${path || ''}`.trim();
    if (!textValue) {
      return '';
    }
    return textValue.replace(/\\/g, '/').replace(/\/+/g, '/').toLowerCase();
  }

  function isConfiguredManifest(manifestPath, configuredManifestPath) {
    const expected = normalizeManifestPathForCompare(manifestPath);
    if (!expected) {
      return false;
    }
    const configured = normalizeManifestPathForCompare(configuredManifestPath);
    return expected === configured;
  }

  function renderStatsText(snapshot) {
    const s = snapshot || normalizeWasmState({});
    return `${text('label_wasm_last_rendered', 'Rendered by WASM')}: ${boolText(s.last_rendered_by_wasm)}, `
      + `text=${s.last_executed_text_commands}, image=${s.last_executed_image_commands}, `
      + `pulse=${s.last_executed_pulse_commands}, polyline=${s.last_executed_polyline_commands}, pathStroke=${s.last_executed_path_stroke_commands}, pathFill=${s.last_executed_path_fill_commands}, glow=${s.last_executed_glow_batch_commands}, sprite=${s.last_executed_sprite_batch_commands}, `
      + `glowEmitter=${s.last_executed_glow_emitter_commands}, spriteEmitter=${s.last_executed_sprite_emitter_commands}, `
      + `${text('label_wasm_metric_throttled', 'Throttled')}=${s.last_throttled_render_commands} `
      + `(cap=${s.last_throttled_by_capacity_render_commands}, int=${s.last_throttled_by_interval_render_commands}), `
      + `${text('label_wasm_dropped_commands', 'Dropped commands')}=${s.last_dropped_render_commands}`;
  }

  function renderCallMetricsText(snapshot) {
    const s = snapshot || normalizeWasmState({});
    return buildWasmCallMetricsText(s, text);
  }

  function renderBudgetFlagsText(snapshot) {
    const s = snapshot || normalizeWasmState({});
    return buildWasmBudgetFlagsText(s, text);
  }

  function renderLifetimeInvokeText(snapshot) {
    const s = snapshot || normalizeWasmState({});
    return buildWasmLifetimeInvokeText(s, text);
  }

  function renderLifetimeRenderText(snapshot) {
    const s = snapshot || normalizeWasmState({});
    return buildWasmLifetimeRenderText(s, text);
  }

  function isDiagnosticWarning(snapshot) {
    const s = snapshot || normalizeWasmState({});
    return hasWasmDiagnosticWarning(s);
  }

  function runtimeBudgetText(snapshot) {
    const s = snapshot || normalizeWasmState({});
    const buffer = s.runtime_output_buffer_bytes || '-';
    const commands = s.runtime_max_commands || '-';
    const executionMs = s.runtime_max_execution_ms || '-';
    return `buffer=${buffer}, commands=${commands}, exec=${executionMs}ms`;
  }

  function runtimeToggleText(enabled) {
    return enabled
      ? text('text_wasm_runtime_on', 'Using Plugin')
      : text('text_wasm_runtime_off', 'Plugin Off');
  }

  function runtimeToggleTip(enabled) {
    return enabled
      ? text('tip_wasm_toggle_disable', 'Currently enabled. Click to disable plugin runtime path.')
      : text('tip_wasm_toggle_enable', 'Currently disabled. Click to enable plugin runtime path.');
  }

  const EFFECT_CHANNELS = [
    { id: 'click', labelKey: 'label_click', fallback: 'Click' },
    { id: 'trail', labelKey: 'label_trail', fallback: 'Trail' },
    { id: 'scroll', labelKey: 'label_scroll', fallback: 'Scroll' },
    { id: 'hold', labelKey: 'label_hold', fallback: 'Hold' },
    { id: 'hover', labelKey: 'label_hover', fallback: 'Hover' },
  ];
  const EFFECT_CHANNEL_IDS = EFFECT_CHANNELS.map((entry) => entry.id);

  function createEmptyChannelManifestMap() {
    return {
      click: '',
      trail: '',
      scroll: '',
      hold: '',
      hover: '',
    };
  }

  function createEmptyChannelCatalogMap() {
    return {
      click: [],
      trail: [],
      scroll: [],
      hold: [],
      hover: [],
    };
  }

  function createEmptyChannelToggleOverrideMap() {
    return {
      click: null,
      trail: null,
      scroll: null,
      hold: null,
      hover: null,
    };
  }

  function channelPolicyKey(channelId) {
    return `manifest_path_${channelId}`;
  }

  function sanitizeChannelId(channelId) {
    const id = `${channelId || ''}`.trim().toLowerCase();
    return EFFECT_CHANNEL_IDS.includes(id) ? id : 'click';
  }

  function channelLabel(channelId) {
    const id = sanitizeChannelId(channelId);
    const entry = EFFECT_CHANNELS.find((item) => item.id === id);
    return text(entry?.labelKey || '', entry?.fallback || id);
  }

  function configuredManifestPathForChannel(snapshot, channelId) {
    const s = snapshot || {};
    const id = sanitizeChannelId(channelId);
    if (id === 'click') {
      return `${s.configured_manifest_path_click || ''}`.trim() || `${s.configured_manifest_path || ''}`.trim();
    }
    if (id === 'trail') {
      return `${s.configured_manifest_path_trail || ''}`.trim() || `${s.configured_manifest_path || ''}`.trim();
    }
    if (id === 'scroll') {
      return `${s.configured_manifest_path_scroll || ''}`.trim() || `${s.configured_manifest_path || ''}`.trim();
    }
    if (id === 'hold') {
      return `${s.configured_manifest_path_hold || ''}`.trim() || `${s.configured_manifest_path || ''}`.trim();
    }
    if (id === 'hover') {
      return `${s.configured_manifest_path_hover || ''}`.trim() || `${s.configured_manifest_path || ''}`.trim();
    }
    return `${s.configured_manifest_path || ''}`.trim();
  }

  function configuredManifestPathForChannelRaw(snapshot, channelId) {
    const s = snapshot || {};
    const id = sanitizeChannelId(channelId);
    if (id === 'click') {
      return `${s.configured_manifest_path_click || ''}`.trim();
    }
    if (id === 'trail') {
      return `${s.configured_manifest_path_trail || ''}`.trim();
    }
    if (id === 'scroll') {
      return `${s.configured_manifest_path_scroll || ''}`.trim();
    }
    if (id === 'hold') {
      return `${s.configured_manifest_path_hold || ''}`.trim();
    }
    if (id === 'hover') {
      return `${s.configured_manifest_path_hover || ''}`.trim();
    }
    return '';
  }

  function activeManifestPathForChannel(snapshot, channelId) {
    const s = snapshot || {};
    const id = sanitizeChannelId(channelId);
    if (id === 'click') {
      return `${s.active_manifest_path_click || ''}`.trim() || `${s.active_manifest_path || ''}`.trim();
    }
    if (id === 'trail') {
      return `${s.active_manifest_path_trail || ''}`.trim();
    }
    if (id === 'scroll') {
      return `${s.active_manifest_path_scroll || ''}`.trim();
    }
    if (id === 'hold') {
      return `${s.active_manifest_path_hold || ''}`.trim();
    }
    if (id === 'hover') {
      return `${s.active_manifest_path_hover || ''}`.trim();
    }
    return `${s.active_manifest_path || ''}`.trim();
  }

  function withConfiguredManifestPath(snapshot, channelId, manifestPath) {
    const s = snapshot || {};
    const id = sanitizeChannelId(channelId);
    const value = `${manifestPath || ''}`.trim();
    const next = { ...s };
    if (id === 'click') {
      next.configured_manifest_path_click = value;
      return next;
    }
    if (id === 'trail') {
      next.configured_manifest_path_trail = value;
      return next;
    }
    if (id === 'scroll') {
      next.configured_manifest_path_scroll = value;
      return next;
    }
    if (id === 'hold') {
      next.configured_manifest_path_hold = value;
      return next;
    }
    next.configured_manifest_path_hover = value;
    return next;
  }

  function channelBindingMapFromSnapshot(snapshot) {
    const source = snapshot || {};
    return {
      click: configuredManifestPathForChannelRaw(source, 'click'),
      trail: configuredManifestPathForChannelRaw(source, 'trail'),
      scroll: configuredManifestPathForChannelRaw(source, 'scroll'),
      hold: configuredManifestPathForChannelRaw(source, 'hold'),
      hover: configuredManifestPathForChannelRaw(source, 'hover'),
    };
  }

  function setConfiguredBinding(channelId, manifestPath) {
    const id = sanitizeChannelId(channelId);
    configuredManifestPathByChannel = {
      ...configuredManifestPathByChannel,
      [id]: `${manifestPath || ''}`.trim(),
    };
  }

  function setChannelToggleOverride(channelId, value) {
    const id = sanitizeChannelId(channelId);
    channelToggleOverrideByChannel = {
      ...channelToggleOverrideByChannel,
      [id]: value,
    };
  }

  function resolveChannelEnabledState(manifestPath, configuredManifestPath, override) {
    if (typeof override === 'boolean') {
      return override;
    }
    return isConfiguredManifest(manifestPath, configuredManifestPath);
  }

  let current = normalizeWasmState(payloadState);
  let currentRanges = normalizePolicyRanges(schemaState?.policy_ranges || {});
  let lastSchemaRef = schemaState;
  let lastPayloadRef = payloadState;
  let catalog = [];
  let catalogByChannel = createEmptyChannelCatalogMap();
  let selectedManifestPathByChannel = createEmptyChannelManifestMap();
  let configuredManifestPathByChannel = channelBindingMapFromSnapshot(current);
  let channelToggleOverrideByChannel = createEmptyChannelToggleOverrideMap();
  let busy = false;
  let statusTone = '';
  let statusMessage = '';
  let statusErrorCode = '';
  let initialCatalogRequested = false;
  let policyFallbackToBuiltin = current.fallback_to_builtin_click !== false;
  let policyOutputBufferBytes = resolvePolicyInputValue(
    current.configured_output_buffer_bytes,
    currentRanges.output_buffer_bytes,
  );
  let policyMaxCommands = resolvePolicyInputValue(
    current.configured_max_commands,
    currentRanges.max_commands,
  );
  let policyMaxExecutionMs = resolvePolicyInputValue(
    current.configured_max_execution_ms,
    currentRanges.max_execution_ms,
  );
  let activePluginTitle = current.active_plugin_name || current.active_plugin_id || text('wasm_text_no_active_plugin', 'Not loaded');
  let manifestPathDisplay = current.active_manifest_path || current.configured_manifest_path || '-';
  let showConfiguredManifestPath = false;
  let decorationForm = normalizeCursorDecoration(cursorDecoration);
  let lastCursorDecorationRef = cursorDecoration;
  let lastEmittedCursorDecoration = normalizeCursorDecoration(cursorDecoration);

  function setSelectedManifestPath(channelId, manifestPath) {
    const id = sanitizeChannelId(channelId);
    selectedManifestPathByChannel = {
      ...selectedManifestPathByChannel,
      [id]: `${manifestPath || ''}`.trim(),
    };
  }

  function findCatalogItemByManifestPath(path, channelId = '') {
    const expected = normalizeManifestPathForCompare(path);
    if (!expected) {
      return null;
    }
    const source = channelId ? (catalogByChannel[sanitizeChannelId(channelId)] || catalog) : catalog;
    for (const item of source) {
      if (normalizeManifestPathForCompare(item.manifest_path) === expected) {
        return item;
      }
    }
    return null;
  }

  function resolveBestManifestForChannel(channelId, preferredPath) {
    const id = sanitizeChannelId(channelId);
    const scopedCatalog = catalogByChannel[id] || catalog;
    if (scopedCatalog.length === 0) {
      return '';
    }
    const selectedMatch = findCatalogItemByManifestPath(preferredPath, id);
    if (selectedMatch) {
      return selectedMatch.manifest_path;
    }
    const activeMatch = findCatalogItemByManifestPath(activeManifestPathForChannel(current, id), id);
    if (activeMatch) {
      return activeMatch.manifest_path;
    }
    const configuredMatch = findCatalogItemByManifestPath(configuredManifestPathForChannel(current, id), id);
    if (configuredMatch) {
      return configuredMatch.manifest_path;
    }
    return scopedCatalog[0].manifest_path;
  }

  function syncChannelSelectionsFromCurrent() {
    if (catalog.length === 0) {
      return;
    }
    let changed = false;
    const next = { ...selectedManifestPathByChannel };
    for (const channelId of EFFECT_CHANNEL_IDS) {
      if (findCatalogItemByManifestPath(next[channelId], channelId)) {
        continue;
      }
      const resolved = resolveBestManifestForChannel(channelId, next[channelId]);
      if (resolved !== next[channelId]) {
        next[channelId] = resolved;
        changed = true;
      }
    }
    if (changed) {
      selectedManifestPathByChannel = next;
    }
  }

  function setCatalogFromResponse(response) {
    const previousSelected = { ...selectedManifestPathByChannel };
    catalog = normalizeCatalogItems(response?.plugins);
    catalogByChannel = buildChannelCatalogMap(catalog);
    const nextSelected = createEmptyChannelManifestMap();
    for (const channelId of EFFECT_CHANNEL_IDS) {
      nextSelected[channelId] = resolveBestManifestForChannel(channelId, previousSelected[channelId]);
    }
    selectedManifestPathByChannel = nextSelected;
  }

  function resolveActionError(response) {
    const errorCode = normalizeActionErrorCode(response?.error_code);
    const textValue = `${response?.error || ''}`.trim();
    const errorByCode = resolveWasmActionErrorMessage(errorCode, text);
    const message = errorByCode || textValue || text('wasm_action_failed', 'WASM action failed.');
    if (!errorCode) {
      return {
        message,
        errorCode: '',
      };
    }
    return {
      message,
      errorCode,
    };
  }

  async function runAction(action, payload) {
    if (busy) {
      return null;
    }
    if (typeof onAction !== 'function') {
      statusTone = 'error';
      statusMessage = text('wasm_action_not_ready', 'WASM action handler is not ready yet.');
      statusErrorCode = '';
      return { ok: false, error: statusMessage };
    }
    busy = true;
    statusTone = '';
    statusMessage = '';
    statusErrorCode = '';
    try {
      const response = await onAction(action, payload || {});
      if (action === 'catalog') {
        setCatalogFromResponse(response || {});
      }
      if (response?.cancelled === true) {
        statusTone = '';
        statusMessage = text('wasm_import_cancelled', 'Import cancelled.');
        statusErrorCode = '';
        return response;
      }
      if (!response || response.ok === false) {
        statusTone = 'error';
        const resolved = resolveActionError(response);
        statusMessage = resolved.message;
        statusErrorCode = resolved.errorCode;
        return response || { ok: false };
      }
      statusTone = 'ok';
      statusMessage = text('wasm_action_success', 'Operation completed.');
      statusErrorCode = '';
      return response;
    } catch (error) {
      statusTone = 'error';
      statusMessage = `${error?.message || error || text('wasm_action_failed', 'WASM action failed.')}`;
      statusErrorCode = '';
      return { ok: false, error: statusMessage };
    } finally {
      busy = false;
    }
  }

  async function requestCatalog(forceRefresh) {
    const response = await runAction('catalog', {
      force: !!forceRefresh,
      surface: 'effects',
    });
    if (!response || response.ok === false) {
      return;
    }
  }

  async function loadSelectedManifest(channelId) {
    const normalizedChannel = sanitizeChannelId(channelId);
    const manifestPath = `${selectedManifestPathByChannel[normalizedChannel] || ''}`.trim();
    if (!manifestPath) {
      statusTone = 'error';
      statusMessage = text('wasm_manifest_required', 'Please select a plugin manifest first.');
      statusErrorCode = '';
      return false;
    }
    const response = await runAction('loadManifest', {
      manifest_path: manifestPath,
      surface: 'effects',
      effect_channel: normalizedChannel,
    });
    if (!response || response.ok === false) {
      return false;
    }
    const activeManifestPath = `${response[`active_manifest_path_${normalizedChannel}`] || ''}`.trim() || manifestPath;
    current = normalizeWasmState(withConfiguredManifestPath(current, normalizedChannel, manifestPath));
    setConfiguredBinding(normalizedChannel, manifestPath);
    await requestCatalog(true);
    const match = findCatalogItemByManifestPath(activeManifestPath);
    if (match) {
      setSelectedManifestPath(normalizedChannel, match.manifest_path);
    }
    return true;
  }

  async function clearSelectedManifestPath(channelId) {
    const normalizedChannel = sanitizeChannelId(channelId);
    const payload = {};
    payload[channelPolicyKey(normalizedChannel)] = '';
    const response = await runAction('setPolicy', {
      ...payload,
    });
    if (!response || response.ok === false) {
      return false;
    }
    current = normalizeWasmState(withConfiguredManifestPath(current, normalizedChannel, ''));
    setConfiguredBinding(normalizedChannel, '');
    statusTone = 'ok';
    statusMessage = `${channelLabel(normalizedChannel)} ${text('status_wasm_manifest_cleared', 'plugin binding cleared.')}`;
    return true;
  }

  async function toggleChannelManifest(channelId) {
    const normalizedChannel = sanitizeChannelId(channelId);
    const manifestPath = `${selectedManifestPathByChannel[normalizedChannel] || ''}`.trim();
    const previousConfiguredPath = `${configuredManifestPathByChannel[normalizedChannel] || ''}`.trim();
    const previousOverride = channelToggleOverrideByChannel[normalizedChannel];
    if (!manifestPath) {
      statusTone = 'error';
      statusMessage = text('wasm_manifest_required', 'Please select a plugin manifest first.');
      statusErrorCode = '';
      return;
    }
    if (resolveChannelEnabledState(manifestPath, previousConfiguredPath, previousOverride)) {
      setChannelToggleOverride(normalizedChannel, false);
      setConfiguredBinding(normalizedChannel, '');
      const ok = await clearSelectedManifestPath(normalizedChannel);
      if (!ok) {
        setConfiguredBinding(normalizedChannel, previousConfiguredPath);
        setChannelToggleOverride(normalizedChannel, null);
        return;
      }
      setChannelToggleOverride(normalizedChannel, null);
      return;
    }
    setChannelToggleOverride(normalizedChannel, true);
    setConfiguredBinding(normalizedChannel, manifestPath);
    const ok = await loadSelectedManifest(normalizedChannel);
    if (!ok) {
      setConfiguredBinding(normalizedChannel, previousConfiguredPath);
      setChannelToggleOverride(normalizedChannel, null);
      return;
    }
    setChannelToggleOverride(normalizedChannel, null);
  }

  async function handleChannelManifestSelection(channelId, event) {
    setChannelToggleOverride(channelId, null);
    const value = `${event?.currentTarget?.value || ''}`.trim();
    setSelectedManifestPath(channelId, value);
  }

  async function savePolicy() {
    await runAction('setPolicy', {
      fallback_to_builtin_click: !!policyFallbackToBuiltin,
      output_buffer_bytes: clampIntByRange(policyOutputBufferBytes, currentRanges.output_buffer_bytes),
      max_commands: clampIntByRange(policyMaxCommands, currentRanges.max_commands),
      max_execution_ms: clampFloatByRange(policyMaxExecutionMs, currentRanges.max_execution_ms),
    });
  }

  async function toggleRuntimeEnabled() {
    await runAction(current.enabled ? 'disable' : 'enable');
  }

  async function resetPolicyToDefaults() {
    policyFallbackToBuiltin = true;
    policyOutputBufferBytes = currentRanges.output_buffer_bytes.defaultValue;
    policyMaxCommands = currentRanges.max_commands.defaultValue;
    policyMaxExecutionMs = currentRanges.max_execution_ms.defaultValue;
    await savePolicy();
  }

  function emitCursorDecorationIfNeeded() {
    const nextValue = normalizeCursorDecoration(decorationForm);
    if (sameCursorDecoration(lastEmittedCursorDecoration, nextValue)) {
      return;
    }
    lastEmittedCursorDecoration = nextValue;
    if (typeof onCursorDecorationChange === 'function') {
      onCursorDecorationChange(nextValue);
    }
  }

  function handleCursorDecorationSelection(event) {
    const selected = `${event?.currentTarget?.value || ''}`.trim();
    if (!selected || selected === '__disabled__') {
      decorationForm = {
        ...decorationForm,
        enabled: false,
      };
      emitCursorDecorationIfNeeded();
      return;
    }
    decorationForm = {
      ...decorationForm,
      enabled: true,
      plugin_id: selected,
    };
    emitCursorDecorationIfNeeded();
  }

  function toggleCursorDecorationEnabled() {
    decorationForm = {
      ...decorationForm,
      enabled: !decorationForm.enabled,
    };
    emitCursorDecorationIfNeeded();
  }

  $: if (schemaState !== lastSchemaRef) {
    lastSchemaRef = schemaState;
    currentRanges = normalizePolicyRanges(schemaState?.policy_ranges || {});
    policyOutputBufferBytes = resolvePolicyInputValue(policyOutputBufferBytes, currentRanges.output_buffer_bytes);
    policyMaxCommands = resolvePolicyInputValue(policyMaxCommands, currentRanges.max_commands);
    policyMaxExecutionMs = resolvePolicyInputValue(policyMaxExecutionMs, currentRanges.max_execution_ms);
  }

  $: if (payloadState !== lastPayloadRef) {
    lastPayloadRef = payloadState;
    current = normalizeWasmState(payloadState);
    configuredManifestPathByChannel = channelBindingMapFromSnapshot(current);
    channelToggleOverrideByChannel = createEmptyChannelToggleOverrideMap();
    policyFallbackToBuiltin = current.fallback_to_builtin_click !== false;
    policyOutputBufferBytes = resolvePolicyInputValue(
      current.configured_output_buffer_bytes,
      currentRanges.output_buffer_bytes,
    );
    policyMaxCommands = resolvePolicyInputValue(
      current.configured_max_commands,
      currentRanges.max_commands,
    );
    policyMaxExecutionMs = resolvePolicyInputValue(
      current.configured_max_execution_ms,
      currentRanges.max_execution_ms,
    );
    syncChannelSelectionsFromCurrent();
  }

  $: if (cursorDecoration !== lastCursorDecorationRef) {
    lastCursorDecorationRef = cursorDecoration;
    decorationForm = normalizeCursorDecoration(cursorDecoration);
    lastEmittedCursorDecoration = normalizeCursorDecoration(cursorDecoration);
  }

  $: decorationPluginOptions = buildCursorDecorationOptions(cursorDecorationOptions);
  $: decorationSelectedValue = decorationSelectValue(decorationForm);

  $: activePluginTitle = current.active_plugin_name || current.active_plugin_id || text('wasm_text_no_active_plugin', 'Not loaded');

  $: manifestPathDisplay =
    current.active_manifest_path_click
    || current.active_manifest_path
    || current.configured_manifest_path_click
    || current.configured_manifest_path
    || '-';

  $: showConfiguredManifestPath =
    !!(current.configured_manifest_path_click || current.configured_manifest_path)
    && (current.configured_manifest_path_click || current.configured_manifest_path)
      !== (current.active_manifest_path_click || current.active_manifest_path);

  $: if (!initialCatalogRequested && typeof onAction === 'function') {
    initialCatalogRequested = true;
    requestCatalog(false);
  }
</script>

<div class="wasm-panel">
  <section class="wasm-summary-strip">
    <article class="wasm-summary-item">
      <div class="wasm-summary-label" data-i18n="label_wasm_plugin_loaded">Plugin loaded</div>
      <div class="wasm-summary-value">
        <span class={`wasm-pill ${current.plugin_loaded ? 'is-on' : 'is-off'}`}>{boolText(current.plugin_loaded)}</span>
      </div>
    </article>
    <article class="wasm-summary-item">
      <div class="wasm-summary-label" data-i18n="label_wasm_runtime_backend">Runtime backend</div>
      <div class="wasm-summary-value">{current.runtime_backend || '-'}</div>
    </article>
    <article class="wasm-summary-item">
      <div class="wasm-summary-label" data-i18n="label_wasm_active_plugin">Active plugin</div>
      <div class="wasm-summary-value wasm-summary-clamp">{activePluginTitle}</div>
    </article>
    <article class="wasm-summary-item">
      <div class="wasm-summary-label" data-i18n="label_wasm_last_call_metrics">Last call metrics</div>
      <div class="wasm-summary-value wasm-summary-clamp">{renderCallMetricsText(current)}</div>
    </article>
  </section>

  <section class="wasm-block wasm-block--plugin-info">
    <header class="wasm-block-header">
      <h4 data-i18n="title_wasm_block_catalog">Plugin Info</h4>
      <p data-i18n="desc_wasm_block_catalog">
        Select plugin bindings for each effect channel. Import/export is managed from the unified plugin management section.
      </p>
    </header>

    <div class="wasm-catalog">
      {#each EFFECT_CHANNELS as channel}
        {@const channelCatalog = catalogByChannel[channel.id] || catalog}
        {@const selectedManifest = selectedManifestPathByChannel[channel.id] || ''}
        {@const configuredManifest = configuredManifestPathByChannel[channel.id] || ''}
        {@const channelToggleOverride = channelToggleOverrideByChannel[channel.id]}
        {@const isChannelEnabled = resolveChannelEnabledState(selectedManifest, configuredManifest, channelToggleOverride)}
        <div class="wasm-catalog-controls wasm-catalog-controls--channel">
          <span class="wasm-catalog-channel-label">{text(channel.labelKey, channel.fallback)}</span>
          <select
            value={selectedManifest}
            disabled={busy || channelCatalog.length === 0}
            on:change={(event) => handleChannelManifestSelection(channel.id, event)}
          >
            {#if channelCatalog.length === 0}
              <option value="">{text('text_wasm_catalog_empty', 'No plugins discovered.')}</option>
            {:else}
              {#each channelCatalog as plugin}
                <option value={plugin.manifest_path}>{pluginLabel(plugin)}</option>
              {/each}
            {/if}
          </select>
          <button
            type="button"
            class={`wasm-toggle wasm-channel-toggle ${isChannelEnabled ? 'is-on' : 'is-off'}`}
            on:click={() => toggleChannelManifest(channel.id)}
            disabled={busy || channelCatalog.length === 0 || !selectedManifest}
            data-i18n-title={isChannelEnabled ? 'tip_wasm_channel_disable' : 'tip_wasm_channel_enable'}
            title={isChannelEnabled
              ? text('tip_wasm_channel_disable', 'Disable plugin binding for this channel.')
              : text('tip_wasm_channel_enable', 'Enable selected plugin for this channel.')}
            aria-pressed={isChannelEnabled ? 'true' : 'false'}
          >
            <span class="wasm-toggle-track" aria-hidden="true">
              <span class="wasm-toggle-knob"></span>
            </span>
            <span
              class="wasm-toggle-text"
              data-i18n={isChannelEnabled ? 'text_wasm_channel_loaded' : 'text_wasm_channel_enable'}
            >
              {isChannelEnabled
                ? text('text_wasm_channel_loaded', 'Loaded')
                : text('text_wasm_channel_enable', 'Enable')}
            </span>
          </button>
        </div>
      {/each}

      <div class="wasm-catalog-controls wasm-catalog-controls--channel wasm-catalog-controls--cursor-decoration">
        <span class="wasm-catalog-channel-label" data-i18n="section_cursor_decoration">Cursor Decoration</span>
        <select
          value={decorationSelectedValue}
          on:change={handleCursorDecorationSelection}
        >
          {#each decorationPluginOptions as option}
            <option value={option.value}>{option.label}</option>
          {/each}
        </select>
        <button
          type="button"
          class={`wasm-toggle wasm-channel-toggle ${decorationForm.enabled ? 'is-on' : 'is-off'}`}
          on:click={toggleCursorDecorationEnabled}
          aria-pressed={decorationForm.enabled ? 'true' : 'false'}
        >
          <span class="wasm-toggle-track" aria-hidden="true">
            <span class="wasm-toggle-knob"></span>
          </span>
          <span
            class="wasm-toggle-text"
            data-i18n={decorationForm.enabled ? 'text_wasm_channel_loaded' : 'text_wasm_channel_enable'}
          >
            {decorationForm.enabled
              ? text('text_wasm_channel_loaded', 'Loaded')
              : text('text_wasm_channel_enable', 'Enable')}
          </span>
        </button>
      </div>

      <div class="wasm-cursor-decoration-detail">
        <div class="wasm-cursor-decoration-detail-grid">
          <label for="wasm_cursor_decoration_color" data-i18n="label_cursor_decoration_color">Accent color</label>
          <input
            id="wasm_cursor_decoration_color"
            type="color"
            bind:value={decorationForm.color_hex}
            disabled={!decorationForm.enabled}
            on:input={emitCursorDecorationIfNeeded}
            on:change={emitCursorDecorationIfNeeded}
          />

          <label for="wasm_cursor_decoration_size" data-i18n="label_cursor_decoration_size">Decoration size (px)</label>
          <input
            id="wasm_cursor_decoration_size"
            type="number"
            min="12"
            max="72"
            bind:value={decorationForm.size_px}
            disabled={!decorationForm.enabled}
            on:input={emitCursorDecorationIfNeeded}
            on:change={emitCursorDecorationIfNeeded}
          />

          <label for="wasm_cursor_decoration_alpha" data-i18n="label_cursor_decoration_alpha">Opacity (%)</label>
          <input
            id="wasm_cursor_decoration_alpha"
            type="number"
            min="15"
            max="100"
            bind:value={decorationForm.alpha_percent}
            disabled={!decorationForm.enabled}
            on:input={emitCursorDecorationIfNeeded}
            on:change={emitCursorDecorationIfNeeded}
          />
        </div>
      </div>
    </div>

    <details class="wasm-collapsible-block grid-offset-top">
      <summary class="wasm-collapsible-summary" data-i18n="label_wasm_runtime_info_toggle">
        Runtime plugin details
      </summary>
      <div class="grid wasm-grid">
        <div class="wasm-label" data-i18n="label_wasm_plugin_api_version">Plugin API version</div>
        <div class="wasm-value">{current.plugin_api_version || 0}</div>

        <div class="wasm-label" data-i18n="label_wasm_catalog_root_path">Catalog root path</div>
        <div class="wasm-value wasm-text-block">{current.configured_catalog_root_path || '-'}</div>

        <div class="wasm-label" data-i18n="label_wasm_manifest_path">Current plugin path</div>
        <div class={`wasm-value wasm-text-block ${showConfiguredManifestPath ? 'is-warn' : ''}`}>
          <span>{manifestPathDisplay}</span>
        </div>

        {#if showConfiguredManifestPath}
          <div class="wasm-label" data-i18n="label_wasm_configured_manifest_path">Plugin config path</div>
          <div class="wasm-value wasm-text-block">{current.configured_manifest_path_click || current.configured_manifest_path}</div>
        {/if}

        <div class="wasm-label" data-i18n="label_wasm_per_category_paths">Per-category plugin paths</div>
        <div class="wasm-value wasm-text-block">
          {#each EFFECT_CHANNELS as channel}
            <div>{text(channel.labelKey, channel.fallback)}: {configuredManifestPathForChannel(current, channel.id) || '-'}</div>
          {/each}
        </div>

        <div class="wasm-label" data-i18n="label_wasm_wasm_path">WASM path</div>
        <div class="wasm-value wasm-text-block">{current.active_wasm_path || '-'}</div>

        <div class="wasm-label" data-i18n="label_wasm_runtime_fallback">Runtime fallback reason</div>
        <div class="wasm-value wasm-text-block">{current.runtime_fallback_reason || '-'}</div>
      </div>
    </details>
  </section>

  <section class="wasm-block wasm-block--runtime-policy">
    <div class="wasm-policy-header-row">
      <header class="wasm-block-header">
        <h4 data-i18n="title_wasm_block_policy">Runtime Policy</h4>
        <p data-i18n="desc_wasm_block_policy">
          Control runtime enabling, fallback behavior, and budget limits.
        </p>
      </header>
      <div class="wasm-policy-top-actions">
        <button
          type="button"
          class={`wasm-toggle ${current.enabled ? 'is-on' : 'is-off'}`}
          on:click={toggleRuntimeEnabled}
          disabled={busy}
          data-i18n-title={current.enabled ? 'tip_wasm_toggle_disable' : 'tip_wasm_toggle_enable'}
          title={runtimeToggleTip(current.enabled)}
          aria-pressed={current.enabled ? 'true' : 'false'}
        >
          <span class="wasm-toggle-track" aria-hidden="true">
            <span class="wasm-toggle-knob"></span>
          </span>
          <span class="wasm-toggle-text" data-i18n={current.enabled ? 'text_wasm_runtime_on' : 'text_wasm_runtime_off'}>
            {runtimeToggleText(current.enabled)}
          </span>
        </button>
        <button
          type="button"
          class="btn-soft"
          on:click={() => runAction('reload')}
          disabled={busy || !current.plugin_loaded}
          data-i18n="btn_wasm_reload"
          data-i18n-title="tip_wasm_reload"
          title="Reload active plugin from disk."
        >
          Reload Plugin
        </button>
      </div>
    </div>

    <details class="wasm-collapsible-block">
      <summary class="wasm-collapsible-summary" data-i18n="label_wasm_policy_toggle">
        Show policy details
      </summary>
      <div class="grid wasm-grid">
      <div class="wasm-label" data-i18n="label_wasm_enabled">WASM plugin enabled</div>
      <div class="wasm-value">
        <span class={`wasm-pill ${current.enabled ? 'is-on' : 'is-off'}`}>{boolText(current.enabled)}</span>
      </div>

      <div class="wasm-label" data-i18n="label_wasm_config_enabled">Configured enabled</div>
      <div class="wasm-value">
        <span class={`wasm-pill ${current.configured_enabled ? 'is-on' : 'is-off'}`}>{boolText(current.configured_enabled)}</span>
      </div>

      <div class="wasm-label" data-i18n="label_wasm_fallback_to_builtin">Fallback to built-in click</div>
      <div class="wasm-actions">
        <label class="check-inline">
          <input type="checkbox" bind:checked={policyFallbackToBuiltin} disabled={busy} />
          <span data-i18n="label_wasm_fallback_to_builtin">Fallback to built-in click</span>
        </label>
        <button
          type="button"
          class="btn-soft"
          on:click={savePolicy}
          disabled={busy}
          data-i18n="btn_wasm_save_policy"
          data-i18n-title="tip_wasm_save_policy"
          title="Save fallback behavior and runtime budget."
        >
          Save Fallback & Budget
        </button>
        <button
          type="button"
          class="btn-soft"
          on:click={resetPolicyToDefaults}
          disabled={busy}
          data-i18n="btn_wasm_reset_policy"
          data-i18n-title="tip_wasm_reset_policy"
          title="Reset fallback and budget to defaults, then save."
        >
          Reset Defaults
        </button>
      </div>

      <div class="wasm-label" data-i18n="label_wasm_budget_output_buffer">Output buffer bytes</div>
      <div class="wasm-actions">
        <input
          type="number"
          min={currentRanges.output_buffer_bytes.min}
          max={currentRanges.output_buffer_bytes.max}
          step={currentRanges.output_buffer_bytes.step}
          bind:value={policyOutputBufferBytes}
          disabled={busy}
        />
      </div>

      <div class="wasm-label" data-i18n="label_wasm_budget_max_commands">Max commands</div>
      <div class="wasm-actions">
        <input
          type="number"
          min={currentRanges.max_commands.min}
          max={currentRanges.max_commands.max}
          step={currentRanges.max_commands.step}
          bind:value={policyMaxCommands}
          disabled={busy}
        />
      </div>

      <div class="wasm-label" data-i18n="label_wasm_budget_max_execution_ms">Max execution ms</div>
      <div class="wasm-actions">
        <input
          type="number"
          min={currentRanges.max_execution_ms.min}
          max={currentRanges.max_execution_ms.max}
          step={currentRanges.max_execution_ms.step}
          bind:value={policyMaxExecutionMs}
          disabled={busy}
        />
      </div>

      <div class="wasm-label" data-i18n="label_wasm_runtime_budget">Runtime budget</div>
      <div class="wasm-value wasm-text-block">{runtimeBudgetText(current)}</div>
      </div>
    </details>
  </section>

  <section class="wasm-block wasm-block--diagnostics">
    <header class="wasm-block-header">
      <h4 data-i18n="title_wasm_block_diagnostics">Diagnostics</h4>
      <p data-i18n="desc_wasm_block_diagnostics">
        Observe runtime performance, budget outcomes, and error states.
      </p>
    </header>

    <details class="wasm-collapsible-block">
      <summary class="wasm-collapsible-summary" data-i18n="label_wasm_diagnostics_toggle">
        Show diagnostics details
      </summary>
      <div class="grid wasm-grid">
        <div class="wasm-label" data-i18n="label_wasm_last_render_stats">Last render stats</div>
        <div class="wasm-value wasm-text-block">{renderStatsText(current)}</div>

        <div class="wasm-label" data-i18n="label_wasm_lifetime_invoke_stats">Lifetime invoke stats</div>
        <div class="wasm-value wasm-text-block">{renderLifetimeInvokeText(current)}</div>

        <div class="wasm-label" data-i18n="label_wasm_lifetime_render_stats">Lifetime render stats</div>
        <div class="wasm-value wasm-text-block">{renderLifetimeRenderText(current)}</div>

        <div class="wasm-label" data-i18n="label_wasm_throttled_capacity">Throttled by capacity</div>
        <div class="wasm-value">{current.last_throttled_by_capacity_render_commands}</div>

        <div class="wasm-label" data-i18n="label_wasm_throttled_interval">Throttled by interval</div>
        <div class="wasm-value">{current.last_throttled_by_interval_render_commands}</div>

        <div class="wasm-label" data-i18n="label_wasm_last_call_metrics">Last call metrics</div>
        <div class={`wasm-value wasm-text-block ${isDiagnosticWarning(current) ? 'is-warn' : ''}`}>
          {renderCallMetricsText(current)}
        </div>

        <div class="wasm-label" data-i18n="label_wasm_budget_flags">Budget flags</div>
        <div class={`wasm-value wasm-text-block ${isDiagnosticWarning(current) ? 'is-warn' : ''}`}>
          {renderBudgetFlagsText(current)}
        </div>

        <div class="wasm-label" data-i18n="label_wasm_budget_reason">Budget reason</div>
        <div class={`wasm-value wasm-text-block ${isDiagnosticWarning(current) ? 'is-warn' : ''}`}>
          {current.last_budget_reason || '-'}
        </div>

        <div class="wasm-label" data-i18n="label_wasm_parse_error">Parse error</div>
        <div class={`wasm-value wasm-text-block ${isDiagnosticWarning(current) ? 'is-warn' : ''}`}>
          {current.last_parse_error || '-'}
        </div>

        <div class="wasm-label" data-i18n="label_wasm_last_load_failure_stage">Last load failure stage</div>
        <div class={`wasm-value wasm-text-block ${isDiagnosticWarning(current) ? 'is-warn' : ''}`}>
          {current.last_load_failure_stage || '-'}
        </div>

        <div class="wasm-label" data-i18n="label_wasm_last_load_failure_code">Last load failure code</div>
        <div class={`wasm-value wasm-text-block ${isDiagnosticWarning(current) ? 'is-warn' : ''}`}>
          {current.last_load_failure_code || '-'}
        </div>

        <div class="wasm-label" data-i18n="label_wasm_last_render_error">Last render error</div>
        <div class="wasm-value wasm-text-block">{current.last_render_error || '-'}</div>

        <div class="wasm-label" data-i18n="label_wasm_last_error">Last host error</div>
        <div class="wasm-value wasm-text-block">{current.last_error || '-'}</div>
      </div>
    </details>
  </section>
</div>

<style>
  .wasm-catalog-controls--cursor-decoration {
    margin-top: 6px;
  }

  .wasm-cursor-decoration-detail {
    padding: 8px 0 4px 124px;
  }

  .wasm-cursor-decoration-detail-grid {
    display: grid;
    grid-template-columns: max-content minmax(180px, 1fr);
    gap: 10px 14px;
    align-items: center;
  }

  .wasm-cursor-decoration-detail-grid label {
    color: rgba(46, 68, 98, 0.88);
    font-size: 13px;
    font-weight: 600;
  }
</style>
