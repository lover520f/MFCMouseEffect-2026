<script>
  import { createEventDispatcher } from 'svelte';

  export let clickOptions = [];
  export let trailOptions = [];
  export let scrollOptions = [];
  export let holdOptions = [];
  export let hoverOptions = [];
  export let cursorDecorationOptions = [];
  export let cursorDecoration = {};
  export let effectCapabilities = {};
  export let active = {};
  export let effectsProfile = {};
  export let showEffectsProfile = false;

  const dispatch = createEventDispatcher();

  function normalizeActive(input) {
    const value = input || {};
    return {
      click: value.click || '',
      trail: value.trail || '',
      scroll: value.scroll || '',
      hold: value.hold || '',
      hover: value.hover || '',
    };
  }

  function toSnapshot(form) {
    const value = form || normalizeActive({});
    return {
      click: value.click || '',
      trail: value.trail || '',
      scroll: value.scroll || '',
      hold: value.hold || '',
      hover: value.hover || '',
    };
  }

  function sameActiveSnapshot(left, right) {
    const a = toSnapshot(left);
    const b = toSnapshot(right);
    return a.click === b.click
      && a.trail === b.trail
      && a.scroll === b.scroll
      && a.hold === b.hold
      && a.hover === b.hover;
  }

  function normalizeCursorDecoration(input) {
    const value = input || {};
    return {
      enabled: value.enabled === true,
      plugin_id: `${value.plugin_id || 'ring'}`.trim() || 'ring',
      color_hex: `${value.color_hex || '#ff5a5a'}`.trim() || '#ff5a5a',
      size_px: Number.isFinite(Number(value.size_px)) ? Number(value.size_px) : 22,
      alpha_percent: Number.isFinite(Number(value.alpha_percent)) ? Number(value.alpha_percent) : 82,
    };
  }

  function normalizeCursorDecorationChannelValue(input) {
    const value = normalizeCursorDecoration(input);
    return value.enabled ? value.plugin_id : '__disabled__';
  }

  function sameCursorDecoration(left, right) {
    const a = normalizeCursorDecoration(left);
    const b = normalizeCursorDecoration(right);
    return a.enabled === b.enabled
      && a.plugin_id === b.plugin_id
      && a.color_hex === b.color_hex
      && a.size_px === b.size_px
      && a.alpha_percent === b.alpha_percent;
  }

  function isSupported(key) {
    const value = effectCapabilities || {};
    return value[key] !== false;
  }

  const effectKeys = ['click', 'trail', 'scroll', 'hold', 'hover'];
  const cursorDecorationDisabledOption = { value: '__disabled__', label: 'Disabled' };
  $: unsupportedEffects = effectKeys.filter((key) => !isSupported(key));
  $: hasEffectsProfile = !!effectsProfile && typeof effectsProfile === 'object' && Object.keys(effectsProfile).length > 0;
  $: effectsProfileText = hasEffectsProfile ? JSON.stringify(effectsProfile, null, 2) : '';
  $: normalizedCursorDecoration = normalizeCursorDecoration(cursorDecoration);
  $: decorationChannelValue = normalizeCursorDecorationChannelValue(normalizedCursorDecoration);
  $: decorationChannelOptions = [cursorDecorationDisabledOption, ...(cursorDecorationOptions || [])];

  async function copyEffectsProfile() {
    if (!effectsProfileText) {
      return;
    }
    try {
      if (typeof navigator !== 'undefined' && navigator.clipboard && navigator.clipboard.writeText) {
        await navigator.clipboard.writeText(effectsProfileText);
      }
    } catch (_) {
      // best effort copy only
    }
  }

  let form = normalizeActive(active);
  let lastActiveRef = active;
  let lastCursorDecorationRef = cursorDecoration;
  let lastEmittedActive = toSnapshot(active);
  let lastEmittedCursorDecoration = normalizeCursorDecoration(cursorDecoration);

  $: if (active !== lastActiveRef) {
    lastActiveRef = active;
    form = normalizeActive(active);
    lastEmittedActive = toSnapshot(active);
  }

  $: if (cursorDecoration !== lastCursorDecorationRef) {
    lastCursorDecorationRef = cursorDecoration;
    decorationChannelValue = normalizeCursorDecorationChannelValue(cursorDecoration);
    lastEmittedCursorDecoration = normalizeCursorDecoration(cursorDecoration);
  }

  function emitCombinedChangeIfNeeded(nextDecoration = normalizedCursorDecoration) {
    const nextActive = toSnapshot(form);
    if (
      sameActiveSnapshot(lastEmittedActive, nextActive)
      && sameCursorDecoration(lastEmittedCursorDecoration, nextDecoration)
    ) {
      return;
    }
    lastEmittedActive = nextActive;
    lastEmittedCursorDecoration = normalizeCursorDecoration(nextDecoration);
    dispatch('change', {
      active: nextActive,
      cursor_decoration: lastEmittedCursorDecoration,
    });
  }

  function handleDecorationChannelChange(nextValue) {
    const normalizedValue = `${nextValue || ''}`.trim();
    const nextDecoration =
      normalizedValue && normalizedValue !== '__disabled__'
        ? {
            ...normalizedCursorDecoration,
            enabled: true,
            plugin_id: normalizedValue,
          }
        : {
            ...normalizedCursorDecoration,
            enabled: false,
          };
    emitCombinedChangeIfNeeded(nextDecoration);
  }
