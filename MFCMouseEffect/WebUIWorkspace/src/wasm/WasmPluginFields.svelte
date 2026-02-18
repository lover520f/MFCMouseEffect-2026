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
  export let onAction = async () => ({ ok: false, error: 'wasm action handler unavailable' });

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

  let current = normalizeState(payloadState);
  let currentRanges = normalizePolicyRanges(schemaState?.policy_ranges || {});
  let lastSchemaRef = schemaState;
  let lastPayloadRef = payloadState;
  let catalog = [];
  let catalogErrors = [];
  let selectedManifestPath = '';
  let busy = false;
  let statusTone = '';
  let statusMessage = '';
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

  function setCatalogFromResponse(response) {
    catalog = normalizeCatalogItems(response?.plugins);
    catalogErrors = normalizeCatalogErrors(response?.errors);
    if (!selectedManifestPath || !catalog.some((item) => item.manifest_path === selectedManifestPath)) {
      selectedManifestPath = catalog.length > 0 ? catalog[0].manifest_path : '';
    }
  }

  function resolveActionError(response) {
    const textValue = `${response?.error || ''}`.trim();
    return textValue || text('wasm_action_failed', 'WASM action failed.');
  }

  async function runAction(action, payload) {
    if (busy || typeof onAction !== 'function') {
      return null;
    }
    busy = true;
    statusTone = '';
    statusMessage = '';
    try {
      const response = await onAction(action, payload || {});
      if (action === 'catalog') {
        setCatalogFromResponse(response || {});
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
    await runAction('loadManifest', {
      manifest_path: selectedManifestPath,
    });
  }

  async function savePolicy() {
    await runAction('setPolicy', {
      fallback_to_builtin_click: !!policyFallbackToBuiltin,
      output_buffer_bytes: clampIntByRange(policyOutputBufferBytes, currentRanges.output_buffer_bytes),
      max_commands: clampIntByRange(policyMaxCommands, currentRanges.max_commands),
      max_execution_ms: clampFloatByRange(policyMaxExecutionMs, currentRanges.max_execution_ms),
    });
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

  $: if (!initialCatalogRequested && typeof onAction === 'function') {
    initialCatalogRequested = true;
    requestCatalog(false);
  }
</script>

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
    >
      Save Policy
    </button>
    <button
      type="button"
      class="btn-soft"
      on:click={resetPolicyToDefaults}
      disabled={busy}
      data-i18n="btn_wasm_reset_policy"
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

  <div class="wasm-label" data-i18n="label_wasm_plugin_loaded">Plugin loaded</div>
  <div class="wasm-value">
    <span class={`wasm-pill ${current.plugin_loaded ? 'is-on' : 'is-off'}`}>{boolText(current.plugin_loaded)}</span>
  </div>

  <div class="wasm-label" data-i18n="label_wasm_runtime_backend">Runtime backend</div>
  <div class="wasm-value">{current.runtime_backend || '-'}</div>

  <div class="wasm-label" data-i18n="label_wasm_runtime_fallback">Runtime fallback reason</div>
  <div class="wasm-value wasm-text-block">{current.runtime_fallback_reason || '-'}</div>

  <div class="wasm-label" data-i18n="label_wasm_active_plugin">Active plugin</div>
  <div class="wasm-value wasm-text-block">
    {current.active_plugin_name || current.active_plugin_id || text('wasm_text_no_active_plugin', 'Not loaded')}
  </div>

  <div class="wasm-label" data-i18n="label_wasm_plugin_api_version">Plugin API version</div>
  <div class="wasm-value">{current.plugin_api_version || 0}</div>

  <div class="wasm-label" data-i18n="label_wasm_manifest_path">Manifest path</div>
  <div class="wasm-value wasm-text-block">{current.active_manifest_path || '-'}</div>

  <div class="wasm-label" data-i18n="label_wasm_configured_manifest_path">Configured manifest path</div>
  <div class="wasm-value wasm-text-block">{current.configured_manifest_path || '-'}</div>

  <div class="wasm-label" data-i18n="label_wasm_wasm_path">WASM path</div>
  <div class="wasm-value wasm-text-block">{current.active_wasm_path || '-'}</div>

  <div class="wasm-label" data-i18n="label_wasm_runtime_budget">Runtime budget</div>
  <div class="wasm-value wasm-text-block">
    buffer={current.runtime_output_buffer_bytes || '-'}, commands={current.runtime_max_commands || '-'}, exec={current.runtime_max_execution_ms || '-'}ms
  </div>

  <div class="wasm-label" data-i18n="label_wasm_catalog">Plugin catalog</div>
  <div class="wasm-catalog">
    <div class="wasm-catalog-controls">
      <button
        type="button"
        class="btn-soft"
        on:click={() => requestCatalog(true)}
        disabled={busy}
        data-i18n="btn_wasm_refresh_catalog"
      >
        Refresh Catalog
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
      >
        Load Selected
      </button>
    </div>
    {#if catalogErrors.length > 0}
      <div class="hint wasm-catalog-errors">
        <strong data-i18n="label_wasm_catalog_errors">Catalog errors:</strong>
        <span>{catalogErrors[0]}</span>
        {#if catalogErrors.length > 1}
          <span> (+{catalogErrors.length - 1})</span>
        {/if}
      </div>
    {/if}
  </div>

  <div class="wasm-label" data-i18n="label_wasm_controls">Runtime controls</div>
  <div class="wasm-actions">
    <button type="button" class="btn-soft" on:click={() => runAction('enable')} disabled={busy} data-i18n="btn_wasm_enable">
      Enable
    </button>
    <button type="button" class="btn-soft" on:click={() => runAction('disable')} disabled={busy} data-i18n="btn_wasm_disable">
      Disable
    </button>
    <button type="button" class="btn-soft" on:click={() => runAction('reload')} disabled={busy} data-i18n="btn_wasm_reload">
      Reload Plugin
    </button>
  </div>

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
    <div class="wasm-label" data-i18n="label_wasm_operation_result">Operation result</div>
    <div class={`wasm-value wasm-text-block ${statusTone === 'error' ? 'is-error' : 'is-ok'}`}>{statusMessage}</div>
  {/if}
</div>
