<script>
  import { createEventDispatcher } from 'svelte';

  export let pluginEnabled = false;
  export let fallbackToNative = true;
  export let manifestPath = '';
  export let wasmState = {};
  export let texts = {};
  export let onAction = null;

  const dispatch = createEventDispatcher();

  function text(key, fallback) {
    const value = texts || {};
    return value[key] || fallback;
  }

  function normalizeCatalogItems(input) {
    const source = Array.isArray(input) ? input : [];
    const out = [];
    for (const item of source) {
      const value = item || {};
      const manifestPath = `${value.manifest_path || ''}`.trim();
      if (!manifestPath || !supportsIndicatorKinds(value.input_kinds, value.surfaces, value.has_explicit_surfaces)) {
        continue;
      }
      out.push({
        id: `${value.id || ''}`.trim(),
        name: `${value.name || ''}`.trim(),
        version: `${value.version || ''}`.trim(),
        surfaces: Array.isArray(value.surfaces)
          ? value.surfaces.map((entry) => `${entry || ''}`.trim()).filter((entry) => entry.length > 0)
          : [],
        has_explicit_surfaces: !!value.has_explicit_surfaces,
        input_kinds: Array.isArray(value.input_kinds)
          ? value.input_kinds.map((entry) => `${entry || ''}`.trim()).filter((entry) => entry.length > 0)
          : [],
        manifest_path: manifestPath,
      });
    }
    return out;
  }

  function normalizeCatalogErrors(input) {
    const source = Array.isArray(input) ? input : [];
    const out = [];
    for (const item of source) {
      const value = `${item || ''}`.trim();
      if (!value) {
        continue;
      }
      out.push(value);
    }
    return out;
  }

  function supportsIndicatorKinds(inputKinds, surfaces, hasExplicitSurfaces) {
    const normalizedSurfaces = Array.isArray(surfaces)
      ? surfaces.map((entry) => `${entry || ''}`.trim()).filter((entry) => entry.length > 0)
      : [];
    if (hasExplicitSurfaces) {
      return normalizedSurfaces.includes('indicator');
    }
    const kinds = Array.isArray(inputKinds)
      ? inputKinds.map((entry) => `${entry || ''}`.trim()).filter((entry) => entry.length > 0)
      : [];
    if (kinds.length === 0) {
      return true;
    }
    return kinds.some((entry) => entry.startsWith('indicator_'));
  }

  function normalizeManifestPathForCompare(path) {
    const value = `${path || ''}`.trim();
    if (!value) {
      return '';
    }
    return value.replace(/\\/g, '/').replace(/\/+/g, '/').toLowerCase();
  }

  function findCatalogItemByManifestPath(items, manifestPath) {
    const expected = normalizeManifestPathForCompare(manifestPath);
    if (!expected) {
      return null;
    }
    for (const item of items || []) {
      if (normalizeManifestPathForCompare(item.manifest_path) === expected) {
        return item;
      }
    }
    return null;
  }

  function pluginLabel(plugin) {
    const title = plugin?.name || plugin?.id || text('wasm_text_unknown_plugin', 'Unknown plugin');
    if (!plugin?.version) {
      return title;
    }
    return `${title} (${plugin.version})`;
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
      return await onAction(action, payload || {});
    } catch (error) {
      const message = error instanceof Error ? error.message : `${error || ''}`.trim();
      statusTone = 'error';
      statusMessage = message || text('wasm_action_failed', 'WASM action failed.');
      return { ok: false, error: statusMessage };
    } finally {
      busy = false;
    }
  }

  function selectBestManifest(items) {
    if (findCatalogItemByManifestPath(items, selectedManifestPath)) {
      return;
    }
    const selectedMatch = findCatalogItemByManifestPath(items, manifestPath);
    if (selectedMatch) {
      selectedManifestPath = selectedMatch.manifest_path;
      return;
    }
    const activeMatch = findCatalogItemByManifestPath(
      items,
      wasmState?.indicator_active_manifest_path || wasmState?.active_manifest_path,
    );
    if (activeMatch) {
      selectedManifestPath = activeMatch.manifest_path;
      return;
    }
    const configuredMatch = findCatalogItemByManifestPath(
      items,
      wasmState?.configured_indicator_manifest_path || wasmState?.configured_manifest_path,
    );
    if (configuredMatch) {
      selectedManifestPath = configuredMatch.manifest_path;
      return;
    }
    if (!findCatalogItemByManifestPath(items, selectedManifestPath)) {
      selectedManifestPath = items.length > 0 ? items[0].manifest_path : '';
    }
  }

  async function refreshCatalog(forceRefresh) {
    const response = await runAction('catalog', {
      force: !!forceRefresh,
      surface: 'indicator',
    });
    if (!response || response.ok === false) {
      return;
    }
    catalog = normalizeCatalogItems(response.plugins);
    catalogErrors = normalizeCatalogErrors(response.errors);
    selectBestManifest(catalog);
    if (!statusMessage) {
      statusTone = 'success';
      statusMessage = text('status_input_indicator_plugin_catalog_ready', 'Indicator plugin list updated.');
    }
  }

  async function loadSelectedPlugin() {
    if (!selectedManifestPath) {
      statusTone = 'error';
      statusMessage = text('wasm_manifest_required', 'Please select a plugin manifest first.');
      return;
    }
    const response = await runAction('loadManifest', {
      manifest_path: selectedManifestPath,
      surface: 'indicator',
    });
    if (!response || response.ok === false) {
      return;
    }
    dispatch('change', {
      manifestPath: selectedManifestPath,
    });
    statusTone = 'success';
    statusMessage = text(
      'status_input_indicator_plugin_loaded',
      'Indicator plugin loaded. Enable WASM override to replace the native indicator.',
    );
  }

  async function enableRuntime() {
    const response = await runAction('enable', {});
    if (!response || response.ok === false) {
      return;
    }
    statusTone = 'success';
    statusMessage = text('status_input_indicator_plugin_runtime_ready', 'WASM runtime enabled.');
  }

  function handlePluginToggle(event) {
    dispatch('change', {
      pluginEnabled: !!event.currentTarget.checked,
    });
  }

  function handleFallbackToggle(event) {
    dispatch('change', {
      fallbackToNative: !!event.currentTarget.checked,
    });
  }

  function handleManifestSelection() {
    dispatch('change', {
      manifestPath: selectedManifestPath,
    });
  }

  function normalizeRouteStatus(input) {
    const value = input && typeof input === 'object' ? input : {};
    return {
      route_attempted: !!value.route_attempted,
      event_kind: `${value.event_kind || ''}`.trim(),
      reason: `${value.reason || ''}`.trim(),
      rendered_by_wasm: !!value.rendered_by_wasm,
      native_fallback_applied: !!value.native_fallback_applied,
      event_supported: !!value.event_supported,
      wasm_fallback_enabled: !!value.wasm_fallback_enabled,
    };
  }

  function routeEventText(kind) {
    switch (kind) {
      case 'click':
        return text('text_input_indicator_route_event_click', 'Click');
      case 'scroll':
        return text('text_input_indicator_route_event_scroll', 'Scroll');
      case 'key':
        return text('text_input_indicator_route_event_key', 'Key');
      default:
        return text('text_input_indicator_route_event_unknown', 'Unknown');
    }
  }

  function routeReasonText(reason) {
    switch (reason) {
      case 'wasm_rendered':
        return text('text_input_indicator_route_reason_wasm_rendered', 'WASM rendered output');
      case 'fallback_disabled':
        return text('text_input_indicator_route_reason_fallback_disabled', 'Fallback disabled');
      case 'event_not_supported':
        return text('text_input_indicator_route_reason_event_not_supported', 'Plugin does not support this event');
      case 'plugin_unloaded':
        return text('text_input_indicator_route_reason_plugin_unloaded', 'Runtime or plugin not ready');
      case 'anchor_unavailable':
        return text('text_input_indicator_route_reason_anchor_unavailable', 'Indicator anchor unavailable');
      case 'invoke_failed_no_output':
        return text('text_input_indicator_route_reason_invoke_failed_no_output', 'Plugin invoked but no output');
      case 'invoke_no_output':
        return text('text_input_indicator_route_reason_invoke_no_output', 'Plugin invoked with no visible output');
      default:
        return text('text_input_indicator_route_reason_unknown', 'Unknown');
    }
  }

  let catalog = [];
  let catalogErrors = [];
  let selectedManifestPath = '';
  let busy = false;
  let statusTone = '';
  let statusMessage = '';
  let initialCatalogRequested = false;

  $: if (!initialCatalogRequested && typeof onAction === 'function') {
    initialCatalogRequested = true;
    refreshCatalog(false);
  }

  $: if (
    normalizeManifestPathForCompare(manifestPath) &&
    normalizeManifestPathForCompare(manifestPath) !== normalizeManifestPathForCompare(selectedManifestPath)
  ) {
    selectedManifestPath = manifestPath;
  }

  $: selectBestManifest(catalog);

  $: runtimeEnabled = !!(wasmState?.indicator_enabled ?? wasmState?.enabled);
  $: pluginLoaded = !!(wasmState?.indicator_plugin_loaded ?? wasmState?.plugin_loaded);
  $: configuredPluginTitle = pluginLabel(
    findCatalogItemByManifestPath(catalog, manifestPath)
    || findCatalogItemByManifestPath(catalog, selectedManifestPath)
    || findCatalogItemByManifestPath(
      catalog,
      wasmState?.configured_indicator_manifest_path || wasmState?.configured_manifest_path,
    ),
  );
  $: activePluginTitle = wasmState?.indicator_active_plugin_name
    || wasmState?.indicator_active_plugin_id
    || wasmState?.active_plugin_name
    || wasmState?.active_plugin_id
    || configuredPluginTitle
    || text('wasm_text_no_active_plugin', 'Not loaded');
  $: routeStatus = normalizeRouteStatus(wasmState?.input_indicator_wasm_route_status);
  $: routeStatusClass = routeStatus.native_fallback_applied
    ? 'is-warn'
    : (routeStatus.rendered_by_wasm ? 'is-ok' : 'is-idle');
  $: routeEventLabel = routeEventText(routeStatus.event_kind);
  $: routeReasonLabel = routeReasonText(routeStatus.reason);
  $: summaryText = pluginEnabled
    ? text('text_input_indicator_plugin_summary_on', 'WASM override is active.')
    : text('text_input_indicator_plugin_summary_off', 'Native indicator remains active.');