</script>

<div class="grid">
  <label for="click" data-i18n="label_click">Click</label>
  <select id="click" bind:value={form.click} disabled={!isSupported('click')} on:change={() => emitCombinedChangeIfNeeded()}>
    {#each clickOptions as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  <label for="trail" data-i18n="label_trail">Trail</label>
  <select id="trail" bind:value={form.trail} disabled={!isSupported('trail')} on:change={() => emitCombinedChangeIfNeeded()}>
    {#each trailOptions as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  <label for="scroll" data-i18n="label_scroll">Scroll</label>
  <select id="scroll" bind:value={form.scroll} disabled={!isSupported('scroll')} on:change={() => emitCombinedChangeIfNeeded()}>
    {#each scrollOptions as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  <label for="hold" data-i18n="label_hold">Hold</label>
  <select id="hold" bind:value={form.hold} disabled={!isSupported('hold')} on:change={() => emitCombinedChangeIfNeeded()}>
    {#each holdOptions as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  <label for="hover" data-i18n="label_hover">Hover</label>
  <select id="hover" bind:value={form.hover} disabled={!isSupported('hover')} on:change={() => emitCombinedChangeIfNeeded()}>
    {#each hoverOptions as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  <label for="cursor_decoration_channel" data-i18n="section_cursor_decoration">Cursor Decoration</label>
  <select
    id="cursor_decoration_channel"
    bind:value={decorationChannelValue}
    on:change={(event) => handleDecorationChannelChange(event.currentTarget?.value)}>
    {#each decorationChannelOptions as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  {#if unsupportedEffects.length > 0}
    <div class="hint span2" data-i18n="hint_effect_capability_limited">
      Some effect categories are unavailable on this platform and have been disabled.
    </div>
  {/if}

  <div class="hint span2" data-i18n="hint_cursor_decoration_channel">
    Cursor decoration is the sixth additive built-in lane. Use the plugin tab below for color, size, and opacity.
  </div>
</div>

{#if showEffectsProfile && hasEffectsProfile}
  <div class="hint span2 effects-profile-box">
    <div class="effects-profile-header">
      <div class="effects-profile-title" data-i18n="label_effects_runtime_profile">Effects Runtime Profile</div>
      <button
        type="button"
        class="effects-profile-copy-btn"
        data-i18n="btn_copy_effects_profile"
        on:click={copyEffectsProfile}>Copy JSON</button>
    </div>
    <pre class="effects-profile-content">{effectsProfileText}</pre>
  </div>
{/if}

<style>
  .effects-profile-box {
    margin-top: 10px;
    padding: 10px 12px;
    border: 1px dashed rgba(80, 120, 180, 0.35);
    border-radius: 10px;
    background: rgba(120, 150, 190, 0.06);
  }

  .effects-profile-title {
    font-size: 12px;
    font-weight: 700;
    color: rgba(20, 40, 66, 0.88);
  }

  .effects-profile-header {
    margin-bottom: 8px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 8px;
  }

  .effects-profile-copy-btn {
    border: 1px solid rgba(70, 108, 160, 0.35);
    border-radius: 8px;
    padding: 2px 8px;
    background: rgba(82, 126, 184, 0.1);
    color: rgba(27, 58, 101, 0.9);
    font-size: 11px;
    line-height: 1.5;
    cursor: pointer;
  }

  .effects-profile-content {
    margin: 0;
    max-height: 240px;
    overflow: auto;
    font-size: 11px;
    line-height: 1.45;
    color: rgba(22, 36, 56, 0.88);
    white-space: pre-wrap;
    word-break: break-word;
  }
</style>
