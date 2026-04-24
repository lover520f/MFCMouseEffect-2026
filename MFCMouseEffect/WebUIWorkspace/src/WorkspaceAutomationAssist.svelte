<script>
  import AutomationSidebarDebugCard from './AutomationSidebarDebugCard.svelte';
  import { normalizeGestureRouteStatus } from './automation/gesture-route-debug-model.js';

  export let activeSectionId = '';
  export let runtimePlatform = 'windows';
  export let runtimeState = {};
  export let i18n = {};

  let routeStatus = null;
  let panelOpen = false;

  function t(key, fallback) {
    const value = `${i18n?.[key] || ''}`.trim();
    return value || fallback;
  }

  $: routeStatus = normalizeGestureRouteStatus(runtimeState);
</script>

{#if activeSectionId === 'automation' && routeStatus}
  <section class="automation-debug-panel">
    <button
      type="button"
      class="automation-debug-panel__toggle"
      aria-expanded={panelOpen ? 'true' : 'false'}
      on:click={() => {
        panelOpen = !panelOpen;
      }}
    >
      <div class="automation-debug-panel__copy">
        <div class="automation-debug-panel__eyebrow">{t('label_auto_gesture_debug', '手势实时调试（Debug）')}</div>
        <div class="automation-debug-panel__meta">
          <span class="automation-debug-panel__chip">
            {t('label_auto_gesture_debug_last_stage', '阶段')} {routeStatus.lastStage || '-'}
          </span>
          <span class="automation-debug-panel__chip automation-debug-panel__chip--muted">
            {t('label_auto_gesture_debug_last_reason', '原因')} {routeStatus.lastReason || '-'}
          </span>
        </div>
      </div>
      <span class="automation-debug-panel__action">
        {panelOpen
          ? t('btn_auto_gesture_debug_collapse', '收起调试面板')
          : t('btn_auto_gesture_debug_expand', '展开调试面板')}
      </span>
    </button>

    {#if panelOpen}
      <div class="automation-debug-panel__body">
        <AutomationSidebarDebugCard
          compact={true}
          payloadState={runtimeState}
          platform={runtimePlatform}
          {i18n}
        />
      </div>
    {/if}
  </section>
{/if}

<style>
  .automation-debug-panel {
    margin-top: 10px;
  }

  .automation-debug-panel__toggle {
    width: 100%;
    border: 1px solid rgba(194, 208, 226, 0.92);
    border-radius: 14px;
    background: linear-gradient(180deg, rgba(244, 249, 255, 0.98), rgba(235, 244, 255, 0.96));
    box-shadow: 0 8px 20px rgba(53, 86, 124, 0.08);
    padding: 12px 14px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 14px;
    text-align: left;
    cursor: pointer;
  }

  .automation-debug-panel__copy {
    min-width: 0;
    flex: 1 1 auto;
  }

  .automation-debug-panel__eyebrow {
    font-size: 12px;
    font-weight: 780;
    color: #244566;
  }

  .automation-debug-panel__meta {
    margin-top: 6px;
    display: flex;
    flex-wrap: wrap;
    gap: 8px;
  }

  .automation-debug-panel__chip {
    min-width: 0;
    border: 1px solid rgba(183, 205, 233, 0.92);
    border-radius: 999px;
    padding: 4px 10px;
    font-size: 11px;
    line-height: 1.35;
    color: #34597d;
    background: rgba(255, 255, 255, 0.9);
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
  }

  .automation-debug-panel__chip--muted {
    color: #4e6d8e;
    background: rgba(247, 251, 255, 0.96);
  }

  .automation-debug-panel__action {
    flex: 0 0 auto;
    font-size: 12px;
    font-weight: 700;
    color: #2f6fb8;
    white-space: nowrap;
  }

  .automation-debug-panel__body {
    margin-top: 10px;
  }

  @media (max-width: 860px) {
    .automation-debug-panel__toggle {
      align-items: flex-start;
      flex-direction: column;
    }

    .automation-debug-panel__action {
      white-space: normal;
    }
  }
</style>
