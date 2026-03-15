<script>
  export let rowEnabled = true;
  export let scopeOptions = [];
  export let scopeMode = 'all';
  export let scopeApps = [];
  export let scopeDraft = '';
  export let texts = {};
  export let catalogEntries = [];
  export let appCatalogLoading = false;
  export let appCatalogError = '';
  export let onScopeModeChange = null;
  export let onScopeDraftInput = null;
  export let onScopeDraftKeydown = null;
  export let onRemoveScopeApp = null;
  export let onAddCatalogScopeApp = null;
  export let onRefreshAppCatalog = null;
  export let onPickFile = null;
  export let catalogMetaText = (entry) => '';
  export let layout = 'stacked';

  function callHandler(handler, ...args) {
    if (typeof handler === 'function') {
      handler(...args);
    }
  }
</script>

{#if layout === 'two-column'}
  <div class="effects-scope-layout">
    <div class="automation-scope-group automation-col">
      <select
        class="automation-scope-select"
        disabled={!rowEnabled}
        value={scopeMode}
        on:change={(event) => callHandler(onScopeModeChange, event)}
      >
        {#each scopeOptions as option (option.value)}
          <option value={option.value}>{option.label}</option>
        {/each}
      </select>
      {#if scopeMode === 'selected'}
        <div class="automation-scope-chip-list">
          {#each scopeApps as app (app)}
            <span class="automation-scope-chip">
              <span>{app}</span>
              <button
                type="button"
                class="automation-scope-chip-remove"
                disabled={!rowEnabled}
                on:click={() => callHandler(onRemoveScopeApp, app)}
              >
                x
              </button>
            </span>
          {/each}
        </div>
      {/if}
    </div>
    {#if scopeMode === 'selected'}
      <div class="automation-shortcut-pane automation-col">
        <input
          class="automation-scope-app"
          type="text"
          disabled={!rowEnabled}
          value={scopeDraft}
          placeholder={texts.scopeSearchPlaceholder || texts.scopeAppPlaceholder}
          on:keydown={(event) => callHandler(onScopeDraftKeydown, event)}
          on:input={(event) => callHandler(onScopeDraftInput, event)}
        />
        <div class="automation-shortcut-scope">
          <div class="automation-scope-tools">
            <button
              class="btn-soft automation-scope-refresh"
              type="button"
              disabled={!rowEnabled || appCatalogLoading}
              on:click={() => callHandler(onRefreshAppCatalog)}
            >
              {appCatalogLoading ? texts.scopeRefreshingCatalog : texts.scopeRefreshCatalog}
            </button>
            <button
              class="btn-soft automation-scope-file"
              type="button"
              disabled={!rowEnabled}
              on:click={() => callHandler(onPickFile)}
            >
              {texts.scopePickFromFile}
            </button>
          </div>
          <div class="automation-scope-catalog">
            {#if appCatalogLoading}
              <div class="automation-scope-catalog-state">{texts.scopeCatalogLoading}</div>
            {:else if appCatalogError}
              <div class="automation-scope-catalog-state is-error">{appCatalogError}</div>
            {:else}
              {#if catalogEntries.length === 0}
                <div class="automation-scope-catalog-state">{texts.scopeCatalogEmpty}</div>
              {:else}
                {#each catalogEntries as app (app.exe)}
                  <button
                    type="button"
                    class="automation-scope-catalog-item"
                    disabled={!rowEnabled}
                    on:click={() => callHandler(onAddCatalogScopeApp, app.exe)}
                  >
                    <span class="automation-scope-catalog-label">{app.label}</span>
                    <span
                      class="automation-scope-catalog-meta"
                      title={catalogMetaText(app)}
                      aria-label={catalogMetaText(app)}
                    >
                      i
                    </span>
                  </button>
                {/each}
              {/if}
            {/if}
          </div>
        </div>
      </div>
    {/if}
  </div>
{:else}
  <div class="automation-scope-group automation-col">
    <select
      class="automation-scope-select"
      disabled={!rowEnabled}
      value={scopeMode}
      on:change={(event) => callHandler(onScopeModeChange, event)}
    >
      {#each scopeOptions as option (option.value)}
        <option value={option.value}>{option.label}</option>
      {/each}
    </select>
    {#if scopeMode === 'selected'}
      <div class="automation-scope-chip-list">
        {#each scopeApps as app (app)}
          <span class="automation-scope-chip">
            <span>{app}</span>
            <button
              type="button"
              class="automation-scope-chip-remove"
              disabled={!rowEnabled}
              on:click={() => callHandler(onRemoveScopeApp, app)}
            >
              x
            </button>
          </span>
        {/each}
      </div>
    {/if}
  </div>
  {#if scopeMode === 'selected'}
    <div class="automation-shortcut-pane automation-col">
      <input
        class="automation-scope-app"
        type="text"
        disabled={!rowEnabled}
        value={scopeDraft}
        placeholder={texts.scopeSearchPlaceholder || texts.scopeAppPlaceholder}
        on:keydown={(event) => callHandler(onScopeDraftKeydown, event)}
        on:input={(event) => callHandler(onScopeDraftInput, event)}
      />
      <div class="automation-shortcut-scope">
        <div class="automation-scope-tools">
          <button
            class="btn-soft automation-scope-refresh"
            type="button"
            disabled={!rowEnabled || appCatalogLoading}
            on:click={() => callHandler(onRefreshAppCatalog)}
          >
            {appCatalogLoading ? texts.scopeRefreshingCatalog : texts.scopeRefreshCatalog}
          </button>
          <button
            class="btn-soft automation-scope-file"
            type="button"
            disabled={!rowEnabled}
            on:click={() => callHandler(onPickFile)}
          >
            {texts.scopePickFromFile}
          </button>
        </div>
        <div class="automation-scope-catalog">
          {#if appCatalogLoading}
            <div class="automation-scope-catalog-state">{texts.scopeCatalogLoading}</div>
          {:else if appCatalogError}
            <div class="automation-scope-catalog-state is-error">{appCatalogError}</div>
          {:else}
            {#if catalogEntries.length === 0}
              <div class="automation-scope-catalog-state">{texts.scopeCatalogEmpty}</div>
            {:else}
              {#each catalogEntries as app (app.exe)}
                <button
                  type="button"
                  class="automation-scope-catalog-item"
                  disabled={!rowEnabled}
                  on:click={() => callHandler(onAddCatalogScopeApp, app.exe)}
                >
                  <span class="automation-scope-catalog-label">{app.label}</span>
                  <span
                    class="automation-scope-catalog-meta"
                    title={catalogMetaText(app)}
                    aria-label={catalogMetaText(app)}
                  >
                    i
                  </span>
                </button>
              {/each}
            {/if}
          {/if}
        </div>
      </div>
    </div>
  {/if}
{/if}
