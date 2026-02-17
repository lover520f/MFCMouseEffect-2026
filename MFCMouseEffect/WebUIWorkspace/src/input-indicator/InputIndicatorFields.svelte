<script>
  import { createEventDispatcher } from 'svelte';

  export let positionModes = [];
  export let targetMonitorOptions = [];
  export let keyDisplayModes = [];
  export let keyLabelLayoutModes = [];
  export let monitors = [];
  export let monitorOverrides = {};
  export let indicator = {};
  export let texts = {};

  const dispatch = createEventDispatcher();

  function num(value, fallback) {
    const parsed = Number(value);
    if (Number.isFinite(parsed)) return parsed;
    return fallback;
  }

  function normalizeIndicator(input) {
    const value = input || {};
    return {
      enabled: value.enabled !== false,
      keyboard_enabled: value.keyboard_enabled !== false,
      position_mode: value.position_mode || 'relative',
      offset_x: num(value.offset_x, 40),
      offset_y: num(value.offset_y, 40),
      absolute_x: num(value.absolute_x, 40),
      absolute_y: num(value.absolute_y, 40),
      target_monitor: value.target_monitor || 'cursor',
      key_display_mode: value.key_display_mode || 'all',
      key_label_layout_mode: value.key_label_layout_mode || 'fixed_font',
      size_px: num(value.size_px, 110),
      duration_ms: num(value.duration_ms, 320),
    };
  }

  function buildMonitorLabel(monitor, index, i18n) {
    const monLabel = i18n.pm_monitor || 'Monitor';
    const primaryBadge = i18n.pm_primary_badge || 'Primary';
    const width = (monitor.right || 0) - (monitor.left || 0);
    const height = (monitor.bottom || 0) - (monitor.top || 0);
    const resolution = (width > 0 && height > 0) ? ` (${width}x${height})` : '';
    const badge = monitor.is_primary ? ` * ${primaryBadge}` : '';
    return `${monLabel} ${index + 1}${resolution}${badge}`;
  }

  function buildMonitorRows(monitorsList, overridesMap, i18n) {
    const rows = [];
    if (!Array.isArray(monitorsList)) return rows;
    for (let idx = 0; idx < monitorsList.length; idx += 1) {
      const monitor = monitorsList[idx];
      const override = overridesMap ? overridesMap[monitor.id] : null;
      rows.push({
        id: monitor.id,
        label: buildMonitorLabel(monitor, idx, i18n || {}),
        enabled: !!(override && override.enabled),
        absolute_x: num(override && override.absolute_x, 40),
        absolute_y: num(override && override.absolute_y, 40),
      });
    }
    return rows;
  }

  function toMonitorOverrideMap(rows) {
    const output = {};
    for (const row of rows || []) {
      output[row.id] = {
        enabled: !!row.enabled,
        absolute_x: num(row.absolute_x, 40),
        absolute_y: num(row.absolute_y, 40),
      };
    }
    return output;
  }

  function toStateSnapshot(base, rows) {
    const current = base || normalizeIndicator({});
    return {
      enabled: !!current.enabled,
      keyboard_enabled: !!current.keyboard_enabled,
      position_mode: current.position_mode || 'relative',
      offset_x: num(current.offset_x, 40),
      offset_y: num(current.offset_y, 40),
      absolute_x: num(current.absolute_x, 40),
      absolute_y: num(current.absolute_y, 40),
      target_monitor: current.target_monitor || 'cursor',
      key_display_mode: current.key_display_mode || 'all',
      key_label_layout_mode: current.key_label_layout_mode || 'fixed_font',
      size_px: num(current.size_px, 110),
      duration_ms: num(current.duration_ms, 320),
      per_monitor_overrides: toMonitorOverrideMap(rows),
    };
  }

  let form = normalizeIndicator(indicator);
  let monitorRows = buildMonitorRows(monitors, monitorOverrides, texts);

  let lastIndicatorRef = indicator;
  let lastMonitorsRef = monitors;
  let lastOverridesRef = monitorOverrides;
  let lastTextsRef = texts;

  $: if (indicator !== lastIndicatorRef) {
    lastIndicatorRef = indicator;
    form = normalizeIndicator(indicator);
  }

  $: if (monitors !== lastMonitorsRef || monitorOverrides !== lastOverridesRef || texts !== lastTextsRef) {
    lastMonitorsRef = monitors;
    lastOverridesRef = monitorOverrides;
    lastTextsRef = texts;
    monitorRows = buildMonitorRows(monitors, monitorOverrides, texts);
  }

  $: dispatch('change', toStateSnapshot(form, monitorRows));
