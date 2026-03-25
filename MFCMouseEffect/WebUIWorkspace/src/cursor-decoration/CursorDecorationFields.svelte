<script>
  import { createEventDispatcher } from 'svelte';

  export let pluginOptions = [];
  export let decoration = {};

  const dispatch = createEventDispatcher();

  function toNumber(value, fallback) {
    const parsed = Number(value);
    return Number.isFinite(parsed) ? parsed : fallback;
  }

  function normalizeDecoration(input) {
    const value = input || {};
    return {
      enabled: value.enabled === true,
      plugin_id: `${value.plugin_id || 'ring'}`.trim() || 'ring',
      color_hex: `${value.color_hex || '#ff5a5a'}`.trim() || '#ff5a5a',
      size_px: toNumber(value.size_px, 22),
      alpha_percent: toNumber(value.alpha_percent, 82),
    };
  }

  function sameDecoration(left, right) {
    const a = normalizeDecoration(left);
    const b = normalizeDecoration(right);
    return a.enabled === b.enabled
      && a.plugin_id === b.plugin_id
      && a.color_hex === b.color_hex
      && a.size_px === b.size_px
      && a.alpha_percent === b.alpha_percent;
  }

  let form = normalizeDecoration(decoration);
  let lastDecorationRef = decoration;
  let lastEmitted = normalizeDecoration(decoration);

  $: if (decoration !== lastDecorationRef) {
    lastDecorationRef = decoration;
    form = normalizeDecoration(decoration);
    lastEmitted = normalizeDecoration(decoration);
  }

  function emitChangeIfNeeded() {
    const nextValue = normalizeDecoration(form);
    if (sameDecoration(lastEmitted, nextValue)) {
      return;
    }
    lastEmitted = nextValue;
    dispatch('change', nextValue);
  }
</script>

<div class="grid cursor-decoration-grid" on:input={emitChangeIfNeeded} on:change={emitChangeIfNeeded}>
  <label class="cursor-decoration-toggle" for="cursor_decoration_enabled">
    <span data-i18n="label_cursor_decoration_enabled">Enable cursor decoration</span>
    <input id="cursor_decoration_enabled" type="checkbox" bind:checked={form.enabled} />
  </label>
  <div></div>

  <label for="cursor_decoration_plugin" data-i18n="label_cursor_decoration_plugin">Plugin</label>
  <select id="cursor_decoration_plugin" bind:value={form.plugin_id} disabled={!form.enabled}>
    {#each pluginOptions as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  <label for="cursor_decoration_color" data-i18n="label_cursor_decoration_color">Accent color</label>
  <input id="cursor_decoration_color" type="color" bind:value={form.color_hex} disabled={!form.enabled} />

  <label for="cursor_decoration_size" data-i18n="label_cursor_decoration_size">Decoration size (px)</label>
  <input id="cursor_decoration_size" type="number" min="12" max="72" bind:value={form.size_px} disabled={!form.enabled} />

  <label for="cursor_decoration_alpha" data-i18n="label_cursor_decoration_alpha">Opacity (%)</label>
  <input id="cursor_decoration_alpha" type="number" min="15" max="100" bind:value={form.alpha_percent} disabled={!form.enabled} />

  <div class="hint span2" data-i18n="hint_cursor_decoration">
    Cursor decoration is an additive plugin lane. It stays attached to the cursor head and can coexist with the five main cursor effects.
  </div>
</div>

<style>
  .cursor-decoration-grid {
    align-items: center;
  }

  .cursor-decoration-toggle {
    display: inline-flex;
    align-items: center;
    gap: 10px;
    color: rgba(69, 86, 116, 0.96);
    font-size: 14px;
    font-weight: 600;
    line-height: 1.2;
  }

  .cursor-decoration-toggle input {
    margin: 0;
  }
</style>
