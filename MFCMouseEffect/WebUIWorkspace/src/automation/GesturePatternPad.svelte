<script>
  import { createEventDispatcher } from 'svelte';
  import {
    normalizeCanvasPoints,
    normalizeGestureStrokes,
    svgPathFromCanvasPoints,
  } from './gesture-path.js';

  export let strokes = [];
  export let disabled = false;
  export let width = 168;
  export let height = 112;
  export let maxStrokes = 4;
  export let texts = {};

  const dispatch = createEventDispatcher();

  let host = null;
  let drawing = false;
  let draftStroke = [];
  let committedStrokes = [];
  let pointerId = -1;
  let committedSignature = '[]';
  let localWritePending = false;
  function clamp(value, min, max) {
    return Math.min(max, Math.max(min, value));
  }

  function strokeSignature(value) {
    try {
      return JSON.stringify(normalizeGestureStrokes(value, maxStrokes));
    } catch (_error) {
      return '[]';
    }
  }

  function localPoint(event) {
    const rect = host?.getBoundingClientRect();
    if (!rect) {
      return null;
    }
    const x = ((event.clientX - rect.left) / Math.max(1, rect.width)) * 100;
    const y = ((event.clientY - rect.top) / Math.max(1, rect.height)) * 100;
    return {
      x: clamp(x, 0, 100),
      y: clamp(y, 0, 100),
    };
  }

  function commitStrokes(nextStrokes) {
    const normalized = normalizeGestureStrokes(nextStrokes, maxStrokes);
    committedStrokes = normalized;
    committedSignature = strokeSignature(normalized);
    localWritePending = true;
    dispatch('change', { strokes: normalized });
  }

  function onPointerDown(event) {
    if (disabled) {
      return;
    }
    const point = localPoint(event);
    if (!point) {
      return;
    }
    if (committedStrokes.length >= maxStrokes) {
      return;
    }
    drawing = true;
    pointerId = event.pointerId;
    draftStroke = [point];
    host?.setPointerCapture?.(pointerId);
  }

  function onPointerMove(event) {
    if (!drawing || event.pointerId !== pointerId) {
      return;
    }
    const point = localPoint(event);
    if (!point) {
      return;
    }
    draftStroke = [...draftStroke, point];
  }

  function finishStroke(event) {
    if (!drawing || event.pointerId !== pointerId) {
      return;
    }
    drawing = false;
    host?.releasePointerCapture?.(pointerId);
    pointerId = -1;
    const normalizedDraft = normalizeCanvasPoints(draftStroke);
    draftStroke = [];
    if (normalizedDraft.length === 0) {
      return;
    }
    commitStrokes([...committedStrokes, normalizedDraft]);
  }

  function clearStrokes() {
    if (committedStrokes.length === 0) {
      return;
    }
    commitStrokes([]);
  }

  function undoStroke() {
    if (committedStrokes.length === 0) {
      return;
    }
    commitStrokes(committedStrokes.slice(0, -1));
  }

  function pointToView(point, inset = 0) {
    if (!point) {
      return { x: 0, y: 0 };
    }
    return {
      x: clamp((point.x / 100) * width, inset, Math.max(inset, width - inset)),
      y: clamp((point.y / 100) * height, inset, Math.max(inset, height - inset)),
    };
  }

  function buildTangentArrowPath(stroke) {
    if (!Array.isArray(stroke) || stroke.length < 2) {
      return '';
    }

    const lastPoint = stroke[stroke.length - 1];
    let prevPoint = null;
    let fallbackPoint = null;
    for (let index = stroke.length - 2; index >= 0; index -= 1) {
      const candidate = stroke[index];
      if (candidate.x !== lastPoint.x || candidate.y !== lastPoint.y) {
        if (!fallbackPoint) {
          fallbackPoint = candidate;
        }
        const tipPreview = pointToView(lastPoint, 2);
        const fromPreview = pointToView(candidate, 2);
        const span = Math.hypot(tipPreview.x - fromPreview.x, tipPreview.y - fromPreview.y);
        if (span >= 8.5) {
          prevPoint = candidate;
          break;
        }
      }
    }
    if (!prevPoint) {
      prevPoint = fallbackPoint;
    }
    if (!prevPoint) {
      return '';
    }

    const tip = pointToView(lastPoint, 2);
    const from = pointToView(prevPoint, 2);
    const dx = tip.x - from.x;
    const dy = tip.y - from.y;
    const length = Math.hypot(dx, dy);
    if (length < 0.8) {
      return '';
    }

    const ux = dx / length;
    const uy = dy / length;
    const arrowLength = clamp(length * 0.46, 11.2, 16.8);
    const arrowWidth = clamp(arrowLength * 0.78, 8.4, 13.4);
    const baseX = tip.x - (ux * arrowLength);
    const baseY = tip.y - (uy * arrowLength);
    const normalX = -uy;
    const normalY = ux;
    const halfWidth = arrowWidth / 2;
    const leftX = baseX + (normalX * halfWidth);
    const leftY = baseY + (normalY * halfWidth);
    const rightX = baseX - (normalX * halfWidth);
    const rightY = baseY - (normalY * halfWidth);
    return `M ${leftX.toFixed(2)} ${leftY.toFixed(2)} L ${tip.x.toFixed(2)} ${tip.y.toFixed(2)} L ${rightX.toFixed(2)} ${rightY.toFixed(2)}`;
  }

  $: {
    const normalizedIncoming = normalizeGestureStrokes(strokes, maxStrokes);
    const incomingSignature = strokeSignature(normalizedIncoming);
    if (!drawing) {
      if (localWritePending) {
        if (incomingSignature === committedSignature) {
          localWritePending = false;
        }
      } else if (incomingSignature !== committedSignature) {
        committedStrokes = normalizedIncoming;
        committedSignature = incomingSignature;
      }
    }
  }

  $: normalizedDraftStroke = normalizeCanvasPoints(draftStroke);
  $: activeStrokes = drawing && normalizedDraftStroke.length > 0
    ? [...committedStrokes, normalizedDraftStroke]
    : committedStrokes;
  $: strokePreview = activeStrokes.map((stroke, index) => {
    const first = stroke[0];
    const last = stroke[stroke.length - 1];
    return {
      key: `stroke_${index + 1}_${stroke.length}`,
      index: index + 1,
      path: stroke.length >= 2 ? svgPathFromCanvasPoints(stroke, width, height, 0) : '',
      arrowPath: stroke.length >= 2 ? buildTangentArrowPath(stroke) : '',
      dot: stroke.length === 1 ? pointToView(last) : null,
      startBadge: first ? pointToView(first, 8) : null,
    };
  });
  $: hasPreview = strokePreview.length > 0;
  $: reachLimit = committedStrokes.length >= maxStrokes;
