<script>
  import { createEventDispatcher } from 'svelte';
  import GesturePatternPad from './GesturePatternPad.svelte';
  import {
    flattenGestureStrokes,
    gestureStrokeDirections,
    gestureStrokePointCount,
    normalizeCanvasPoints,
    normalizeGestureStrokes,
  } from './gesture-path.js';

  export let row = {};
  export let options = [];
  export let texts = {};
  export let disabled = false;

  const MAX_CUSTOM_STROKES = 4;
  const MIN_THRESHOLD_PERCENT = 50;
  const MAX_THRESHOLD_PERCENT = 95;
  const PRESET_VIEW_WIDTH = 96;
  const PRESET_VIEW_HEIGHT = 54;
  const PRESET_VIEW_PADDING = 8;
  const PRESET_FIT_RATIO = 0.82;

  const dispatch = createEventDispatcher();
  let localMode = 'preset';
  let localCustomStrokes = [];
  let localStrokeSignature = '[]';
  let localWritePending = false;
  let localModeWritePending = false;
  let localThresholdPercent = 75;
  let localThresholdWritePending = false;

  function strokesSignature(strokes) {
    try {
      return JSON.stringify(normalizeGestureStrokes(strokes, MAX_CUSTOM_STROKES));
    } catch (_error) {
      return '[]';
    }
  }

  function readLegacyCustomPoints(pattern = {}) {
    const source = pattern?.customPoints !== undefined
      ? pattern.customPoints
      : pattern?.custom_points;
    return normalizeCanvasPoints(source);
  }

  function normalizePatternStrokes(pattern = {}) {
    const source = pattern?.customStrokes !== undefined
      ? pattern.customStrokes
      : pattern?.custom_strokes;
    const normalized = normalizeGestureStrokes(source, MAX_CUSTOM_STROKES);
    const legacy = readLegacyCustomPoints(pattern);
    return normalized.length > 0
      ? normalized
      : (legacy.length > 0 ? [legacy] : []);
  }

  function normalizeModeValue(value) {
    const configured = `${value || ''}`.trim().toLowerCase();
    if (configured === 'custom' || configured === 'preset') {
      return configured;
    }
    return '';
  }

  function normalizeThresholdValue(value) {
    const parsed = Number(value);
    if (!Number.isFinite(parsed)) {
      return 75;
    }
    return Math.max(MIN_THRESHOLD_PERCENT, Math.min(MAX_THRESHOLD_PERCENT, Math.round(parsed)));
  }

  function readThreshold(pattern = {}) {
    if (pattern?.matchThresholdPercent !== undefined) {
      return normalizeThresholdValue(pattern.matchThresholdPercent);
    }
    if (pattern?.match_threshold_percent !== undefined) {
      return normalizeThresholdValue(pattern.match_threshold_percent);
    }
    return 75;
  }

  function emitPatch(patch) {
    dispatch('change', {
      gesturePattern: {
        ...(row?.gesturePattern || {}),
        ...patch,
      },
    });
  }

  function onModeChange(event) {
    const nextMode = `${event.currentTarget.value || 'preset'}`.trim().toLowerCase() === 'custom'
      ? 'custom'
      : 'preset';
    localMode = nextMode;
    localModeWritePending = true;
    emitPatch({ mode: nextMode });
  }

  function onPresetValueChange(value) {
    dispatch('triggerchange', { trigger: value });
  }

  function presetPoints(value) {
    const key = `${value || ''}`.trim().toLowerCase();
    switch (key) {
      case 'line_right':
      case 'right':
        return [{ x: 10, y: 50 }, { x: 90, y: 50 }];
      case 'line_left':
      case 'left':
        return [{ x: 90, y: 50 }, { x: 10, y: 50 }];
      case 'up':
        return [{ x: 50, y: 85 }, { x: 50, y: 15 }];
      case 'down':
        return [{ x: 50, y: 15 }, { x: 50, y: 85 }];
      case 'v':
        return [{ x: 16, y: 18 }, { x: 50, y: 82 }, { x: 84, y: 18 }];
      case 'w':
        return [{ x: 8, y: 18 }, { x: 28, y: 82 }, { x: 50, y: 36 }, { x: 72, y: 82 }, { x: 92, y: 18 }];
      case 'slash':
        return [{ x: 20, y: 82 }, { x: 80, y: 18 }];
      case 'backslash':
        return [{ x: 20, y: 18 }, { x: 80, y: 82 }];
      case 'up_right':
        return [{ x: 34, y: 82 }, { x: 34, y: 22 }, { x: 80, y: 22 }];
      case 'up_left':
        return [{ x: 66, y: 82 }, { x: 66, y: 22 }, { x: 20, y: 22 }];
      case 'down_right':
        return [{ x: 34, y: 18 }, { x: 34, y: 78 }, { x: 80, y: 78 }];
      case 'down_left':
        return [{ x: 66, y: 18 }, { x: 66, y: 78 }, { x: 20, y: 78 }];
      default:
        return [{ x: 10, y: 50 }, { x: 90, y: 50 }];
    }
  }

  function presetDirectionHint(value) {
    const key = `${value || ''}`.trim().toLowerCase();
    switch (key) {
      case 'line_right':
      case 'right':
        return '→';
      case 'line_left':
      case 'left':
        return '←';
      case 'up':
        return '↑';
      case 'down':
        return '↓';
      case 'slash':
        return '↗';
      case 'backslash':
        return '↘';
      case 'up_right':
        return '↑→';
      case 'up_left':
        return '↑←';
      case 'down_right':
        return '↓→';
      case 'down_left':
        return '↓←';
      case 'v':
        return '↘↗';
      case 'w':
        return '↘↗↘↗';
      default:
        return '';
    }
  }

  function presetCardA11yLabel(option) {
    const label = `${option?.label || option?.value || ''}`.trim();
    const direction = presetDirectionHint(option?.value);
    if (!direction) {
      return label;
    }
    return `${label} (${direction})`;
  }

  function clampValue(value, min, max) {
    return Math.min(max, Math.max(min, value));
  }

  function presetViewPoints(value) {
    const source = normalizeCanvasPoints(presetPoints(value));
    if (source.length === 0) {
      return [];
    }

    let minX = source[0].x;
    let maxX = source[0].x;
    let minY = source[0].y;
    let maxY = source[0].y;
    for (const point of source) {
      minX = Math.min(minX, point.x);
      maxX = Math.max(maxX, point.x);
      minY = Math.min(minY, point.y);
      maxY = Math.max(maxY, point.y);
    }

    const spanX = Math.max(1, maxX - minX);
    const spanY = Math.max(1, maxY - minY);
    const drawW = Math.max(1, PRESET_VIEW_WIDTH - PRESET_VIEW_PADDING * 2);
    const drawH = Math.max(1, PRESET_VIEW_HEIGHT - PRESET_VIEW_PADDING * 2);
    const scale = Math.min(drawW / spanX, drawH / spanY) * PRESET_FIT_RATIO;
    const centerX = (minX + maxX) / 2;
    const centerY = (minY + maxY) / 2;

    return source.map((point) => ({
      x: clampValue(PRESET_VIEW_WIDTH / 2 + ((point.x - centerX) * scale), PRESET_VIEW_PADDING, PRESET_VIEW_WIDTH - PRESET_VIEW_PADDING),
      y: clampValue(PRESET_VIEW_HEIGHT / 2 + ((point.y - centerY) * scale), PRESET_VIEW_PADDING, PRESET_VIEW_HEIGHT - PRESET_VIEW_PADDING),
    }));
  }

  function svgPathFromViewPoints(points) {
    if (points.length === 0) {
      return '';
    }
    return points.map((point, index) => `${index === 0 ? 'M' : 'L'} ${point.x.toFixed(1)} ${point.y.toFixed(1)}`).join(' ');
  }

  function presetPath(value) {
    return svgPathFromViewPoints(presetViewPoints(value));
  }

  function presetStartPoint(value) {
    const points = presetViewPoints(value);
    if (points.length === 0) {
      return null;
    }
    return points[0];
  }

  function buildPresetEndArrowPath(value) {
    const points = presetViewPoints(value);
    if (points.length < 2) {
      return '';
    }

    const tip = points[points.length - 1];
    let from = null;
    for (let index = points.length - 2; index >= 0; index -= 1) {
      const candidate = points[index];
      if (candidate.x !== tip.x || candidate.y !== tip.y) {
        from = candidate;
        break;
      }
    }
    if (!from) {
      return '';
    }

    const dx = tip.x - from.x;
    const dy = tip.y - from.y;
    const length = Math.hypot(dx, dy);
    if (length < 0.8) {
      return '';
    }

    const ux = dx / length;
    const uy = dy / length;
    const arrowLength = 9.6;
    const arrowWidth = 8.4;
    const baseX = tip.x - (ux * arrowLength);
    const baseY = tip.y - (uy * arrowLength);
    const nx = -uy;
    const ny = ux;
    const half = arrowWidth / 2;
    const leftX = baseX + (nx * half);
    const leftY = baseY + (ny * half);
    const rightX = baseX - (nx * half);
    const rightY = baseY - (ny * half);
    return `M ${leftX.toFixed(2)} ${leftY.toFixed(2)} L ${tip.x.toFixed(2)} ${tip.y.toFixed(2)} L ${rightX.toFixed(2)} ${rightY.toFixed(2)}`;
  }

  function emitThreshold(nextValue) {
    const next = normalizeThresholdValue(nextValue);
    localThresholdPercent = next;
    localThresholdWritePending = true;
    emitPatch({ matchThresholdPercent: next });
  }

  function onThresholdInput(event) {
    emitThreshold(event.currentTarget.value);
  }

  function onThresholdWheel(event) {
    if (disabled) {
      return;
    }
    const delta = event.deltaY < 0 ? 1 : (event.deltaY > 0 ? -1 : 0);
    if (delta === 0) {
      return;
    }
    event.preventDefault();
    emitThreshold(localThresholdPercent + delta);
  }

  function onPadChange(event) {
    localCustomStrokes = normalizeGestureStrokes(event.detail?.strokes, MAX_CUSTOM_STROKES);
    localStrokeSignature = strokesSignature(localCustomStrokes);
    localWritePending = true;
    emitPatch({
      mode: 'custom',
      customStrokes: localCustomStrokes,
      customPoints: flattenGestureStrokes(localCustomStrokes, MAX_CUSTOM_STROKES),
    });
  }

  $: incomingPattern = row?.gesturePattern || {};
  $: incomingStrokes = normalizePatternStrokes(incomingPattern);
  $: incomingStrokeSignature = strokesSignature(incomingStrokes);
  $: {
    if (localWritePending) {
      if (incomingStrokeSignature === localStrokeSignature) {
        localWritePending = false;
      }
    } else if (incomingStrokeSignature !== localStrokeSignature) {
      localCustomStrokes = incomingStrokes;
      localStrokeSignature = incomingStrokeSignature;
    }
  }
  $: incomingMode = normalizeModeValue(incomingPattern.mode);
  $: {
    const fallbackMode = localCustomStrokes.length > 0 ? 'custom' : 'preset';
    if (localModeWritePending) {
      if ((incomingMode || fallbackMode) === localMode) {
        localModeWritePending = false;
      }
    } else if ((incomingMode || fallbackMode) !== localMode) {
      localMode = incomingMode || fallbackMode;
    }
  }
  $: customStrokeSet = localCustomStrokes;
  $: customPointCount = gestureStrokePointCount(customStrokeSet);
  $: customStrokeCount = customStrokeSet.length;
  $: customDirections = gestureStrokeDirections(customStrokeSet, 8);
  $: customStrokeSummary = customStrokeSet.map((stroke, index) => ({
    index: index + 1,
    points: stroke.length,
    direction: customDirections[index] || (texts.canvasNoDirection || ''),
  }));
  $: currentPreset = `${row?.trigger || ''}`.trim();
  $: {
    const incomingThreshold = readThreshold(incomingPattern);
    if (localThresholdWritePending) {
      if (incomingThreshold === localThresholdPercent) {
        localThresholdWritePending = false;
      }
    } else if (incomingThreshold !== localThresholdPercent) {
      localThresholdPercent = incomingThreshold;
    }
  }