</script>

<div class="indicator-plugin-menu">
  <div class="indicator-plugin-menu__hero">
    <div class="indicator-plugin-menu__summary-copy">
      <div class="indicator-plugin-menu__title" data-i18n="title_input_indicator_plugin_menu">Indicator plugin</div>
      <div class="indicator-plugin-menu__summary">{summaryText}</div>
    </div>
    <span class={`indicator-plugin-menu__pill ${pluginEnabled ? 'is-on' : 'is-off'}`}>
      {pluginEnabled
        ? (texts.text_wasm_runtime_on || 'Using Plugin')
        : (texts.text_wasm_runtime_off || 'Plugin Off')}
    </span>
  </div>

  <div class="indicator-plugin-menu__body">
    <label class="indicator-plugin-menu__toggle">
      <span class="indicator-plugin-menu__toggle-copy">
        <span data-i18n="label_input_indicator_plugin_override">Use plugin to override native indicator</span>
        <small data-i18n="hint_input_indicator_plugin_override">
          When enabled, the input indicator switches to the active WASM plugin; the native renderer is no longer used unless fallback is triggered.
        </small>
      </span>
      <input type="checkbox" checked={pluginEnabled} on:change={handlePluginToggle} />
    </label>

    <div class="indicator-plugin-menu__status-grid indicator-plugin-menu__status-grid--primary">
      <div class="indicator-plugin-menu__status-item indicator-plugin-menu__status-item--action">
        <div class="indicator-plugin-menu__meta-label" data-i18n="label_input_indicator_plugin_runtime">WASM runtime</div>
        <div class="indicator-plugin-menu__meta-value">{runtimeEnabled ? (texts.text_input_indicator_plugin_runtime_ready || 'Ready') : (texts.text_input_indicator_plugin_runtime_off || 'Disabled')}</div>
        <button
          type="button"
          class="secondary"
          disabled={busy || runtimeEnabled}
          on:click={enableRuntime}
          data-i18n="btn_input_indicator_plugin_enable_runtime"
          title={text('tip_input_indicator_plugin_enable_runtime', 'Enable the shared WASM runtime so the selected indicator plugin can render.')}
        >
          {texts.btn_input_indicator_plugin_enable_runtime || 'Enable Runtime'}
        </button>
      </div>

      <div class="indicator-plugin-menu__status-item indicator-plugin-menu__status-item--plugin">
        <div class="indicator-plugin-menu__meta-label" data-i18n="label_input_indicator_plugin_active">Active plugin</div>
        <div class="indicator-plugin-menu__meta-value indicator-plugin-menu__meta-value--plugin" title={activePluginTitle}>{activePluginTitle}</div>
        <span class={`indicator-plugin-menu__state ${pluginLoaded ? 'is-on' : 'is-off'}`}>
          {pluginLoaded ? (texts.wasm_text_yes || 'Yes') : (texts.wasm_text_no || 'No')}
        </span>
      </div>
    </div>

    <div class="indicator-plugin-menu__status-grid indicator-plugin-menu__status-grid--route">
      <div class="indicator-plugin-menu__status-item indicator-plugin-menu__status-item--route">
        <div class="indicator-plugin-menu__meta-label" data-i18n="label_input_indicator_plugin_route_status">Route status</div>
        {#if routeStatus.route_attempted}
          <span class={`indicator-plugin-menu__state ${routeStatusClass}`}>{routeReasonLabel}</span>
        {:else}
          <span class="indicator-plugin-menu__state is-off">{text('text_input_indicator_plugin_route_unavailable', 'No route attempt yet')}</span>
        {/if}
      </div>
      {#if routeStatus.route_attempted}
        <div class="indicator-plugin-menu__status-item indicator-plugin-menu__status-item--route">
          <div class="indicator-plugin-menu__meta-label" data-i18n="label_input_indicator_plugin_route_event">Event kind</div>
          <div class="indicator-plugin-menu__meta-value">{routeEventLabel}</div>
        </div>
        <div class="indicator-plugin-menu__status-item indicator-plugin-menu__status-item--route">
          <div class="indicator-plugin-menu__meta-label" data-i18n="label_input_indicator_plugin_route_rendered">Rendered by WASM</div>
          <div class="indicator-plugin-menu__meta-value">{routeStatus.rendered_by_wasm ? (texts.wasm_text_yes || 'Yes') : (texts.wasm_text_no || 'No')}</div>
        </div>
        <div class="indicator-plugin-menu__status-item indicator-plugin-menu__status-item--route">
          <div class="indicator-plugin-menu__meta-label" data-i18n="label_input_indicator_plugin_route_native_fallback">Native fallback applied</div>
          <div class="indicator-plugin-menu__meta-value">{routeStatus.native_fallback_applied ? (texts.wasm_text_yes || 'Yes') : (texts.wasm_text_no || 'No')}</div>
        </div>
      {/if}
    </div>

    <div class="indicator-plugin-menu__catalog">
      <div class="indicator-plugin-menu__catalog-row">
        <label for="ii_plugin_manifest" data-i18n="label_input_indicator_plugin_select">Indicator plugin</label>
        <div class="indicator-plugin-menu__catalog-actions">
          <button
            type="button"
            class="secondary"
            disabled={busy}
            on:click={() => refreshCatalog(true)}
            data-i18n="btn_wasm_refresh_catalog"
            title={text('tip_input_indicator_plugin_refresh', 'Refresh the compatible indicator plugin list.')}
          >
            {texts.btn_wasm_refresh_catalog || 'Refresh Plugin List'}
          </button>
          <button
            type="button"
            disabled={busy || !selectedManifestPath}
            on:click={loadSelectedPlugin}
            data-i18n="btn_wasm_load_selected"
            title={text('tip_input_indicator_plugin_load', 'Load the selected indicator-compatible plugin into the shared WASM runtime.')}
          >
            {texts.btn_wasm_load_selected || 'Load Selected'}
          </button>
        </div>
      </div>

      <select
        id="ii_plugin_manifest"
        bind:value={selectedManifestPath}
        disabled={busy || catalog.length === 0}
        on:change={handleManifestSelection}
      >
        {#if catalog.length === 0}
          <option value="">{text('text_input_indicator_plugin_catalog_empty', 'No compatible indicator plugins discovered.')}</option>
        {:else}
          {#each catalog as plugin}
            <option value={plugin.manifest_path}>{pluginLabel(plugin)}</option>
          {/each}
        {/if}
      </select>

      <div class="hint" data-i18n="hint_input_indicator_plugin_catalog">
        Indicator-compatible plugins are preferred by explicit `surfaces: ["indicator"]`; older manifests still fall back to `indicator_*` input kinds.
      </div>

      {#if catalogErrors.length > 0}
        <div class="hint indicator-plugin-menu__catalog-error">
          <strong data-i18n="label_wasm_catalog_errors">Plugin scan errors:</strong>
          <span>{catalogErrors[0]}</span>
        </div>
      {/if}
    </div>

    <label class="indicator-plugin-menu__toggle">
      <span class="indicator-plugin-menu__toggle-copy">
        <span data-i18n="label_input_indicator_wasm_fallback">WASM fallback to native</span>
        <small data-i18n="hint_input_indicator_plugin_fallback">
          Keep native fallback on if you want the original indicator to recover automatically when runtime or plugin rendering is unavailable.
        </small>
      </span>
      <input
        type="checkbox"
        checked={fallbackToNative}
        on:change={handleFallbackToggle}
        disabled={!pluginEnabled}
      />
    </label>

    {#if statusMessage}
      <div class={`indicator-plugin-menu__status is-${statusTone || 'info'}`}>{statusMessage}</div>
    {/if}
  </div>
</div>

<style>
  .indicator-plugin-menu {
    display: flex;
    flex-direction: column;
    gap: 14px;
  }

  .indicator-plugin-menu__hero {
    display: flex;
    align-items: flex-start;
    justify-content: space-between;
    gap: 12px;
  }

  .indicator-plugin-menu__title {
    font-weight: 600;
  }

  .indicator-plugin-menu__summary {
    margin-top: 4px;
    color: rgba(82, 96, 122, 0.92);
    font-size: 13px;
  }

  .indicator-plugin-menu__pill,
  .indicator-plugin-menu__state {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    min-width: 84px;
    padding: 4px 10px;
    border-radius: 999px;
    font-size: 12px;
    font-weight: 600;
    white-space: nowrap;
  }

  .indicator-plugin-menu__pill.is-on,
  .indicator-plugin-menu__state.is-on {
    background: rgba(18, 168, 117, 0.12);
    color: #117a56;
  }

  .indicator-plugin-menu__pill.is-off,
  .indicator-plugin-menu__state.is-off {
    background: rgba(140, 153, 177, 0.14);
    color: #4f5f78;
  }

  .indicator-plugin-menu__body {
    display: grid;
    gap: 14px;
  }

  .indicator-plugin-menu__toggle {
    display: flex;
    align-items: flex-start;
    justify-content: space-between;
    gap: 12px;
  }

  .indicator-plugin-menu__toggle-copy {
    display: grid;
    gap: 4px;
  }

  .indicator-plugin-menu__toggle-copy small {
    color: rgba(82, 96, 122, 0.92);
    line-height: 1.4;
  }

  .indicator-plugin-menu__catalog {
    display: grid;
    gap: 10px;
    padding: 12px;
    border-radius: 10px;
    background: rgba(255, 255, 255, 0.54);
  }

  .indicator-plugin-menu__status-grid {
    display: grid;
    grid-template-columns: repeat(2, minmax(0, 1fr));
    gap: 8px;
  }

  .indicator-plugin-menu__status-grid--route {
    grid-template-columns: repeat(4, minmax(0, 1fr));
  }

  .indicator-plugin-menu__status-item,
  .indicator-plugin-menu__catalog-row {
    display: flex;
    align-items: center;
    gap: 12px;
    flex-wrap: wrap;
  }

  .indicator-plugin-menu__status-item {
    display: grid;
    grid-template-columns: minmax(108px, 132px) minmax(0, 1fr) auto;
    align-items: center;
    gap: 6px 10px;
    padding: 8px 12px;
    border-radius: 12px;
    background: rgba(255, 255, 255, 0.56);
    border: 1px solid rgba(186, 202, 226, 0.72);
    box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.44);
    min-height: 54px;
  }

  .indicator-plugin-menu__status-item--route {
    grid-template-columns: 1fr auto;
  }

  .indicator-plugin-menu__status-item--plugin {
    grid-template-columns: minmax(108px, 132px) minmax(0, 1fr) auto;
  }

  .indicator-plugin-menu__status-item--action {
    grid-template-columns: minmax(108px, 132px) minmax(0, 1fr) auto;
  }

  .indicator-plugin-menu__meta-label {
    min-width: 0;
    font-weight: 600;
    color: rgba(47, 66, 96, 0.92);
    font-size: 12px;
    line-height: 1.2;
  }

  .indicator-plugin-menu__meta-value {
    flex: none;
    min-width: 0;
    word-break: break-word;
    color: rgba(30, 43, 64, 0.96);
    font-size: 12px;
    line-height: 1.2;
  }

  .indicator-plugin-menu__meta-value--plugin {
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
    word-break: normal;
  }

  .indicator-plugin-menu__catalog label {
    font-weight: 600;
  }

  .indicator-plugin-menu__catalog-actions {
    display: flex;
    gap: 8px;
    margin-left: auto;
    flex-wrap: wrap;
  }

  .indicator-plugin-menu__catalog select {
    width: 100%;
  }

  .indicator-plugin-menu__catalog-error {
    color: #9e3a3a;
  }

  .indicator-plugin-menu__status {
    padding: 10px 12px;
    border-radius: 10px;
    font-size: 13px;
  }

  .indicator-plugin-menu__status.is-success {
    background: rgba(18, 168, 117, 0.1);
    color: #117a56;
  }

  .indicator-plugin-menu__status.is-error {
    background: rgba(200, 61, 61, 0.12);
    color: #9e2d2d;
  }

  .indicator-plugin-menu__status.is-info {
    background: rgba(90, 120, 180, 0.12);
    color: #37538a;
  }

  .indicator-plugin-menu__state.is-warn {
    background: rgba(200, 126, 34, 0.16);
    color: #8a5411;
  }

  .indicator-plugin-menu__state.is-ok {
    background: rgba(18, 168, 117, 0.12);
    color: #117a56;
  }

  .indicator-plugin-menu__state.is-idle {
    background: rgba(120, 138, 170, 0.12);
    color: #4a5b78;
  }

  @media (max-width: 1400px) {
    .indicator-plugin-menu__status-grid--route {
      grid-template-columns: repeat(2, minmax(0, 1fr));
    }
  }

  @media (max-width: 720px) {
    .indicator-plugin-menu__hero,
    .indicator-plugin-menu__toggle,
    .indicator-plugin-menu__catalog-row {
      align-items: flex-start;
      flex-direction: column;
    }

    .indicator-plugin-menu__status-grid,
    .indicator-plugin-menu__status-grid--route {
      grid-template-columns: 1fr;
    }

    .indicator-plugin-menu__status-item,
    .indicator-plugin-menu__status-item--route,
    .indicator-plugin-menu__status-item--plugin,
    .indicator-plugin-menu__status-item--action {
      grid-template-columns: minmax(0, 1fr);
      align-items: flex-start;
    }

    .indicator-plugin-menu__catalog-actions {
      margin-left: 0;
    }
  }
</style>