</script>

<div class="gesture-pad">
  <div
    bind:this={host}
    role="img"
    aria-label={texts.guide || 'Draw custom gesture template'}
    class="gesture-pad__surface"
    class:is-disabled={disabled}
    on:pointerdown={onPointerDown}
    on:pointermove={onPointerMove}
    on:pointerup={finishStroke}
    on:pointercancel={finishStroke}
  >
    <svg class="gesture-pad__svg" viewBox={`0 0 ${width} ${height}`} aria-hidden="true">
      <path class="gesture-pad__grid" d={`M 0 ${height / 2} H ${width} M ${width / 2} 0 V ${height}`} />
      {#each strokePreview as stroke (stroke.key)}
        <g class="gesture-pad__stroke">
          {#if stroke.path}
            <path class="gesture-pad__path" d={stroke.path} />
            {#if stroke.arrowPath}
              <path class="gesture-pad__arrow" d={stroke.arrowPath} />
            {/if}
          {:else if stroke.dot}
            <circle class="gesture-pad__dot" cx={stroke.dot.x} cy={stroke.dot.y} r="3.2" />
          {/if}
          {#if stroke.startBadge}
            <circle class="gesture-pad__badge-bg" cx={stroke.startBadge.x} cy={stroke.startBadge.y} r="7" />
            <text class="gesture-pad__badge-text" x={stroke.startBadge.x} y={stroke.startBadge.y + 0.6}>
              {stroke.index}
            </text>
          {/if}
        </g>
      {/each}
    </svg>
    {#if !hasPreview}
      <div class="gesture-pad__empty">{texts.empty}</div>
    {/if}
  </div>
  <div class="gesture-pad__actions">
    <button type="button" class="btn-soft" disabled={disabled || committedStrokes.length === 0} on:click={undoStroke}>
      {texts.undo}
    </button>
    <button type="button" class="btn-soft" disabled={disabled || committedStrokes.length === 0} on:click={clearStrokes}>
      {texts.clear}
    </button>
  </div>
  <div class="gesture-pad__hint">
    <span>{texts.guide}</span>
    {#if reachLimit}
      <span class="gesture-pad__limit">{texts.limitReached || ''}</span>
    {/if}
  </div>
</div>
