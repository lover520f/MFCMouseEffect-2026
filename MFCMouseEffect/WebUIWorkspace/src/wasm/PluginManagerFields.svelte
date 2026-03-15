<script>
  import { normalizeActionErrorCode, resolveWasmActionErrorMessage } from './action-error-model.js';
  import { normalizeWasmState } from './state-model.js';

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
      if (!manifestPath) {
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

  function normalizeCatalogRoots(input) {
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

  function pluginLabel(plugin) {
    const title = plugin?.name || plugin?.id || text('wasm_text_unknown_plugin', 'Unknown plugin');
    if (!plugin?.version) {
      return title;
    }
    return `${title} (${plugin.version})`;
  }

  function pluginSurfacesLabel(plugin) {
    const surfaces = Array.isArray(plugin?.surfaces)
      ? plugin.surfaces.filter((value) => `${value || ''}`.trim().length > 0)
      : [];
    if (surfaces.length > 0) {
      return surfaces.join(', ');
    }
    if (plugin?.has_explicit_surfaces) {
      return text('text_plugin_surface_none', 'none');
    }
    return text('text_plugin_surface_auto', 'auto');
  }

  function normalizePathForCompare(path) {
    const value = `${path || ''}`.trim().replace(/\\/g, '/').replace(/\/+/g, '/').toLowerCase();
    return value;
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

  function resolvedSurfaceFilter() {
    if (surfaceFilter === 'effects') {
      return 'effects';
    }
    if (surfaceFilter === 'indicator') {
      return 'indicator';
    }
    return '';
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
        catalog = normalizeCatalogItems(response?.plugins);
        catalogErrors = normalizeCatalogErrors(response?.errors);
        catalogSearchRoots = normalizeCatalogRoots(response?.search_roots);
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
    await runAction('catalog', {
      force: !!forceRefresh,
      surface: resolvedSurfaceFilter(),
    });
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
    if (!importedManifestPath) {
      return;
    }
    statusTone = 'ok';
    statusMessage = `${text('wasm_import_success_prefix', 'Imported to primary root: ')}${importedManifestPath}`;
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

  let current = normalizeWasmState(payloadState);
  let lastPayloadRef = payloadState;
  let initialCatalogRequested = false;
  let policyCatalogRootPath = current.configured_catalog_root_path || '';
  let surfaceFilter = 'all';
  let catalog = [];
  let catalogErrors = [];
  let catalogSearchRoots = [];
  let busy = false;
  let statusTone = '';
  let statusMessage = '';
  let statusErrorCode = '';
  let configuredCatalogRootNormalized = '';
  let firstCatalogRootNormalized = '';
  let showCatalogRoots = false;

  $: if (payloadState !== lastPayloadRef) {
    lastPayloadRef = payloadState;
    current = normalizeWasmState(payloadState);
    policyCatalogRootPath = current.configured_catalog_root_path || '';
  }

  $: configuredCatalogRootNormalized = normalizePathForCompare(current.configured_catalog_root_path || '');
  $: firstCatalogRootNormalized = normalizePathForCompare(catalogSearchRoots[0] || '');
  $: showCatalogRoots = catalogSearchRoots.length > 1
    || (catalogSearchRoots.length === 1 && firstCatalogRootNormalized !== configuredCatalogRootNormalized);

  $: if (!initialCatalogRequested && typeof onAction === 'function') {
    initialCatalogRequested = true;
    requestCatalog(false);
  }
</script>

<div class="wasm-panel plugin-manager-panel">
  <section class="wasm-block">
    <div class="wasm-catalog">
      <div class="wasm-catalog-controls">
        <label class="plugin-manager-filter-label" for="plugin_catalog_root_path" data-i18n="label_wasm_catalog_root_path">
          Catalog root path
        </label>
        <input
          id="plugin_catalog_root_path"
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
        <label class="plugin-manager-filter-label" for="plugin_surface_filter" data-i18n="label_plugin_surface_filter">Surface</label>
        <select
          id="plugin_surface_filter"
          bind:value={surfaceFilter}
          disabled={busy}
        >
          <option value="all" data-i18n="text_plugin_surface_all">All</option>
          <option value="effects" data-i18n="text_plugin_surface_effects">Effects</option>
          <option value="indicator" data-i18n="text_plugin_surface_indicator">Indicator</option>
        </select>
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

      {#if showCatalogRoots}
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

      <div class="plugin-manager-list">
        <div class="plugin-manager-list-head">
          <span class="plugin-manager-count">{text('label_plugin_catalog_count', 'Catalog items')}: {catalog.length}</span>
        </div>
        {#if catalog.length === 0}
          <div class="hint">{text('text_plugin_catalog_empty', 'No plugins discovered for current filter.')}</div>
        {:else}
          <div class="plugin-manager-list-body">
            {#each catalog as plugin}
              <div class="plugin-manager-row">
                <div class="plugin-manager-name">{pluginLabel(plugin)}</div>
                <div class="plugin-manager-meta">
                  <span class="plugin-manager-surface-line">
                    <strong>{text('label_plugin_surfaces', 'Surfaces')}:</strong>
                    <span>{pluginSurfacesLabel(plugin)}</span>
                  </span>
                </div>
                <div class="plugin-manager-meta">
                  <span class="plugin-manager-path-line">
                    <strong>{text('label_plugin_manifest_path', 'Manifest')}:</strong>
                    <span class="plugin-manager-path" title={plugin.manifest_path}>{plugin.manifest_path}</span>
                  </span>
                </div>
              </div>
            {/each}
          </div>
        {/if}
      </div>
    </div>
  </section>

</div>
