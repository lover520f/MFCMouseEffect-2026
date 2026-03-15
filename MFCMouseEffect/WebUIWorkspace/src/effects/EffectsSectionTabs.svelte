<script>
  import { createEventDispatcher } from "svelte";
  import ActiveEffectsFields from "./ActiveEffectsFields.svelte";
  import EffectSizeFields from "./EffectSizeFields.svelte";
  import EffectConflictPolicyFields from "./EffectConflictPolicyFields.svelte";
  import EffectsBlacklistFields from "./EffectsBlacklistFields.svelte";

  export let effectProps = {};
  export let activeTab = "active";

  const dispatch = createEventDispatcher();

  const TAB_ACTIVE = "active";
  const TAB_TEXT = "text";
  const TAB_TRAIL = "trail";
  const TAB_SIZE = "size";
  const TAB_CONFLICT = "conflict";
  const TAB_BLACKLIST = "blacklist";
  const TAB_PLUGIN = "plugin";

  function normalizeTab(tabId) {
    if (tabId === TAB_TEXT) {
      return TAB_TEXT;
    }
    if (tabId === TAB_TRAIL) {
      return TAB_TRAIL;
    }
    if (tabId === TAB_SIZE) {
      return TAB_SIZE;
    }
    if (tabId === TAB_CONFLICT) {
      return TAB_CONFLICT;
    }
    if (tabId === TAB_BLACKLIST) {
      return TAB_BLACKLIST;
    }
    if (tabId === TAB_PLUGIN) {
      return TAB_PLUGIN;
    }
    return TAB_ACTIVE;
  }

  let selectedTab = normalizeTab(activeTab);
  let lastActiveTabProp = activeTab;

  // Sync local UI state only when parent prop actually changes.
  $: if (activeTab !== lastActiveTabProp) {
    lastActiveTabProp = activeTab;
    selectedTab = normalizeTab(activeTab);
  }

  function selectTab(tabId) {
    const normalized = normalizeTab(tabId);
    selectedTab = normalized;
    dispatch("tabChange", { tabId: normalized });
  }

  // Reactive derived booleans – Svelte tracks these automatically.
  $: isActiveTab = selectedTab === TAB_ACTIVE;
  $: isTextTab = selectedTab === TAB_TEXT;
  $: isTrailTab = selectedTab === TAB_TRAIL;
  $: isSizeTab = selectedTab === TAB_SIZE;
  $: isConflictTab = selectedTab === TAB_CONFLICT;
  $: isBlacklistTab = selectedTab === TAB_BLACKLIST;
  $: isPluginTab = selectedTab === TAB_PLUGIN;

  function handleActiveEffectChange(event) {
    dispatch("activeChange", event?.detail || {});
  }

  function handleSizeScaleChange(event) {
    dispatch("sizeChange", event?.detail || {});
  }

  function handleConflictPolicyChange(event) {
    dispatch("conflictPolicyChange", event?.detail || {});
  }

  function handleBlacklistChange(event) {
    dispatch("blacklistChange", event?.detail || {});
  }

  function normalizeEffectProps(input) {
    const value = input || {};
    return {
      clickOptions: value.clickOptions || [],
      trailOptions: value.trailOptions || [],
      scrollOptions: value.scrollOptions || [],
      holdOptions: value.holdOptions || [],
      hoverOptions: value.hoverOptions || [],
      effectCapabilities: value.effectCapabilities || {},
      active: value.active || {},
      effectsProfile: value.effectsProfile || {},
      showEffectsProfile: !!value.showEffectsProfile,
      effectSizeScales: value.effectSizeScales || {},
      effectConflictPolicy: value.effectConflictPolicy || {},
      effectConflictPolicyOptions: value.effectConflictPolicyOptions || {},
      effectsBlacklistApps: value.effectsBlacklistApps || [],
      platform: value.platform || 'windows',
    };
  }

  $: normalizedEffectProps = normalizeEffectProps(effectProps);
</script>

