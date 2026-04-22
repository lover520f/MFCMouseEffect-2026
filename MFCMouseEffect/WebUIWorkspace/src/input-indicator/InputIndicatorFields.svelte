<script>
  import { createEventDispatcher } from 'svelte';
  import InputIndicatorBasicFields from './InputIndicatorBasicFields.svelte';
  import InputIndicatorSectionTabs from './InputIndicatorSectionTabs.svelte';

  export let positionModes = [];
  export let targetMonitorOptions = [];
  export let keyDisplayModes = [];
  export let keyLabelLayoutModes = [];
  export let monitors = [];
  export let monitorOverrides = {};
  export let indicator = {};
  export let wasmState = {};
  export let onWasmAction = null;
  export let texts = {};
  export let activeTab = 'basic';
  export let onChangeState = null;
  export let onTabChange = null;

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
      render_mode: value.render_mode || 'native',
      wasm_fallback_to_native: value.wasm_fallback_to_native !== false,
      wasm_manifest_path: `${value.wasm_manifest_path || ''}`.trim(),
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

  function toStateSnapshot(base, rows) {
    const current = base || normalizeIndicator({});
    return {
      enabled: !!current.enabled,
      keyboard_enabled: !!current.keyboard_enabled,
      render_mode: current.render_mode || 'native',
      wasm_fallback_to_native: current.wasm_fallback_to_native !== false,
      wasm_manifest_path: `${current.wasm_manifest_path || ''}`.trim(),
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

  function handlePluginMenuChange(event) {
    const detail = event?.detail || {};
    if (typeof detail.pluginEnabled === 'boolean') {
      form = {
        ...form,
        render_mode: detail.pluginEnabled ? 'wasm' : 'native',
      };
    }
    if (typeof detail.fallbackToNative === 'boolean') {
      form = {
        ...form,
        wasm_fallback_to_native: detail.fallbackToNative,
      };
    }
    if (typeof detail.manifestPath === 'string') {
      form = {
        ...form,
        wasm_manifest_path: detail.manifestPath.trim(),
      };
    }
  }

  function handleTabChange(event) {
    const detail = event?.detail || {};
    if (typeof onTabChange === 'function') {
      onTabChange(detail);
    }
    dispatch('tabChange', detail);
  }

  let form = normalizeIndicator(indicator);
  let monitorRows = [];

  let lastIndicatorRef = indicator;

  $: if (indicator !== lastIndicatorRef) {
    lastIndicatorRef = indicator;
    form = normalizeIndicator(indicator);
  }

  $: {
    const snapshot = toStateSnapshot(form, monitorRows);
    if (typeof onChangeState === 'function') {
      onChangeState(snapshot);
    }
    dispatch('change', snapshot);
  }
</script>

<div class="input-indicator-fields">
  <div class="input-indicator-fields__toggles">
    <label class="input-indicator-fields__toggle" for="ii_enabled">
      <span data-i18n="label_input_indicator_enabled">Enable indicator</span>
      <input id="ii_enabled" type="checkbox" bind:checked={form.enabled} />
    </label>

    <label class="input-indicator-fields__toggle" for="ii_keyboard_enabled">
      <span data-i18n="label_input_indicator_keyboard_enabled">Enable keyboard indicator</span>
      <input id="ii_keyboard_enabled" type="checkbox" bind:checked={form.keyboard_enabled} />
    </label>
  </div>

  <InputIndicatorSectionTabs
    positionModes={positionModes}
    targetMonitorOptions={targetMonitorOptions}
    keyDisplayModes={keyDisplayModes}
    keyLabelLayoutModes={keyLabelLayoutModes}
    monitors={monitors}
    monitorOverrides={monitorOverrides}
    bind:form={form}
    wasmState={wasmState}
    bind:monitorRows={monitorRows}
    onWasmAction={onWasmAction}
    texts={texts}
    activeTab={activeTab}
    on:tabChange={handleTabChange}
    on:pluginChange={handlePluginMenuChange}
  />
</div>

<style>
  .input-indicator-fields {
    display: grid;
    gap: 12px;
  }

  .input-indicator-fields__toggles {
    display: flex;
    align-items: center;
    gap: 18px;
    flex-wrap: wrap;
  }

  .input-indicator-fields__toggle {
    display: inline-flex;
    align-items: center;
    gap: 10px;
    color: rgba(69, 86, 116, 0.96);
    font-size: 14px;
    font-weight: 600;
    line-height: 1.2;
  }

  .input-indicator-fields__toggle input {
    margin: 0;
  }

  @media (max-width: 720px) {
    .input-indicator-fields__toggles {
      align-items: flex-start;
      flex-direction: column;
      gap: 10px;
    }
  }
</style>
