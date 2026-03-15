<script>
  import { createEventDispatcher } from 'svelte';
  import {
    findCatalogItemByManifestPath,
    findCatalogItemByPluginId,
    normalizeCatalogErrors,
    normalizeCatalogItems,
    normalizeManifestPathForCompare,
    manifestPathForIndicatorLoad,
    normalizeRouteStatus,
    pluginLabel,
    routeEventText,
    routeReasonText,
  } from './input-indicator-plugin-menu-model.js';

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

  function pluginLabelWithText(plugin) {
    return pluginLabel(plugin, text);
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
    const activePluginMatch = findCatalogItemByPluginId(
      items,
      wasmState?.indicator_active_plugin_id || wasmState?.active_plugin_id,
    );
    if (activePluginMatch) {
      selectedManifestPath = activePluginMatch.manifest_path;
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

  async function enableRuntime() {
    const response = await runAction('enable', {});
    if (!response || response.ok === false) {
      return;
    }
    statusTone = 'success';
    statusMessage = text('status_input_indicator_plugin_runtime_ready', 'WASM runtime enabled.');
  }

  async function handlePluginToggle(event) {
    const nextEnabled = !!event.currentTarget.checked;
    dispatch('change', {
      pluginEnabled: nextEnabled,
    });
    if (!nextEnabled) {
      statusTone = 'info';
      statusMessage = text(
        'text_input_indicator_plugin_summary_off',
        'Native indicator remains active.',
      );
      return;
    }
    const candidateManifestPath = `${selectedManifestPath || manifestPath || ''}`.trim();
    if (!candidateManifestPath) {
      dispatch('change', {
        pluginEnabled: false,
      });
      statusTone = 'error';
      statusMessage = text(
        'wasm_manifest_required',
        'Please select a plugin manifest first.',
      );
      return;
    }
    const response = await runAction('loadManifest', {
      manifest_path: candidateManifestPath,
      surface: 'indicator',
    });
    if (!response || response.ok === false) {
      dispatch('change', {
        pluginEnabled: false,
      });
      return;
    }
    const confirmedManifestPath = `${response.configured_indicator_manifest_path || candidateManifestPath}`.trim();
    if (confirmedManifestPath && confirmedManifestPath !== candidateManifestPath) {
      dispatch('change', {
        manifestPath: confirmedManifestPath,
      });
    }
  }

  function handleFallbackToggle(event) {
    dispatch('change', {
      fallbackToNative: !!event.currentTarget.checked,
    });
  }

  async function handleManifestSelection() {
    const nextManifestPath = manifestPathForIndicatorLoad(selectedManifestPath, manifestPath);
    if (!nextManifestPath) {
      return;
    }
    const previousManifestPath = `${manifestPath || ''}`.trim();
    dispatch('change', {
      manifestPath: nextManifestPath,
    });
    const response = await runAction('loadManifest', {
      manifest_path: nextManifestPath,
      surface: 'indicator',
    });
    if (!response || response.ok === false) {
      if (previousManifestPath && previousManifestPath !== nextManifestPath) {
        dispatch('change', {
          manifestPath: previousManifestPath,
        });
      }
      return;
    }
    const confirmedManifestPath = `${response.configured_indicator_manifest_path || nextManifestPath}`.trim();
    if (confirmedManifestPath && confirmedManifestPath !== nextManifestPath) {
      dispatch('change', {
        manifestPath: confirmedManifestPath,
      });
    }
    statusTone = 'success';
    statusMessage = text(
      'status_input_indicator_plugin_loaded',
      'Indicator plugin loaded.',
    );
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

  $: selectBestManifest(catalog);

  $: runtimeEnabled = !!(wasmState?.indicator_enabled ?? wasmState?.enabled);
  $: pluginLoaded = !!(wasmState?.indicator_plugin_loaded ?? wasmState?.plugin_loaded);
  $: configuredPluginTitle = pluginLabelWithText(
    findCatalogItemByManifestPath(catalog, manifestPath)
    || findCatalogItemByManifestPath(catalog, selectedManifestPath)
    || findCatalogItemByManifestPath(
      catalog,
      wasmState?.configured_indicator_manifest_path || wasmState?.configured_manifest_path,
    )
    || findCatalogItemByPluginId(
      catalog,
      wasmState?.indicator_active_plugin_id || wasmState?.active_plugin_id,
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
  $: routeEventLabel = routeEventText(routeStatus.event_kind, text);
  $: routeReasonLabel = routeReasonText(routeStatus.reason, text);
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
    <label class="indicator-plugin-menu__toggle indicator-plugin-menu__toggle--primary">
      <span class="indicator-plugin-menu__toggle-copy">
        <span data-i18n="label_input_indicator_plugin_override">Use plugin to override native indicator</span>
        <small data-i18n="hint_input_indicator_plugin_override">
          When enabled, the input indicator switches to the active WASM plugin; the native renderer is no longer used unless fallback is triggered.
        </small>
      </span>
      <span class="indicator-plugin-menu__switch-wrap">
        <input
          type="checkbox"
          class="indicator-plugin-menu__switch-input"
          checked={pluginEnabled}
          on:change={handlePluginToggle}
        />
        <span class="indicator-plugin-menu__switch" aria-hidden="true">
          <span class="indicator-plugin-menu__switch-thumb"></span>
        </span>
      </span>
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
            <option value={plugin.manifest_path}>{pluginLabelWithText(plugin)}</option>
          {/each}
        {/if}
      </select>

      <div class="hint" data-i18n="hint_input_indicator_plugin_catalog">
        Indicator-compatible plugins are preferred by explicit `surfaces: ["indicator"]`; older manifests still fall back to `indicator_*` input kinds.
      </div>
      <div class="hint" data-i18n="hint_input_indicator_plugin_manage_entry">
        Import and export plugins from the unified Plugin Management section.
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

  .indicator-plugin-menu__toggle--primary {
    align-items: center;
  }

  .indicator-plugin-menu__toggle-copy {
    display: grid;
    gap: 4px;
  }

  .indicator-plugin-menu__switch-wrap {
    position: relative;
    flex: none;
    width: 52px;
    height: 30px;
    display: inline-flex;
    align-items: center;
    justify-content: center;
  }

  .indicator-plugin-menu__switch-input {
    position: absolute;
    inset: 0;
    margin: 0;
    opacity: 0;
    cursor: pointer;
    z-index: 2;
  }

  .indicator-plugin-menu__switch {
    width: 52px;
    height: 30px;
    border-radius: 999px;
    background: rgba(147, 162, 188, 0.38);
    border: 1px solid rgba(168, 184, 210, 0.78);
    transition: background-color 160ms ease, border-color 160ms ease;
    display: inline-flex;
    align-items: center;
    padding: 3px;
    box-sizing: border-box;
  }

  .indicator-plugin-menu__switch-thumb {
    width: 22px;
    height: 22px;
    border-radius: 50%;
    background: #ffffff;
    box-shadow: 0 1px 2px rgba(0, 0, 0, 0.2);
    transform: translateX(0);
    transition: transform 160ms ease;
  }

  .indicator-plugin-menu__switch-input:checked + .indicator-plugin-menu__switch {
    background: rgba(18, 168, 117, 0.24);
    border-color: rgba(18, 168, 117, 0.5);
  }

  .indicator-plugin-menu__switch-input:checked + .indicator-plugin-menu__switch .indicator-plugin-menu__switch-thumb {
    transform: translateX(22px);
  }

  .indicator-plugin-menu__switch-input:focus-visible + .indicator-plugin-menu__switch {
    box-shadow: 0 0 0 3px rgba(68, 138, 227, 0.28);
  }

  .indicator-plugin-menu__switch-input:disabled {
    cursor: not-allowed;
  }

  .indicator-plugin-menu__switch-input:disabled + .indicator-plugin-menu__switch {
    opacity: 0.55;
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