</script>

<div class="gesture-pattern-editor">
  <div class="gesture-pattern-editor__head">
    <div class="gesture-pattern-editor__title">{texts.title}</div>
    <div class="gesture-pattern-editor__modes">
      <label class="gesture-pattern-editor__mode">
        <input type="radio" name={`gesture_mode_${row?.id || 'row'}`} value="preset" checked={localMode === 'preset'} disabled={disabled} on:change={onModeChange} />
        <span>{texts.modePreset}</span>
      </label>
      <label class="gesture-pattern-editor__mode">
        <input type="radio" name={`gesture_mode_${row?.id || 'row'}`} value="custom" checked={localMode === 'custom'} disabled={disabled} on:change={onModeChange} />
        <span>{texts.modeCustom}</span>
      </label>
    </div>
  </div>

  {#if localMode === 'preset'}
    <div class="gesture-pattern-editor__section">
      <div class="gesture-pattern-editor__label">{texts.preset}</div>
      <div class="gesture-preset-grid">
        {#each options as option (option.value)}
          <button
            type="button"
            class="gesture-preset-card"
            class:is-active={currentPreset === option.value}
            disabled={disabled}
            title={presetCardA11yLabel(option)}
            aria-label={presetCardA11yLabel(option)}
            on:click={() => onPresetValueChange(option.value)}
          >
            <svg class="gesture-preset-card__svg" viewBox="0 0 96 54" aria-hidden="true">
              <path class="gesture-preset-card__grid" d="M 8 27 H 88 M 48 8 V 46" />
              <path class="gesture-preset-card__path" d={presetPath(option.value)} />
              {#if presetStartPoint(option.value)}
                <circle class="gesture-preset-card__start" cx={presetStartPoint(option.value).x} cy={presetStartPoint(option.value).y} r="2.6" />
              {/if}
              {#if buildPresetEndArrowPath(option.value)}
                <path class="gesture-preset-card__arrow" d={buildPresetEndArrowPath(option.value)} />
              {/if}
            </svg>
            <span class="gesture-preset-card__label">{option.label}</span>
          </button>
        {/each}
      </div>
    </div>
  {:else}
    <div class="gesture-pattern-editor__section">
      <div class="gesture-pattern-editor__label-row">
        <div class="gesture-pattern-editor__label">{texts.custom}</div>
        <span
          class="gesture-pattern-editor__limit-tip"
          title={texts.canvasLimitHint}
          aria-label={texts.canvasLimitHint}
        >
          {texts.canvasLimitBadge || '!'}
        </span>
      </div>
      <GesturePatternPad
        strokes={customStrokeSet}
        disabled={disabled}
        maxStrokes={MAX_CUSTOM_STROKES}
        texts={{
          empty: texts.canvasEmpty,
          clear: texts.canvasClear,
          undo: texts.canvasUndo,
          guide: texts.canvasGuide,
          limitReached: texts.canvasLimitReached,
        }}
        on:change={onPadChange}
      />
      {#if customStrokeCount > 0}
        <div class="gesture-pattern-editor__stroke-list">
          {#each customStrokeSummary as item (item.index)}
            <span class="gesture-pattern-editor__stroke-chip">
              <span class="gesture-pattern-editor__stroke-chip-index">#{item.index}</span>
              <span class="gesture-pattern-editor__stroke-chip-dir">{item.direction}</span>
              <span class="gesture-pattern-editor__stroke-chip-points">{item.points}{texts.canvasPointUnit || 'pt'}</span>
            </span>
          {/each}
        </div>
      {/if}
      <div class="gesture-pattern-editor__meta-row">
        <div class="gesture-pattern-editor__meta-item">
          <span>{texts.canvasStrokeCount || 'Stroke'}</span>
          <strong>{customStrokeCount}</strong>
        </div>
        <div class="gesture-pattern-editor__threshold-control">
          <label class="gesture-pattern-editor__threshold-label" for={`gesture_threshold_${row?.id || 'row'}`}>
            {texts.threshold}
          </label>
          <input
            id={`gesture_threshold_${row?.id || 'row'}`}
            class="gesture-pattern-editor__threshold-input"
            type="number"
            min={MIN_THRESHOLD_PERCENT}
            max={MAX_THRESHOLD_PERCENT}
            step="1"
            disabled={disabled}
            value={localThresholdPercent}
            on:input={onThresholdInput}
            on:wheel={onThresholdWheel}
          />
          <span class="gesture-pattern-editor__threshold-unit">%</span>
        </div>
        <div class="gesture-pattern-editor__meta-item gesture-pattern-editor__meta-item--end">
          <strong>{customPointCount}</strong>
          <span>{texts.canvasPointUnit || 'pt'}</span>
        </div>
      </div>
    </div>
  {/if}
</div>
