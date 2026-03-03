<script>
  import { createEventDispatcher } from 'svelte';
  import { normalizeGeneralState } from './general-state-model.js';

  export let uiLanguages = [];
  export let themes = [];
  export let overlayTargetFpsRange = {};
  export let holdFollowModes = [];
  export let holdPresenterBackends = [];
  export let general = {};

  const dispatch = createEventDispatcher();

  function toSnapshot(form) {
    return normalizeGeneralState(form);
  }

  let form = normalizeGeneralState(general);
  let lastGeneralRef = general;
  $: overlayFpsMin = Number.isFinite(Number(overlayTargetFpsRange?.min))
    ? Number(overlayTargetFpsRange.min)
    : 0;
  $: overlayFpsMax = Number.isFinite(Number(overlayTargetFpsRange?.max))
    ? Number(overlayTargetFpsRange.max)
    : 360;
  $: overlayFpsStep = Number.isFinite(Number(overlayTargetFpsRange?.step))
    ? Number(overlayTargetFpsRange.step)
    : 1;

  $: if (general !== lastGeneralRef) {
    lastGeneralRef = general;
    form = normalizeGeneralState(general);
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
  <input
    id="theme_catalog_root_path"
    type="text"
    bind:value={form.theme_catalog_root_path}
    data-i18n-placeholder="placeholder_theme_catalog_root_path"
    placeholder="Optional: custom theme catalog directory" />

  <label for="hold_follow_mode" class="label-with-tip">
    <span data-i18n="label_hold_follow_mode">Follow Strategy</span>
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

  <label for="overlay_target_fps" class="label-with-tip">
    <span data-i18n="label_overlay_target_fps">Overlay Target FPS</span>
    <span
      class="info-badge"
      data-i18n-title="tip_overlay_target_fps"
      title="0 means auto (follow display max refresh). Positive value caps FPS."
    >!</span>
  </label>
  <input
    id="overlay_target_fps"
    type="number"
    min={overlayFpsMin}
    max={overlayFpsMax}
    step={overlayFpsStep}
    bind:value={form.overlay_target_fps}
    data-i18n-placeholder="placeholder_overlay_target_fps"
    placeholder="0 = Auto(Max), for example 144" />
</div>