<div class="effects-subtabs">
  <div
    class="effects-subtabs-bar"
    role="tablist"
    aria-label="Effects sub sections"
  >
    <button
      type="button"
      role="tab"
      class="effects-subtab-btn"
      class:is-active={isActiveTab}
      aria-selected={isActiveTab ? "true" : "false"}
      data-i18n="tab_effects_channel"
      on:click={() => selectTab(TAB_ACTIVE)}
    >
      Effect Channel
    </button>
    <button
      type="button"
      role="tab"
      class="effects-subtab-btn"
      class:is-active={isTextTab}
      aria-selected={isTextTab ? "true" : "false"}
      data-i18n="tab_text_config"
      on:click={() => selectTab(TAB_TEXT)}
    >
      Text Config
    </button>
    <button
      type="button"
      role="tab"
      class="effects-subtab-btn"
      class:is-active={isTrailTab}
      aria-selected={isTrailTab ? "true" : "false"}
      data-i18n="section_trail_tuning"
      on:click={() => selectTab(TAB_TRAIL)}
    >
      Trail Tuning
    </button>
    <button
      type="button"
      role="tab"
      class="effects-subtab-btn"
      class:is-active={isSizeTab}
      aria-selected={isSizeTab ? "true" : "false"}
      data-i18n="tab_effect_size"
      on:click={() => selectTab(TAB_SIZE)}
    >
      Effect Size
    </button>
    <button
      type="button"
      role="tab"
      class="effects-subtab-btn"
      class:is-active={isConflictTab}
      aria-selected={isConflictTab ? "true" : "false"}
      data-i18n="tab_effect_conflict_policy"
      on:click={() => selectTab(TAB_CONFLICT)}
    >
      Hold-Move Policy
    </button>
    <button
      type="button"
      role="tab"
      class="effects-subtab-btn"
      class:is-active={isBlacklistTab}
      aria-selected={isBlacklistTab ? "true" : "false"}
      data-i18n="tab_effects_blacklist"
      on:click={() => selectTab(TAB_BLACKLIST)}
    >
      Effect Blacklist
    </button>
    <button
      type="button"
      role="tab"
      class="effects-subtab-btn"
      class:is-active={isPluginTab}
      aria-selected={isPluginTab ? "true" : "false"}
      data-i18n="tab_effects_plugin"
      on:click={() => selectTab(TAB_PLUGIN)}
    >
      Effect Plugins
    </button>
  </div>

  <div
    class="effects-subtab-panel"
    role="tabpanel"
    style:display={isActiveTab ? "" : "none"}
    aria-label="active-effects"
  >
    <ActiveEffectsFields
      clickOptions={normalizedEffectProps.clickOptions}
      trailOptions={normalizedEffectProps.trailOptions}
      scrollOptions={normalizedEffectProps.scrollOptions}
      holdOptions={normalizedEffectProps.holdOptions}
      hoverOptions={normalizedEffectProps.hoverOptions}
      effectCapabilities={normalizedEffectProps.effectCapabilities}
      active={normalizedEffectProps.active}
      effectsProfile={normalizedEffectProps.effectsProfile}
      showEffectsProfile={normalizedEffectProps.showEffectsProfile}
      on:change={handleActiveEffectChange}
    />
  </div>

  <div
    class="effects-subtab-panel"
    role="tabpanel"
    style:display={isTextTab ? "" : "none"}
    aria-label="text-content"
  >
    <div id="text_settings_mount"></div>
  </div>

  <div
    class="effects-subtab-panel"
    role="tabpanel"
    style:display={isTrailTab ? "" : "none"}
    aria-label="trail-tuning"
  >
    <div id="trail_settings_mount"></div>
  </div>

  <div
    class="effects-subtab-panel"
    role="tabpanel"
    style:display={isSizeTab ? "" : "none"}
    aria-label="effect-size"
  >
    <EffectSizeFields
      scales={normalizedEffectProps.effectSizeScales}
      on:change={handleSizeScaleChange}
    />
  </div>

  <div
    class="effects-subtab-panel"
    role="tabpanel"
    style:display={isConflictTab ? "" : "none"}
    aria-label="effect-conflict-policy"
  >
    <EffectConflictPolicyFields
      policy={normalizedEffectProps.effectConflictPolicy}
      options={normalizedEffectProps.effectConflictPolicyOptions}
      on:change={handleConflictPolicyChange}
    />
  </div>

  <div
    class="effects-subtab-panel"
    role="tabpanel"
    style:display={isBlacklistTab ? "" : "none"}
    aria-label="effect-blacklist"
  >
    <EffectsBlacklistFields
      apps={normalizedEffectProps.effectsBlacklistApps}
      platform={normalizedEffectProps.platform}
      on:change={handleBlacklistChange}
    />
  </div>

  <div
    class="effects-subtab-panel"
    role="tabpanel"
    style:display={isPluginTab ? "" : "none"}
    aria-label="effect-plugin"
  >
    <div id="wasm_settings_mount"></div>
  </div>
</div>

<style>
  .effects-subtabs {
    display: grid;
    gap: 14px;
  }

  .effects-subtabs-bar {
    display: flex;
    gap: 2px;
    border-bottom: 1px solid rgba(160, 185, 215, 0.35);
    padding-bottom: 0;
  }

  .effects-subtab-btn {
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
    transition:
      color 150ms ease,
      border-color 150ms ease;
  }

  .effects-subtab-btn:hover {
    color: rgba(34, 59, 92, 0.88);
  }

  .effects-subtab-btn.is-active {
    color: rgba(13, 90, 168, 0.95);
    border-bottom-color: rgba(13, 90, 168, 0.85);
    font-weight: 700;
  }
</style>
