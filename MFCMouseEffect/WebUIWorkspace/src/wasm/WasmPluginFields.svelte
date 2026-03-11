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
  export let i18n = {};
  export let onAction = null;

  function text(key, fallback) {
    const value = i18n || {};
    return value[key] || fallback;
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

  function normalizeCatalogErrors(input) {
    const source = Array.isArray(input) ? input : [];
    const out = [];
    for (const item of source) {
      const text = `${item || ''}`.trim();
      if (!text) {
        continue;
      }
      out.push(text);
    }
    return out;
  }

  function normalizeCatalogRoots(input) {
    const source = Array.isArray(input) ? input : [];
    const out = [];
    for (const item of source) {
      const text = `${item || ''}`.trim();
      if (!text) {
        continue;
      }
      out.push(text);
    }
    return out;
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

  function renderStatsText(snapshot) {
    const s = snapshot || normalizeWasmState({});
      return `${text('label_wasm_last_rendered', 'Rendered by WASM')}: ${boolText(s.last_rendered_by_wasm)}, `
      + `text=${s.last_executed_text_commands}, image=${s.last_executed_image_commands}, `
      + `pulse=${s.last_executed_pulse_commands}, polyline=${s.last_executed_polyline_commands}, pathStroke=${s.last_executed_path_stroke_commands}, pathFill=${s.last_executed_path_fill_commands}, glow=${s.last_executed_glow_batch_commands}, sprite=${s.last_executed_sprite_batch_commands}, `
      + `glowEmitter=${s.last_executed_glow_emitter_commands}, spriteEmitter=${s.last_executed_sprite_emitter_commands}, `
      + `${text('label_wasm_throttled_commands', 'Throttled')}=${s.last_throttled_render_commands} `
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

  let current = normalizeWasmState(payloadState);
  let currentRanges = normalizePolicyRanges(schemaState?.policy_ranges || {});
  let lastSchemaRef = schemaState;
  let lastPayloadRef = payloadState;
  let catalog = [];
  let catalogByChannel = createEmptyChannelCatalogMap();
  let catalogErrors = [];
  let catalogSearchRoots = [];
  let selectedManifestPathByChannel = createEmptyChannelManifestMap();
  let busy = false;
  let statusTone = '';
  let statusMessage = '';
  let statusErrorCode = '';
  let initialCatalogRequested = false;
  let policyFallbackToBuiltin = current.fallback_to_builtin_click !== false;
  let policyCatalogRootPath = current.configured_catalog_root_path || '';
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
    catalogErrors = normalizeCatalogErrors(response?.errors);
    catalogSearchRoots = normalizeCatalogRoots(response?.search_roots);
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
    const label = text('label_wasm_error_code', 'Error code');
    if (!errorCode) {
      return {
        message,
        errorCode: '',
      };
    }
    return {
      message: `${message} (${label}: ${errorCode})`,
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
      return;
    }
    const response = await runAction('loadManifest', {
      manifest_path: manifestPath,
      surface: 'effects',
      effect_channel: normalizedChannel,
    });
    if (!response || response.ok === false) {
      return;
    }
    const activeManifestPath = `${response[`active_manifest_path_${normalizedChannel}`] || ''}`.trim() || manifestPath;
    await requestCatalog(true);
    const match = findCatalogItemByManifestPath(activeManifestPath);
    if (match) {
      setSelectedManifestPath(normalizedChannel, match.manifest_path);
    }
  }

  async function persistSelectedManifestPath(channelId) {
    const normalizedChannel = sanitizeChannelId(channelId);
    const manifestPath = `${selectedManifestPathByChannel[normalizedChannel] || ''}`.trim();
    if (!manifestPath) {
      return;
    }
    const payload = {};
    payload[channelPolicyKey(normalizedChannel)] = manifestPath;
    const response = await runAction('setPolicy', {
      ...payload,
    });
    if (!response || response.ok === false) {
      return;
    }
    statusTone = 'ok';
    statusMessage = `${channelLabel(normalizedChannel)} ${text('status_wasm_manifest_selected_prefix', 'Selected plugin saved: ')}${manifestPath}`;
  }

  async function handleChannelManifestSelection(channelId, event) {
    const value = `${event?.currentTarget?.value || ''}`.trim();
    setSelectedManifestPath(channelId, value);
    await persistSelectedManifestPath(channelId);
  }

  async function importFromFolderDialog() {
    const response = await runAction('importFromFolderDialog', {
      initial_path: policyCatalogRootPath || '',
    });
    if (response?.cancelled === true) {
      return;
    }
    if (!response || response.ok === false) {
      return;
    }
    await requestCatalog(true);
    const importedManifestPath = `${response.manifest_path || ''}`.trim();
    if (importedManifestPath) {
      setSelectedManifestPath('click', importedManifestPath);
      statusTone = 'ok';
      statusMessage = `${text('wasm_import_success_prefix', 'Imported to primary root: ')}${importedManifestPath}`;
    }
  }

  async function exportAllPlugins() {
    const response = await runAction('exportAll', {});
    if (!response || response.ok === false) {
      return;
    }
    const exportPath = `${response.export_path || ''}`.trim();
    if (!exportPath) {
      return;
    }
    statusTone = 'ok';
    statusMessage = `${text('wasm_export_success_prefix', 'Exported all plugins to: ')}${exportPath}`;
  }

  async function saveCatalogRootPath() {
    const catalogRootPath = `${policyCatalogRootPath || ''}`.trim();
    const response = await runAction('setPolicy', {
      catalog_root_path: catalogRootPath,
    });
    if (!response || response.ok === false) {
      return;
    }
    await requestCatalog(true);
    if (catalogRootPath) {
      statusTone = 'ok';
      statusMessage = `${text('wasm_catalog_root_saved_prefix', 'Catalog root updated: ')}${catalogRootPath}`;
      return;
    }
    statusTone = 'ok';
    statusMessage = text('wasm_catalog_root_cleared', 'Catalog root cleared. Using default scan roots.');
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
    policyFallbackToBuiltin = current.fallback_to_builtin_click !== false;
    policyCatalogRootPath = current.configured_catalog_root_path || '';
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

  <section class="wasm-block">
    <header class="wasm-block-header">
      <h4 data-i18n="title_wasm_block_catalog">Plugin Info</h4>
      <p data-i18n="desc_wasm_block_catalog">
        Browse discovered plugins, import plugin folders, and load the selected plugin into runtime.
      </p>
    </header>

    <div class="wasm-catalog">
      <div class="wasm-catalog-controls">
        <input
          type="text"
          bind:value={policyCatalogRootPath}
          disabled={busy}
          placeholder={text('placeholder_wasm_catalog_root_path', 'Optional: custom plugin scan directory')}
        />
        <button
          type="button"
          class="btn-soft"
          on:click={saveCatalogRootPath}
          disabled={busy}
          data-i18n="btn_wasm_save_catalog_root"
          data-i18n-title="tip_wasm_save_catalog_root"
          title="Persist scan root path and refresh discovery."
        >
          Apply Scan Path
        </button>
      </div>
      <div class="wasm-catalog-controls">
        <button
          type="button"
          class="btn-soft"
          on:click={() => requestCatalog(true)}
          disabled={busy}
          data-i18n="btn_wasm_refresh_catalog"
          data-i18n-title="tip_wasm_refresh_catalog"
          title="Rescan plugin folders and update list."
        >
          Refresh Plugin List
        </button>
      </div>
      {#each EFFECT_CHANNELS as channel}
        {@const channelCatalog = catalogByChannel[channel.id] || catalog}
        <div class="wasm-catalog-controls wasm-catalog-controls--channel">
          <span class="wasm-catalog-channel-label">{text(channel.labelKey, channel.fallback)}</span>
          <select
            value={selectedManifestPathByChannel[channel.id] || ''}
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
            class="btn-soft"
            on:click={() => loadSelectedManifest(channel.id)}
            disabled={busy || channelCatalog.length === 0 || !(selectedManifestPathByChannel[channel.id] || '')}
            data-i18n="btn_wasm_load_selected"
            data-i18n-title="tip_wasm_load_selected"
            title="Load selected plugin into runtime."
          >
            Load Selected
          </button>
        </div>
      {/each}
      <div class="wasm-catalog-controls">
        <button
          type="button"
          class="btn-soft"
          on:click={importFromFolderDialog}
          disabled={busy}
          data-i18n="btn_wasm_import_folder"
          data-i18n-title="tip_wasm_import_folder"
          title="Import plugin folder into primary root."
        >
          Import Plugin Folder
        </button>
        <button
          type="button"
          class="btn-soft"
          on:click={exportAllPlugins}
          disabled={busy}
          data-i18n="btn_wasm_export_all"
          data-i18n-title="tip_wasm_export_all"
          title="Export all discovered plugins."
        >
          Export All
        </button>
      </div>
      {#if catalogSearchRoots.length > 0}
        <div class="hint wasm-catalog-errors">
          <strong data-i18n="label_wasm_catalog_roots">Plugin scan roots:</strong>
          <span>{catalogSearchRoots[0]}</span>
          {#if catalogSearchRoots.length > 1}
            <span> (+{catalogSearchRoots.length - 1})</span>
          {/if}
        </div>
      {/if}
      {#if catalogErrors.length > 0}
        <div class="hint wasm-catalog-errors">
          <strong data-i18n="label_wasm_catalog_errors">Plugin scan errors:</strong>
          <span>{catalogErrors[0]}</span>
          {#if catalogErrors.length > 1}
            <span> (+{catalogErrors.length - 1})</span>
          {/if}
        </div>
      {/if}
    </div>

    <div class="grid wasm-grid grid-offset-top">
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

      <div class="wasm-label">Per-category plugin paths</div>
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
  </section>

  <section class="wasm-block">
    <header class="wasm-block-header">
      <h4 data-i18n="title_wasm_block_policy">Runtime Policy</h4>
      <p data-i18n="desc_wasm_block_policy">
        Control runtime enabling, fallback behavior, and budget limits.
      </p>
    </header>

    <div class="grid wasm-grid">
      <div class="wasm-label" data-i18n="label_wasm_controls">Runtime controls</div>
      <div class="wasm-actions">
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
  </section>

  <section class="wasm-block">
    <header class="wasm-block-header">
      <h4 data-i18n="title_wasm_block_diagnostics">Diagnostics</h4>
      <p data-i18n="desc_wasm_block_diagnostics">
        Observe runtime performance, budget outcomes, and error states.
      </p>
    </header>

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

      {#if statusMessage}
        <div class="wasm-label">{text('label_wasm_operation_result', 'Operation result')}</div>
        <div class={`wasm-value wasm-text-block ${statusTone === 'error' ? 'is-error' : 'is-ok'}`}>{statusMessage}</div>
        {#if statusTone === 'error' && statusErrorCode}
          <div class="wasm-label" data-i18n="label_wasm_error_code">Error code</div>
          <div class="wasm-value wasm-text-block is-error">{statusErrorCode}</div>
        {/if}
      {/if}
    </div>
  </section>
</div>