</script>

<div class="grid">
  <label for="ii_enabled" data-i18n="label_input_indicator_enabled">Enable indicator</label>
  <input id="ii_enabled" type="checkbox" bind:checked={form.enabled} />

  <label for="ii_keyboard_enabled" data-i18n="label_input_indicator_keyboard_enabled">Enable keyboard indicator</label>
  <input id="ii_keyboard_enabled" type="checkbox" bind:checked={form.keyboard_enabled} />

  <label for="ii_position_mode" data-i18n="label_input_indicator_position_mode">Position mode</label>
  <select id="ii_position_mode" bind:value={form.position_mode}>
    {#each positionModes as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  <label for="ii_offset_x" data-i18n="label_input_indicator_offset">Relative offset X/Y</label>
  <div class="pair" style:opacity={form.position_mode === 'relative' ? '1' : '0.45'}>
    <input id="ii_offset_x" type="number" min="-2000" max="2000" bind:value={form.offset_x} />
    <input id="ii_offset_y" type="number" min="-2000" max="2000" bind:value={form.offset_y} />
  </div>

  <label for="ii_key_display_mode" data-i18n="label_key_display_mode">Keyboard Display</label>
  <select id="ii_key_display_mode" bind:value={form.key_display_mode}>
    {#each keyDisplayModes as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  <label for="ii_key_label_layout_mode" data-i18n="label_key_label_layout_mode">Key label layout</label>
  <select id="ii_key_label_layout_mode" bind:value={form.key_label_layout_mode}>
    {#each keyLabelLayoutModes as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  <label for="ii_absolute_x" data-i18n="label_input_indicator_absolute">Absolute X/Y</label>
  <div class="pair" style:opacity={form.position_mode === 'absolute' ? '1' : '0.45'}>
    <input id="ii_absolute_x" type="number" min="-20000" max="20000" bind:value={form.absolute_x} />
    <input id="ii_absolute_y" type="number" min="-20000" max="20000" bind:value={form.absolute_y} />
  </div>

  <div class="monitor-overrides-label" data-i18n="label_per_monitor_cfg">Per-Monitor Overrides (Absolute)</div>
  <div
    id="ii_per_monitor_overrides"
    class="monitor-overrides"
    style:display={form.position_mode === 'absolute' && form.target_monitor === 'custom' ? 'block' : 'none'}
  >
    {#if !Array.isArray(monitorRows) || monitorRows.length === 0}
      <div class="pm-empty">{texts.pm_no_monitors || 'No monitors detected.'}</div>
    {:else}
      {#each monitorRows as row}
        <div class="monitor-row" class:is-disabled={!row.enabled}>
          <input
            type="checkbox"
            id={`ii_ov_${row.id}_en`}
            class="monitor-toggle"
            bind:checked={row.enabled}
          />
          <label for={`ii_ov_${row.id}_en`} class="monitor-name" title={row.label}>
            {row.label}
          </label>
          <input
            type="number"
            id={`ii_ov_${row.id}_x`}
            class="monitor-input"
            title="X"
            placeholder="X"
            bind:value={row.absolute_x}
            disabled={!row.enabled}
          />
          <input
            type="number"
            id={`ii_ov_${row.id}_y`}
            class="monitor-input"
            title="Y"
            placeholder="Y"
            bind:value={row.absolute_y}
            disabled={!row.enabled}
          />
        </div>
      {/each}
    {/if}
  </div>

  <label for="ii_target_monitor" data-i18n="label_input_indicator_target_monitor">Target monitor (mouse)</label>
  <select id="ii_target_monitor" bind:value={form.target_monitor}>
    {#each targetMonitorOptions as option}
      <option value={option.value}>{option.label}</option>
    {/each}
  </select>

  <label for="ii_size_px" data-i18n="label_input_indicator_size">Indicator size (px)</label>
  <input id="ii_size_px" type="number" min="40" max="200" bind:value={form.size_px} />

  <label for="ii_duration_ms" data-i18n="label_input_indicator_duration">Animation duration (ms)</label>
  <input id="ii_duration_ms" type="number" min="120" max="2000" bind:value={form.duration_ms} />

  <div class="hint span2" data-i18n="hint_input_indicator">
    Supports click, scroll, and keyboard display. Choose target monitor for multi-screen setups; select "Custom Multi-Screen" for simultaneous display on multiple monitors.
  </div>
</div>
