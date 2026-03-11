<script>
  import { createEventDispatcher } from 'svelte';
  import InputIndicatorBasicFields from './InputIndicatorBasicFields.svelte';
  import InputIndicatorPluginMenu from './InputIndicatorPluginMenu.svelte';

  export let positionModes = [];
  export let targetMonitorOptions = [];
  export let keyDisplayModes = [];
  export let keyLabelLayoutModes = [];
  export let monitors = [];
  export let monitorOverrides = {};
  export let monitorRows = [];
  export let form = {};
  export let wasmState = {};
  export let onWasmAction = null;
  export let texts = {};
  export let activeTab = 'basic';

  const dispatch = createEventDispatcher();

  const TAB_BASIC = 'basic';
  const TAB_PLUGIN = 'plugin';

  function normalizeTab(tabId) {
    return tabId === TAB_PLUGIN ? TAB_PLUGIN : TAB_BASIC;
  }

  function selectTab(tabId) {
    const normalized = normalizeTab(tabId);
    selectedTab = normalized;
    dispatch('tabChange', { tabId: normalized });
  }

  function handlePluginMenuChange(event) {
    dispatch('pluginChange', event?.detail || {});
  }

  let selectedTab = normalizeTab(activeTab);
  let lastActiveTabProp = activeTab;

  $: if (activeTab !== lastActiveTabProp) {
    lastActiveTabProp = activeTab;
    selectedTab = normalizeTab(activeTab);
  }

  $: isBasicTab = selectedTab === TAB_BASIC;
  $: isPluginTab = selectedTab === TAB_PLUGIN;
</script>

<div class="indicator-subtabs">
  <div class="indicator-subtabs-bar" role="tablist" aria-label="Input indicator sub sections">
    <button
      type="button"
      role="tab"
      class="indicator-subtab-btn"
      class:is-active={isBasicTab}
      aria-selected={isBasicTab ? 'true' : 'false'}
      data-i18n="tab_input_indicator_basic"
      on:click={() => selectTab(TAB_BASIC)}
    >
      Basic Settings
    </button>
    <button
      type="button"
      role="tab"
      class="indicator-subtab-btn"
      class:is-active={isPluginTab}
      aria-selected={isPluginTab ? 'true' : 'false'}
      data-i18n="tab_input_indicator_plugin"
      on:click={() => selectTab(TAB_PLUGIN)}
    >
      Plugin Override
    </button>
  </div>

  <div class="indicator-subtab-panel" role="tabpanel" style:display={isBasicTab ? '' : 'none'}>
    <InputIndicatorBasicFields
      positionModes={positionModes}
      targetMonitorOptions={targetMonitorOptions}
      keyDisplayModes={keyDisplayModes}
      keyLabelLayoutModes={keyLabelLayoutModes}
      monitors={monitors}
      monitorOverrides={monitorOverrides}
      bind:monitorRows={monitorRows}
      form={form}
      texts={texts}
    />
  </div>

  <div class="indicator-subtab-panel" role="tabpanel" style:display={isPluginTab ? '' : 'none'}>
    <InputIndicatorPluginMenu
      pluginEnabled={form.render_mode === 'wasm'}
      fallbackToNative={form.wasm_fallback_to_native}
      manifestPath={form.wasm_manifest_path}
      wasmState={wasmState}
      texts={texts}
      onAction={onWasmAction}
      on:change={handlePluginMenuChange}
    />
  </div>
</div>

<style>
  .indicator-subtabs {
    display: grid;
    gap: 14px;
    grid-column: 1 / -1;
  }

  .indicator-subtabs-bar {
    display: flex;
    gap: 2px;
    border-bottom: 1px solid rgba(160, 185, 215, 0.35);
    padding-bottom: 0;
  }

  .indicator-subtab-btn {
    border: none;
    border-bottom: 2px solid transparent;
    border-radius: 0;
    background: transparent;
    color: rgba(34, 59, 92, 0.6);
    font-size: 13px;
    font-weight: 600;
    line-height: 1.25;
    padding: 8px 16px;
    cursor: pointer;
    position: relative;
    transition: color 150ms ease, border-color 150ms ease;
  }

  .indicator-subtab-btn:hover {
    color: rgba(34, 59, 92, 0.88);
  }

  .indicator-subtab-btn.is-active {
    color: rgba(13, 90, 168, 0.95);
    border-bottom-color: rgba(13, 90, 168, 0.85);
    font-weight: 700;
  }
 </style>
