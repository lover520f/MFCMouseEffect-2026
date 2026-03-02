<script>
  import { createEventDispatcher } from 'svelte';
  import { normalizeGeneralState } from './general-state-model.js';

  export let uiLanguages = [];
  export let themes = [];
  export let holdFollowModes = [];
  export let holdPresenterBackends = [];
  export let general = {};
  export let onAction = null;

  const dispatch = createEventDispatcher();

  function toSnapshot(form) {
    return normalizeGeneralState(form);
  }

  let form = normalizeGeneralState(general);
  let lastGeneralRef = general;
  let browsePending = false;

  $: if (general !== lastGeneralRef) {
    lastGeneralRef = general;
    form = normalizeGeneralState(general);
  }

  async function browseThemeCatalogPath() {
    if (browsePending) {
      return;
    }
    if (typeof onAction !== 'function') {
      return;
    }
    browsePending = true;
    try {
      const response = await onAction('pickThemeCatalogRootPath', {
        initial_path: form.theme_catalog_root_path || '',
      });
      if (!response || !response.ok) {
        return;
      }
      if (typeof response.selected_folder_path !== 'string') {
        return;
      }
      form = {
        ...form,
        theme_catalog_root_path: response.selected_folder_path,
      };
    } finally {
      browsePending = false;
    }
  }

  $: dispatch('change', toSnapshot(form));
</script>

<div class="grid">
  <label for="ui_language" data-i18n="label_language">Language</label>
  <select id="ui_language" bind:value={form.ui_language}>
    {#each uiLanguages as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  <label for="theme" data-i18n="label_theme">Theme</label>
  <select id="theme" bind:value={form.theme}>
    {#each themes as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  <label for="theme_catalog_root_path" class="label-with-tip">
    <span data-i18n="label_theme_catalog_root_path">Theme Catalog Root</span>
    <span
      class="info-badge"
      data-i18n-title="tip_theme_catalog_root_path"
      title="Optional path for external theme packages."
    >!</span>
  </label>
  <div class="field-inline">
    <input
      id="theme_catalog_root_path"
      type="text"
      bind:value={form.theme_catalog_root_path}
      data-i18n-placeholder="placeholder_theme_catalog_root_path"
      placeholder="Optional: custom theme catalog directory" />
    <button
      type="button"
      class="btn-soft"
      on:click={browseThemeCatalogPath}
      disabled={browsePending}
      data-i18n="btn_theme_catalog_browse"
      data-i18n-title="tip_theme_catalog_browse"
      title="Pick a folder for external theme files (*.theme.json / theme.json).">
      Browse
    </button>
  </div>

  <label for="hold_follow_mode" class="label-with-tip">
    <span data-i18n="label_hold_follow_mode">Hold Tracking</span>
    <span
      class="info-badge"
      data-i18n-title="tip_hold_follow_mode"
      title="Choose by feel and CPU budget."
    >!</span>
  </label>
  <select id="hold_follow_mode" bind:value={form.hold_follow_mode}>
    {#each holdFollowModes as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  <label for="hold_presenter_backend" class="label-with-tip">
    <span data-i18n="label_hold_presenter_backend">Hold GPU Presenter</span>
    <span
      class="info-badge"
      data-i18n-title="tip_hold_presenter_backend"
      title="Select presenter backend strategy."
    >!</span>
  </label>
  <select id="hold_presenter_backend" bind:value={form.hold_presenter_backend}>
    {#each holdPresenterBackends as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>
</div>

<style>
  .field-inline {
    display: flex;
    gap: 8px;
    align-items: center;
  }

  .field-inline input {
    flex: 1;
    min-width: 0;
  }

  .field-inline button {
    white-space: nowrap;
  }
</style>
