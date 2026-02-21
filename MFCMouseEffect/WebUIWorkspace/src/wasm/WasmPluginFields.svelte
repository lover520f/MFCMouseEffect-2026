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
    hasWasmDiagnosticWarning,
    normalizeWasmDiagnostics,
  } from './diagnostics-model.js';

  export let schemaState = {};
  export let payloadState = {};
  export let i18n = {};
  export let onAction = null;

  function text(key, fallback) {
    const value = i18n || {};
    return value[key] || fallback;
  }

  function normalizeState(input) {
    const value = input || {};
    const diagnostics = normalizeWasmDiagnostics(value);
    return {
      enabled: !!value.enabled,
      configured_enabled: !!value.configured_enabled,
      fallback_to_builtin_click: value.fallback_to_builtin_click !== false,
      configured_manifest_path: `${value.configured_manifest_path || ''}`.trim(),
      configured_catalog_root_path: `${value.configured_catalog_root_path || ''}`.trim(),
      configured_output_buffer_bytes: Number(value.configured_output_buffer_bytes) || 0,
      configured_max_commands: Number(value.configured_max_commands) || 0,
      configured_max_execution_ms: Number(value.configured_max_execution_ms) || 0,
      runtime_output_buffer_bytes: Number(value.runtime_output_buffer_bytes) || 0,
      runtime_max_commands: Number(value.runtime_max_commands) || 0,
      runtime_max_execution_ms: Number(value.runtime_max_execution_ms) || 0,
      last_call_duration_us: diagnostics.last_call_duration_us,
      last_output_bytes: diagnostics.last_output_bytes,
      last_command_count: diagnostics.last_command_count,
      last_call_exceeded_budget: diagnostics.last_call_exceeded_budget,
      last_call_rejected_by_budget: diagnostics.last_call_rejected_by_budget,
      last_output_truncated_by_budget: diagnostics.last_output_truncated_by_budget,
      last_command_truncated_by_budget: diagnostics.last_command_truncated_by_budget,
      last_budget_reason: diagnostics.last_budget_reason,
      last_parse_error: diagnostics.last_parse_error,
      runtime_backend: `${value.runtime_backend || ''}`.trim(),
      runtime_fallback_reason: `${value.runtime_fallback_reason || ''}`.trim(),
      plugin_loaded: !!value.plugin_loaded,
      plugin_api_version: Number(value.plugin_api_version) || 0,
      active_plugin_id: `${value.active_plugin_id || ''}`.trim(),
      active_plugin_name: `${value.active_plugin_name || ''}`.trim(),
      active_manifest_path: `${value.active_manifest_path || ''}`.trim(),
      active_wasm_path: `${value.active_wasm_path || ''}`.trim(),
      last_rendered_by_wasm: !!value.last_rendered_by_wasm,
      last_executed_text_commands: Number(value.last_executed_text_commands) || 0,
      last_executed_image_commands: Number(value.last_executed_image_commands) || 0,
      last_dropped_render_commands: Number(value.last_dropped_render_commands) || 0,
      last_error: `${value.last_error || ''}`.trim(),
      last_render_error: `${value.last_render_error || ''}`.trim(),
    };
  }

  function normalizeCatalogItems(input) {
    const source = Array.isArray(input) ? input : [];
    const out = [];
    for (const item of source) {
      const value = item || {};
      const manifestPath = `${value.manifest_path || ''}`.trim();
      if (!manifestPath) {
        continue;
      }
      out.push({
        id: `${value.id || ''}`.trim(),
        name: `${value.name || ''}`.trim(),
        version: `${value.version || ''}`.trim(),
        api_version: Number(value.api_version) || 0,
        manifest_path: manifestPath,
        wasm_path: `${value.wasm_path || ''}`.trim(),
      });
    }
    return out;
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
      return title;
    }
    return `${title} (${version})`;
  }

  function normalizeManifestPathForCompare(path) {
    const textValue = `${path || ''}`.trim();
    if (!textValue) {
      return '';
    }
    return textValue.replace(/\\/g, '/').replace(/\/+/g, '/').toLowerCase();
  }

  function renderStatsText(snapshot) {
    const s = snapshot || normalizeState({});
    return `${text('label_wasm_last_rendered', 'Rendered by WASM')}: ${boolText(s.last_rendered_by_wasm)}, `
      + `text=${s.last_executed_text_commands}, image=${s.last_executed_image_commands}, `
      + `${text('label_wasm_dropped_commands', 'Dropped commands')}=${s.last_dropped_render_commands}`;
  }

  function renderCallMetricsText(snapshot) {
    const s = snapshot || normalizeState({});
    return buildWasmCallMetricsText(s, text);
  }

  function renderBudgetFlagsText(snapshot) {
    const s = snapshot || normalizeState({});
    return buildWasmBudgetFlagsText(s, text);
  }

  function isDiagnosticWarning(snapshot) {
    const s = snapshot || normalizeState({});
    return hasWasmDiagnosticWarning(s);
  }

  function runtimeBudgetText(snapshot) {
    const s = snapshot || normalizeState({});
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

  let current = normalizeState(payloadState);
  let currentRanges = normalizePolicyRanges(schemaState?.policy_ranges || {});
  let lastSchemaRef = schemaState;
  let lastPayloadRef = payloadState;
  let catalog = [];
  let catalogErrors = [];
  let catalogSearchRoots = [];
  let selectedManifestPath = '';
  let busy = false;
  let statusTone = '';
  let statusMessage = '';
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

  function findCatalogItemByManifestPath(path) {
    const expected = normalizeManifestPathForCompare(path);
    if (!expected) {
      return null;
    }
    for (const item of catalog) {
      if (normalizeManifestPathForCompare(item.manifest_path) === expected) {
        return item;
      }
    }
    return null;
  }

  function setCatalogFromResponse(response) {
    const previousSelected = selectedManifestPath;
    catalog = normalizeCatalogItems(response?.plugins);
    catalogErrors = normalizeCatalogErrors(response?.errors);
    catalogSearchRoots = normalizeCatalogRoots(response?.search_roots);
    const selectedMatch = findCatalogItemByManifestPath(previousSelected);
    if (selectedMatch) {
      selectedManifestPath = selectedMatch.manifest_path;
      return;
    }
    const activeMatch = findCatalogItemByManifestPath(current.active_manifest_path);
    if (activeMatch) {
      selectedManifestPath = activeMatch.manifest_path;
      return;
    }
    selectedManifestPath = catalog.length > 0 ? catalog[0].manifest_path : '';
  }

  function resolveActionError(response) {
    const textValue = `${response?.error || ''}`.trim();
    return textValue || text('wasm_action_failed', 'WASM action failed.');
  }

  async function runAction(action, payload) {
    if (busy) {
      return null;
    }
    if (typeof onAction !== 'function') {
      statusTone = 'error';
      statusMessage = text('wasm_action_not_ready', 'WASM action handler is not ready yet.');
      return { ok: false, error: statusMessage };
    }
    busy = true;
    statusTone = '';
    statusMessage = '';
    try {
      const response = await onAction(action, payload || {});
      if (action === 'catalog') {
        setCatalogFromResponse(response || {});
      }
      if (response?.cancelled === true) {
        statusTone = '';
        statusMessage = text('wasm_import_cancelled', 'Import cancelled.');
        return response;
      }
      if (!response || response.ok === false) {
        statusTone = 'error';
        statusMessage = resolveActionError(response);
        return response || { ok: false };
      }
      statusTone = 'ok';
      statusMessage = text('wasm_action_success', 'Operation completed.');
      return response;
    } catch (error) {
      statusTone = 'error';
      statusMessage = `${error?.message || error || text('wasm_action_failed', 'WASM action failed.')}`;
      return { ok: false, error: statusMessage };
    } finally {
      busy = false;
    }
  }

  async function requestCatalog(forceRefresh) {
    const response = await runAction('catalog', { force: !!forceRefresh });
    if (!response || response.ok === false) {
      return;
    }
  }

  async function loadSelectedManifest() {
    if (!selectedManifestPath) {
      statusTone = 'error';
      statusMessage = text('wasm_manifest_required', 'Please select a plugin manifest first.');
      return;
    }
    const response = await runAction('loadManifest', {
      manifest_path: selectedManifestPath,
    });
    if (!response || response.ok === false) {
      return;
    }
    const activeManifestPath = `${response.active_manifest_path || ''}`.trim();
    await requestCatalog(true);
    const match = findCatalogItemByManifestPath(activeManifestPath || selectedManifestPath);
    if (match) {
      selectedManifestPath = match.manifest_path;
    }
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
      selectedManifestPath = importedManifestPath;
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
    current = normalizeState(payloadState);
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
  }

  $: activePluginTitle = current.active_plugin_name || current.active_plugin_id || text('wasm_text_no_active_plugin', 'Not loaded');

  $: manifestPathDisplay = current.active_manifest_path || current.configured_manifest_path || '-';

  $: showConfiguredManifestPath = !!current.configured_manifest_path
    && current.configured_manifest_path !== current.active_manifest_path;

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
      <div class="wasm-catalog-controls">
        <select bind:value={selectedManifestPath} disabled={busy || catalog.length === 0}>
          {#if catalog.length === 0}
            <option value="">{text('text_wasm_catalog_empty', 'No plugins discovered.')}</option>
          {:else}
            {#each catalog as plugin}
              <option value={plugin.manifest_path}>{pluginLabel(plugin)}</option>
            {/each}
          {/if}
        </select>
        <button
          type="button"
          class="btn-soft"
          on:click={loadSelectedManifest}
          disabled={busy || !selectedManifestPath}
          data-i18n="btn_wasm_load_selected"
          data-i18n-title="tip_wasm_load_selected"
          title="Load selected plugin into runtime."
        >
          Load Selected
        </button>
      </div>
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
        <div class="wasm-value wasm-text-block">{current.configured_manifest_path}</div>
      {/if}

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

      <div class="wasm-label" data-i18n="label_wasm_last_render_error">Last render error</div>
      <div class="wasm-value wasm-text-block">{current.last_render_error || '-'}</div>

      <div class="wasm-label" data-i18n="label_wasm_last_error">Last host error</div>
      <div class="wasm-value wasm-text-block">{current.last_error || '-'}</div>

      {#if statusMessage}
        <div class="wasm-label">{text('label_wasm_operation_result', 'Operation result')}</div>
        <div class={`wasm-value wasm-text-block ${statusTone === 'error' ? 'is-error' : 'is-ok'}`}>{statusMessage}</div>
      {/if}
    </div>
  </section>
</div>
