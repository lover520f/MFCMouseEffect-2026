<script>
  import {
    boolLabel,
    formatGestureRouteModifiers,
    gestureRouteSourceLabel,
    normalizeGestureRouteStatus,
    recentGestureRouteEvents,
    selectLatestGestureEvent,
    selectLatestMatchedGestureEvent,
    selectLatestRecognizedGestureEvent,
  } from './automation/gesture-route-debug-model.js';
  import {
    buildGesturePreviewFromPoints,
    gestureIdDirectionHint,
  } from './automation/gesture-id-preview.js';

  export let payloadState = {};
  export let platform = 'windows';
  export let i18n = {};

  let routeStatus = null;
  let recentEvents = [];
  let latestGestureEvent = null;
  let latestRecognizedGestureEvent = null;
  let latestMatchedGestureEvent = null;
  let recognizedGestureId = '';
  let matchedGestureId = '';
  let recognizedGesturePreview = null;
  let matchedGesturePreview = null;
  let recognizedGesturePreviewDisplay = null;
  let lastRecognizedPreviewPoints = [];
  let lastMatchedPreviewPoints = [];
  let recognizedGestureHint = '';
  let matchedGestureHint = '';
  let shouldResetRecognizedPreview = false;

  function t(key, fallback) {
    const value = `${i18n?.[key] || ''}`.trim();
    return value || fallback;
  }

  function shouldResetPreviewByRouteStatus(status) {
    const stage = `${status?.lastStage || ''}`.trim();
    const reason = `${status?.lastReason || ''}`.trim();
    if (stage === 'buttonless_move_skipped') {
      return reason === 'awaiting_first_motion_arm';
    }
    if (stage === 'buttonless_idle_reset' && reason === 'idle_timeout') {
      return true;
    }
    return false;
  }

  function samplePointsEvenly(points, maxPoints = 220) {
    if (!Array.isArray(points) || points.length <= 0) {
      return [];
    }
    const cap = Math.max(24, Math.min(320, Math.trunc(maxPoints)));
    if (points.length <= cap) {
      return points;
    }
    const out = [];
    const step = (points.length - 1) / (cap - 1);
    for (let i = 0; i < cap; i += 1) {
      const index = Math.min(points.length - 1, Math.round(step * i));
      out.push(points[index]);
    }
    return out;
  }

  $: routeStatus = normalizeGestureRouteStatus(payloadState);
  $: recentEvents = recentGestureRouteEvents(routeStatus, 8);
  $: latestGestureEvent = selectLatestGestureEvent(routeStatus);
  $: latestRecognizedGestureEvent = selectLatestRecognizedGestureEvent(routeStatus);
  $: latestMatchedGestureEvent = selectLatestMatchedGestureEvent(routeStatus);
  $: shouldResetRecognizedPreview = shouldResetPreviewByRouteStatus(routeStatus);
  $: recognizedGestureId = `${routeStatus?.lastRecognizedGestureId || latestRecognizedGestureEvent?.recognizedGestureId || latestGestureEvent?.gestureId || ''}`.trim();
  $: matchedGestureId = `${routeStatus?.lastMatchedGestureId || latestMatchedGestureEvent?.matchedGestureId || ''}`.trim();
  $: {
    const current = routeStatus?.lastPreviewPoints?.length >= 2
      ? routeStatus.lastPreviewPoints
      : (latestRecognizedGestureEvent?.previewPoints?.length >= 2
        ? latestRecognizedGestureEvent.previewPoints
        : []);
    if (shouldResetRecognizedPreview) {
      lastRecognizedPreviewPoints = [];
    } else if (current.length >= 2) {
      lastRecognizedPreviewPoints = current;
    }
  }
  $: {
    const current = latestMatchedGestureEvent?.previewPoints?.length >= 2
      ? latestMatchedGestureEvent.previewPoints
      : [];
    if (current.length >= 2) {
      lastMatchedPreviewPoints = current;
    }
  }
  $: recognizedGesturePreview = shouldResetRecognizedPreview
    ? null
    : buildGesturePreviewFromPoints(
      samplePointsEvenly(lastRecognizedPreviewPoints, 220),
      { width: 108, height: 52, padding: 5, fitRatio: 0.9 },
    );
  $: matchedGesturePreview = buildGesturePreviewFromPoints(
    samplePointsEvenly(lastMatchedPreviewPoints, 220),
    { width: 108, height: 52, padding: 5, fitRatio: 0.9 },
  );
  $: {
    recognizedGesturePreviewDisplay =
      recognizedGesturePreview ||
      matchedGesturePreview;
  }
  $: recognizedGestureHint = shouldResetRecognizedPreview ? '' : gestureIdDirectionHint(recognizedGestureId);
  $: matchedGestureHint = gestureIdDirectionHint(matchedGestureId);

  function displayGestureLabel(gestureHint, gestureId) {
    return gestureHint || gestureId || '-';
  }

  function sourceLabel(event) {
    if (event?.usedCustom) {
      return t('sourceCustom', 'Custom');
    }
    if (event?.usedPreset) {
      return t('sourcePreset', 'Preset');
    }
    return t('sourceUnknown', 'Unknown');
  }

  function eventLine(event) {
    if (!event) {
      return '-';
    }
    const stage = event.stage || '-';
    const reason = event.reason || '-';
    return `${stage} · ${reason}`;
  }
