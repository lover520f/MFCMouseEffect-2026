<script>
  import { createEventDispatcher } from 'svelte';

  export let uiLanguages = [];
  export let themes = [];
  export let holdFollowModes = [];
  export let holdPresenterBackends = [];
  export let general = {};

  const dispatch = createEventDispatcher();

  function normalizeGeneral(input) {
    const value = input || {};
    return {
      ui_language: value.ui_language || '',
      theme: value.theme || '',
      hold_follow_mode: value.hold_follow_mode || 'smooth',
      hold_presenter_backend: value.hold_presenter_backend || 'auto',
    };
  }

  function toSnapshot(form) {
    const value = form || normalizeGeneral({});
    return {
      ui_language: value.ui_language || '',
      theme: value.theme || '',
      hold_follow_mode: value.hold_follow_mode || 'smooth',
      hold_presenter_backend: value.hold_presenter_backend || 'auto',
    };
  }

  let form = normalizeGeneral(general);
  let lastGeneralRef = general;

  $: if (general !== lastGeneralRef) {
    lastGeneralRef = general;
    form = normalizeGeneral(general);
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
