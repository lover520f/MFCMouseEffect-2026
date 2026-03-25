<script>
  import { createEventDispatcher } from "svelte";

  export let scales = {};
  export let cursorDecoration = {};

  const dispatch = createEventDispatcher();

  const rows = [
    { key: "click", labelKey: "label_click", labelDefault: "Click" },
    { key: "trail", labelKey: "label_trail", labelDefault: "Trail" },
    { key: "scroll", labelKey: "label_scroll", labelDefault: "Scroll" },
    { key: "hold", labelKey: "label_hold", labelDefault: "Hold" },
    { key: "hover", labelKey: "label_hover", labelDefault: "Hover" },
  ];

  function toNumber(value, fallback) {
    const parsed = Number(value);
    return Number.isFinite(parsed) ? parsed : fallback;
  }

  function clampPercent(value) {
    return Math.min(200, Math.max(50, Math.round(toNumber(value, 100))));
  }

  function normalizeScales(input) {
    const value = input || {};
    return {
      click: clampPercent(value.click),
      trail: clampPercent(value.trail),
      scroll: clampPercent(value.scroll),
      hold: clampPercent(value.hold),
      hover: clampPercent(value.hover),
    };
  }

  function normalizeCursorDecoration(input) {
    const value = input || {};
    return {
      enabled: value.enabled === true,
      plugin_id: `${value.plugin_id || 'ring'}`.trim() || 'ring',
      color_hex: `${value.color_hex || '#ff5a5a'}`.trim() || '#ff5a5a',
      size_px: Math.min(72, Math.max(12, Math.round(toNumber(value.size_px, 22)))),
      alpha_percent: Math.min(100, Math.max(15, Math.round(toNumber(value.alpha_percent, 82)))),
    };
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

  function toSnapshot(form) {
    const value = form || normalizeScales({});
    return {
      click: clampPercent(value.click),
      trail: clampPercent(value.trail),
      scroll: clampPercent(value.scroll),
      hold: clampPercent(value.hold),
      hover: clampPercent(value.hover),
    };
  }

  function updateScale(key, value) {
    const next = clampPercent(value);
    if (form[key] === next) {
      return;
    }
    form = {
      ...form,
      [key]: next,
    };
  }

  let form = normalizeScales(scales);
  let lastScalesRef = scales;
  let decorationForm = normalizeCursorDecoration(cursorDecoration);
  let lastCursorDecorationRef = cursorDecoration;
  let lastDecorationSnapshot = normalizeCursorDecoration(cursorDecoration);

  function toSliderFraction(value) {
    return (clampPercent(value) - 50) / 150;
  }

  $: if (scales !== lastScalesRef) {
    lastScalesRef = scales;
    form = normalizeScales(scales);
  }

  $: if (cursorDecoration !== lastCursorDecorationRef) {
    lastCursorDecorationRef = cursorDecoration;
    decorationForm = normalizeCursorDecoration(cursorDecoration);
    lastDecorationSnapshot = normalizeCursorDecoration(cursorDecoration);
  }

  $: dispatch("change", toSnapshot(form));

  function emitCursorDecorationIfNeeded() {
    const nextValue = normalizeCursorDecoration(decorationForm);
    if (sameCursorDecoration(lastDecorationSnapshot, nextValue)) {
      return;
    }
    lastDecorationSnapshot = nextValue;
    dispatch("cursorDecorationChange", nextValue);
  }
</script>

<div class="grid effect-size-grid">
  {#each rows as row}
    <label
      for={`effect_size_${row.key}`}
      class="effect-size-label"
      data-i18n={row.labelKey}>{row.labelDefault}</label
    >
    <div
      class="effect-size-slider-shell"
      style="--slider-frac:{toSliderFraction(form[row.key])}"
      title="{form[row.key]}% (50%–200%)"
    >
      <div class="effect-size-slider-track" aria-hidden="true">
        <div class="effect-size-slider-fill"></div>
      </div>
      <div class="effect-size-slider-thumb" aria-hidden="true"></div>
      <input
        id={`effect_size_${row.key}`}
        type="range"
        min="50"
        max="200"
        step="5"
        value={form[row.key]}
        aria-label={row.labelDefault}
        aria-valuemin="50"
        aria-valuemax="200"
        aria-valuenow={form[row.key]}
        on:input={(event) => updateScale(row.key, event?.target?.value)}
      />
    </div>
    <input
      id={`effect_size_${row.key}_number`}
      class="pair effect-size-number"
      type="number"
      min="50"
      max="200"
      step="5"
      value={form[row.key]}
      on:input={(event) => updateScale(row.key, event?.target?.value)}
      on:change={(event) => updateScale(row.key, event?.target?.value)}
    />
  {/each}
  <div class="hint effect-size-hint" data-i18n="hint_effect_size_scale">
    100 means default size. Range is 50% to 200%.
  </div>
</div>

<section class="cursor-decoration-config-card">
  <header class="cursor-decoration-config-header">
    <div class="cursor-decoration-config-title" data-i18n="section_cursor_decoration">
      Cursor Decoration
    </div>
    <p class="hint cursor-decoration-config-hint" data-i18n="desc_cursor_decoration">
      Attach a persistent decorator plugin to the cursor head without changing the five main cursor effects.
    </p>
  </header>

  <div class="grid cursor-decoration-config-grid">
    <label for="effect_cursor_decoration_color" data-i18n="label_cursor_decoration_color">Accent color</label>
    <input
      id="effect_cursor_decoration_color"
      class="pair"
      type="color"
      bind:value={decorationForm.color_hex}
      disabled={!decorationForm.enabled}
      on:input={emitCursorDecorationIfNeeded}
      on:change={emitCursorDecorationIfNeeded}
    />

    <label for="effect_cursor_decoration_size" data-i18n="label_cursor_decoration_size">Decoration size (px)</label>
    <input
      id="effect_cursor_decoration_size"
      class="pair"
      type="number"
      min="12"
      max="72"
      bind:value={decorationForm.size_px}
      disabled={!decorationForm.enabled}
      on:input={emitCursorDecorationIfNeeded}
      on:change={emitCursorDecorationIfNeeded}
    />

    <label for="effect_cursor_decoration_alpha" data-i18n="label_cursor_decoration_alpha">Opacity (%)</label>
    <input
      id="effect_cursor_decoration_alpha"
      class="pair"
      type="number"
      min="15"
      max="100"
      bind:value={decorationForm.alpha_percent}
      disabled={!decorationForm.enabled}
      on:input={emitCursorDecorationIfNeeded}
      on:change={emitCursorDecorationIfNeeded}
    />
  </div>
</section>

<style>
  .cursor-decoration-config-card {
    margin-top: 22px;
    padding: 18px 18px 20px;
    border: 1px solid rgba(188, 206, 228, 0.9);
    border-radius: 18px;
    background:
      linear-gradient(180deg, rgba(255, 255, 255, 0.96), rgba(246, 250, 255, 0.94));
    box-shadow: 0 10px 28px rgba(18, 30, 47, 0.06);
  }

  .cursor-decoration-config-header {
    display: flex;
    flex-direction: column;
    gap: 6px;
    margin-bottom: 16px;
  }

  .cursor-decoration-config-title {
    color: rgba(20, 42, 72, 0.92);
    font-size: 18px;
    font-weight: 700;
    line-height: 1.3;
  }

  .cursor-decoration-config-hint {
    max-width: 760px;
    margin: 0;
    line-height: 1.65;
  }

  .cursor-decoration-config-grid {
    row-gap: 14px;
    align-items: center;
  }

  .cursor-decoration-config-grid label {
    color: rgba(44, 69, 103, 0.88);
    font-weight: 650;
  }

  @media (max-width: 860px) {
    .cursor-decoration-config-card {
      padding: 16px;
    }
  }
</style>