</script>

{#if routeStatus}
  <section class="workspace-debug-card" aria-live="polite">
    <div class="workspace-debug-title">{t('label_auto_gesture_debug', '手势实时调试（Debug）')}</div>
    <div class="workspace-debug-grid">
      <div class="workspace-debug-item">
        <span>{t('label_auto_gesture_debug_last_stage', '阶段')}</span>
        <strong>{routeStatus.lastStage || '-'}</strong>
      </div>
      <div class="workspace-debug-item">
        <span>{t('label_auto_gesture_debug_last_reason', '原因')}</span>
        <strong>{routeStatus.lastReason || '-'}</strong>
      </div>
      <div class="workspace-debug-item">
        <span>{t('label_auto_gesture_debug_last_gesture', '识别手势')}</span>
        <div class="workspace-debug-gesture">
          {#if recognizedGesturePreviewDisplay}
            <svg
              class="workspace-debug-gesture__svg"
              viewBox={`0 0 ${recognizedGesturePreviewDisplay.width} ${recognizedGesturePreviewDisplay.height}`}
              aria-hidden="true"
            >
              <path
                class="workspace-debug-gesture__grid"
                fill="none"
                stroke="rgba(157, 186, 219, 0.45)"
                stroke-width="1"
                stroke-dasharray="2 2"
                d={`M 4 ${(recognizedGesturePreviewDisplay.height / 2).toFixed(1)} H ${(recognizedGesturePreviewDisplay.width - 4).toFixed(1)} M ${(recognizedGesturePreviewDisplay.width / 2).toFixed(1)} 3 V ${(recognizedGesturePreviewDisplay.height - 3).toFixed(1)}`}
              />
              <path
                class="workspace-debug-gesture__path"
                fill="none"
                stroke="#2f7ed8"
                stroke-width="2.1"
                stroke-linecap="round"
                stroke-linejoin="round"
                d={recognizedGesturePreviewDisplay.path}
              />
              {#if recognizedGesturePreviewDisplay.startPoint}
                <circle
                  class="workspace-debug-gesture__start"
                  cx={recognizedGesturePreviewDisplay.startPoint.x}
                  cy={recognizedGesturePreviewDisplay.startPoint.y}
                  r="2.3"
                  fill="#ebf4ff"
                  stroke="#2f7ed8"
                  stroke-width="1.5"
                />
              {/if}
              {#if recognizedGesturePreviewDisplay.arrowPath}
                <path
                  class="workspace-debug-gesture__arrow"
                  fill="#ebf4ff"
                  stroke="#2f7ed8"
                  stroke-width="1.8"
                  stroke-linecap="round"
                  stroke-linejoin="round"
                  d={recognizedGesturePreviewDisplay.arrowPath}
                />
              {/if}
            </svg>
          {/if}
          <strong title={recognizedGestureId || '-'}>
            {displayGestureLabel(recognizedGestureHint, recognizedGestureId)}
          </strong>
        </div>
      </div>
      <div class="workspace-debug-item">
        <span>{t('label_auto_gesture_debug_matched_gesture', '匹配手势')}</span>
        <div class="workspace-debug-gesture">
          {#if matchedGesturePreview}
            <svg
              class="workspace-debug-gesture__svg"
              viewBox={`0 0 ${matchedGesturePreview.width} ${matchedGesturePreview.height}`}
              aria-hidden="true"
            >
              <path
                class="workspace-debug-gesture__grid"
                fill="none"
                stroke="rgba(157, 186, 219, 0.45)"
                stroke-width="1"
                stroke-dasharray="2 2"
                d={`M 4 ${(matchedGesturePreview.height / 2).toFixed(1)} H ${(matchedGesturePreview.width - 4).toFixed(1)} M ${(matchedGesturePreview.width / 2).toFixed(1)} 3 V ${(matchedGesturePreview.height - 3).toFixed(1)}`}
              />
              <path
                class="workspace-debug-gesture__path"
                fill="none"
                stroke="#2f7ed8"
                stroke-width="2.1"
                stroke-linecap="round"
                stroke-linejoin="round"
                d={matchedGesturePreview.path}
              />
              {#if matchedGesturePreview.startPoint}
                <circle
                  class="workspace-debug-gesture__start"
                  cx={matchedGesturePreview.startPoint.x}
                  cy={matchedGesturePreview.startPoint.y}
                  r="2.3"
                  fill="#ebf4ff"
                  stroke="#2f7ed8"
                  stroke-width="1.5"
                />
              {/if}
              {#if matchedGesturePreview.arrowPath}
                <path
                  class="workspace-debug-gesture__arrow"
                  fill="#ebf4ff"
                  stroke="#2f7ed8"
                  stroke-width="1.8"
                  stroke-linecap="round"
                  stroke-linejoin="round"
                  d={matchedGesturePreview.arrowPath}
                />
              {/if}
            </svg>
          {/if}
          <strong title={matchedGestureId || '-'}>
            {displayGestureLabel(matchedGestureHint, matchedGestureId)}
          </strong>
        </div>
      </div>
      <div class="workspace-debug-item">
        <span>{t('label_auto_gesture_debug_trigger_button', '触发按键')}</span>
        <strong>{routeStatus.lastTriggerButton || '-'}</strong>
      </div>
      <div class="workspace-debug-item">
        <span>{t('label_auto_gesture_debug_matched', '命中映射')}</span>
        <strong>{boolLabel(routeStatus.lastMatched, i18n)}</strong>
      </div>
      <div class="workspace-debug-item">
        <span>{t('label_auto_gesture_debug_injected', '快捷键已注入')}</span>
        <strong>{boolLabel(routeStatus.lastInjected, i18n)}</strong>
      </div>
      <div class="workspace-debug-item">
        <span>{t('label_auto_gesture_debug_source', '匹配来源')}</span>
        <strong>{gestureRouteSourceLabel(routeStatus, i18n)}</strong>
      </div>
      <div class="workspace-debug-item">
        <span>{t('label_auto_gesture_debug_modifiers', '修饰键')}</span>
        <strong>{formatGestureRouteModifiers(routeStatus.modifiers, platform, i18n)}</strong>
      </div>
      <div class="workspace-debug-item">
        <span>{t('label_auto_gesture_debug_candidates', '候选/窗口')}</span>
        <strong>
          {routeStatus.lastCandidateCount || 0}
          ·
          [{routeStatus.lastBestWindowStart ?? -1}, {routeStatus.lastBestWindowEnd ?? -1}]
        </strong>
      </div>
      <div class="workspace-debug-item">
        <span>{t('label_auto_gesture_debug_runner_up', '次优分数')}</span>
        <strong>{routeStatus.lastRunnerUpScore >= 0 ? routeStatus.lastRunnerUpScore.toFixed(1) : '-'}</strong>
      </div>
    </div>
    {#if recentEvents.length > 0}
      <div class="workspace-debug-events">
        <div class="workspace-debug-events__title">
          {t('label_auto_gesture_debug_recent_events', '最近事件')}
        </div>
        <div class="workspace-debug-events__list">
          {#each recentEvents as event (`event_${event.seq}_${event.timestampMs}`)}
            <div class="workspace-debug-event-row">
              <span class="workspace-debug-event-row__seq">#{event.seq}</span>
              <span class="workspace-debug-event-row__line">{eventLine(event)}</span>
              <span class="workspace-debug-event-row__tag">{sourceLabel(event)}</span>
              <span class="workspace-debug-event-row__tag">{event.triggerButton || '-'}</span>
              <span class="workspace-debug-event-row__tag">
                {boolLabel(event.matched, i18n)}/{boolLabel(event.injected, i18n)}
              </span>
            </div>
          {/each}
        </div>
      </div>
    {/if}
  </section>
{/if}

<style>
  .workspace-debug-card {
    margin-top: 10px;
    border: 1px solid rgba(194, 208, 226, 0.86);
    border-radius: 12px;
    background: linear-gradient(180deg, rgba(245, 251, 255, 0.96), rgba(237, 246, 255, 0.94));
    box-shadow: 0 12px 28px rgba(53, 86, 124, 0.08);
    padding: 10px;
  }

  .workspace-debug-title {
    margin: 0 0 8px;
    font-size: 12px;
    font-weight: 760;
    color: #2a4462;
  }

  .workspace-debug-grid {
    display: grid;
    grid-template-columns: 1fr;
    gap: 6px;
    margin-bottom: 8px;
  }

  .workspace-debug-item {
    border: 1px solid #cdddf0;
    border-radius: 9px;
    background: rgba(255, 255, 255, 0.92);
    padding: 6px 8px;
    display: flex;
    justify-content: space-between;
    gap: 8px;
    align-items: center;
  }

  .workspace-debug-item span {
    min-width: 0;
    font-size: 11px;
    color: #64809f;
  }

  .workspace-debug-item strong {
    min-width: 0;
    max-width: 55%;
    font-size: 12px;
    color: #244566;
    font-weight: 700;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }

  .workspace-debug-gesture {
    min-width: 0;
    max-width: 55%;
    display: inline-flex;
    align-items: center;
    justify-content: flex-end;
    gap: 6px;
  }

  .workspace-debug-gesture strong {
    min-width: 0;
    max-width: 100%;
    font-size: 12px;
    color: #244566;
    font-weight: 700;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }

  .workspace-debug-gesture__svg {
    width: 108px;
    height: 52px;
    flex: 0 0 auto;
    border: 1px solid #c9dff5;
    border-radius: 8px;
    background: rgba(245, 251, 255, 0.95);
  }

  .workspace-debug-gesture__grid {
    fill: none;
    stroke: rgba(157, 186, 219, 0.45);
    stroke-width: 1;
    stroke-dasharray: 2 2;
  }

  .workspace-debug-gesture__path {
    fill: none;
    stroke: #2f7ed8;
    stroke-width: 2.1;
    stroke-linecap: round;
    stroke-linejoin: round;
  }

  .workspace-debug-gesture__start {
    fill: #ebf4ff;
    stroke: #2f7ed8;
    stroke-width: 1.5;
  }

  .workspace-debug-gesture__arrow {
    fill: none;
    stroke: #2f7ed8;
    stroke-width: 2.2;
    stroke-linecap: round;
    stroke-linejoin: round;
  }

  .workspace-debug-events {
    border-top: 1px solid rgba(195, 213, 232, 0.88);
    padding-top: 8px;
  }

  .workspace-debug-events__title {
    font-size: 11px;
    font-weight: 700;
    color: #476589;
    margin-bottom: 6px;
  }

  .workspace-debug-events__list {
    display: flex;
    flex-direction: column;
    gap: 5px;
    max-height: 208px;
    overflow: auto;
    padding-right: 2px;
  }

  .workspace-debug-event-row {
    display: grid;
    grid-template-columns: auto 1fr auto auto auto;
    gap: 6px;
    align-items: center;
    border: 1px solid rgba(204, 221, 241, 0.92);
    border-radius: 8px;
    padding: 5px 6px;
    background: rgba(255, 255, 255, 0.9);
  }

  .workspace-debug-event-row__seq {
    font-size: 11px;
    color: #37587f;
    font-weight: 700;
    min-width: 34px;
  }

  .workspace-debug-event-row__line {
    min-width: 0;
    font-size: 11px;
    color: #335170;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }

  .workspace-debug-event-row__tag {
    font-size: 10px;
    color: #3f5f82;
    border: 1px solid rgba(191, 211, 235, 0.9);
    border-radius: 999px;
    padding: 2px 6px;
    background: rgba(238, 246, 255, 0.9);
    white-space: nowrap;
  }
</style>
