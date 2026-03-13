function asText(value) {
  return `${value || ''}`.trim().toLowerCase();
}

function asFiniteNumber(value, fallback) {
  const parsed = Number(value);
  return Number.isFinite(parsed) ? parsed : fallback;
}

function clamp(value, min, max) {
  return Math.min(max, Math.max(min, value));
}

const DIRECTION_VECTORS = {
  left: [-1, 0],
  right: [1, 0],
  up: [0, -1],
  down: [0, 1],
  diag_up_left: [-1, -1],
  diag_up_right: [1, -1],
  diag_down_left: [-1, 1],
  diag_down_right: [1, 1],
};

const DIRECTION_ARROWS = {
  left: '←',
  right: '→',
  up: '↑',
  down: '↓',
  diag_up_left: '↖',
  diag_up_right: '↗',
  diag_down_left: '↙',
  diag_down_right: '↘',
};

const GESTURE_ALIASES = {
  line_right: 'right',
  line_left: 'left',
  line_up: 'up',
  line_down: 'down',
  hline: 'right',
  vline: 'down',
  slash: 'diag_down_right',
  backslash: 'diag_down_left',
  v: 'diag_down_right_diag_up_right',
  w: 'diag_down_right_diag_up_right_diag_down_right',
};

function normalizeGestureId(value) {
  const text = asText(value).replace(/[\s-]+/g, '_');
  return GESTURE_ALIASES[text] || text;
}

export function parseGestureDirections(gestureId) {
  const normalized = normalizeGestureId(gestureId);
  if (!normalized) {
    return [];
  }
  const parts = normalized.split('_').filter((part) => !!part);
  const directions = [];

  for (let index = 0; index < parts.length;) {
    if (parts[index] === 'diag' && index + 2 < parts.length) {
      const diagonal = `diag_${parts[index + 1]}_${parts[index + 2]}`;
      if (DIRECTION_VECTORS[diagonal]) {
        directions.push(diagonal);
        index += 3;
        continue;
      }
    }

    const single = parts[index];
    if (DIRECTION_VECTORS[single]) {
      directions.push(single);
      index += 1;
      continue;
    }

    return [];
  }

  return directions;
}

export function gestureIdDirectionHint(gestureId) {
  const directions = parseGestureDirections(gestureId);
  if (directions.length <= 0) {
    return '';
  }
  return directions.map((direction) => DIRECTION_ARROWS[direction]).join('');
}

function buildRawPoints(directions) {
  let x = 0;
  let y = 0;
  const points = [{ x, y }];
  for (const direction of directions) {
    const vector = DIRECTION_VECTORS[direction];
    if (!vector) {
      continue;
    }
    x += vector[0];
    y += vector[1];
    points.push({ x, y });
  }
  return points;
}

function fitPoints(points, width, height, padding, fitRatio) {
  if (!Array.isArray(points) || points.length <= 0) {
    return [];
  }

  let minX = points[0].x;
  let maxX = points[0].x;
  let minY = points[0].y;
  let maxY = points[0].y;
  for (const point of points) {
    minX = Math.min(minX, point.x);
    maxX = Math.max(maxX, point.x);
    minY = Math.min(minY, point.y);
    maxY = Math.max(maxY, point.y);
  }

  const spanX = Math.max(1, maxX - minX);
  const spanY = Math.max(1, maxY - minY);
  const drawWidth = Math.max(1, width - padding * 2);
  const drawHeight = Math.max(1, height - padding * 2);
  const scale = Math.min(drawWidth / spanX, drawHeight / spanY) * fitRatio;
  const centerX = (minX + maxX) / 2;
  const centerY = (minY + maxY) / 2;

  return points.map((point) => ({
    x: clamp(width / 2 + ((point.x - centerX) * scale), padding, width - padding),
    y: clamp(height / 2 + ((point.y - centerY) * scale), padding, height - padding),
  }));
}

function svgPathFromPoints(points) {
  if (!Array.isArray(points) || points.length <= 0) {
    return '';
  }
  return points.map((point, index) => (
    `${index === 0 ? 'M' : 'L'} ${point.x.toFixed(2)} ${point.y.toFixed(2)}`
  )).join(' ');
}

function resamplePolylineByCount(points, targetCount = 48) {
  if (!Array.isArray(points) || points.length < 2) {
    return points || [];
  }
  const count = Math.max(8, Math.min(256, Math.trunc(targetCount)));
  if (points.length >= count) {
    return points;
  }

  const cumulative = [0];
  for (let i = 1; i < points.length; i += 1) {
    const prev = points[i - 1];
    const curr = points[i];
    cumulative.push(cumulative[i - 1] + Math.hypot(curr.x - prev.x, curr.y - prev.y));
  }
  const total = cumulative[cumulative.length - 1];
  if (total <= 0.001) {
    return points;
  }

  const out = [];
  for (let i = 0; i < count; i += 1) {
    const t = i / (count - 1);
    const dist = t * total;
    let seg = 1;
    while (seg < cumulative.length && cumulative[seg] < dist) {
      seg += 1;
    }
    const right = Math.min(seg, cumulative.length - 1);
    const left = Math.max(0, right - 1);
    const d0 = cumulative[left];
    const d1 = cumulative[right];
    const span = Math.max(1e-6, d1 - d0);
    const alpha = clamp((dist - d0) / span, 0, 1);
    const p0 = points[left];
    const p1 = points[right];
    out.push({
      x: p0.x + (p1.x - p0.x) * alpha,
      y: p0.y + (p1.y - p0.y) * alpha,
    });
  }
  return out;
}

function densifyPoints(points) {
  if (!Array.isArray(points) || points.length < 2) {
    return points || [];
  }
  const out = [points[0]];
  for (let i = 1; i < points.length; i += 1) {
    const prev = points[i - 1];
    const curr = points[i];
    const dx = curr.x - prev.x;
    const dy = curr.y - prev.y;
    const distance = Math.hypot(dx, dy);
    const segments = Math.max(1, Math.min(10, Math.round(distance / 4)));
    for (let s = 1; s <= segments; s += 1) {
      const t = s / segments;
      out.push({
        x: prev.x + dx * t,
        y: prev.y + dy * t,
      });
    }
  }
  return out;
}

function smoothPointsChaikin(points, iterations = 2) {
  if (!Array.isArray(points) || points.length < 3) {
    return points || [];
  }
  let current = points.slice();
  const roundCount = Math.max(1, Math.min(4, Math.trunc(iterations)));
  for (let iter = 0; iter < roundCount; iter += 1) {
    if (current.length < 3) {
      break;
    }
    const next = [current[0]];
    for (let i = 0; i < current.length - 1; i += 1) {
      const a = current[i];
      const b = current[i + 1];
      next.push({
        x: 0.75 * a.x + 0.25 * b.x,
        y: 0.75 * a.y + 0.25 * b.y,
      });
      next.push({
        x: 0.25 * a.x + 0.75 * b.x,
        y: 0.25 * a.y + 0.75 * b.y,
      });
    }
    next.push(current[current.length - 1]);
    current = next;
  }
  return current;
}

function smoothPathFromPoints(points) {
  if (!Array.isArray(points) || points.length <= 0) {
    return '';
  }
  if (points.length < 3) {
    return svgPathFromPoints(points);
  }
  let path = `M ${points[0].x.toFixed(2)} ${points[0].y.toFixed(2)}`;
  for (let i = 1; i < points.length - 1; i += 1) {
    const current = points[i];
    const next = points[i + 1];
    const controlX = current.x;
    const controlY = current.y;
    const endX = ((current.x + next.x) / 2);
    const endY = ((current.y + next.y) / 2);
    path += ` Q ${controlX.toFixed(2)} ${controlY.toFixed(2)} ${endX.toFixed(2)} ${endY.toFixed(2)}`;
  }
  const tail = points[points.length - 1];
  path += ` T ${tail.x.toFixed(2)} ${tail.y.toFixed(2)}`;
  return path;
}

function endArrowPath(points, length = 8.2, width = 6.8) {
  if (!Array.isArray(points) || points.length < 2) {
    return '';
  }

  const tip = points[points.length - 1];
  const tailSamples = [];
  for (let index = points.length - 2; index >= 0 && tailSamples.length < 8; index -= 1) {
    const candidate = points[index];
    if (candidate.x !== tip.x || candidate.y !== tip.y) {
      tailSamples.push(candidate);
    }
  }
  if (tailSamples.length <= 0) {
    return '';
  }
  let dx = 0;
  let dy = 0;
  let weight = 1.0;
  const weightDecay = 0.78;
  for (const p of tailSamples) {
    dx += (tip.x - p.x) * weight;
    dy += (tip.y - p.y) * weight;
    weight *= weightDecay;
  }
  const vectorLength = Math.hypot(dx, dy);
  if (vectorLength < 0.01) {
    return '';
  }

  const ux = dx / vectorLength;
  const uy = dy / vectorLength;
  const nx = -uy;
  const ny = ux;
  const baseX = tip.x - (ux * length);
  const baseY = tip.y - (uy * length);
  const halfWidth = width / 2;
  const leftX = baseX + (nx * halfWidth);
  const leftY = baseY + (ny * halfWidth);
  const rightX = baseX - (nx * halfWidth);
  const rightY = baseY - (ny * halfWidth);
  return `M ${leftX.toFixed(2)} ${leftY.toFixed(2)} L ${tip.x.toFixed(2)} ${tip.y.toFixed(2)} L ${rightX.toFixed(2)} ${rightY.toFixed(2)} Z`;
}

export function buildGesturePreviewFromId(gestureId, options = {}) {
  const directions = parseGestureDirections(gestureId);
  if (directions.length <= 0) {
    return null;
  }

  const width = Math.max(40, asFiniteNumber(options.width, 88));
  const height = Math.max(24, asFiniteNumber(options.height, 40));
  const padding = clamp(asFiniteNumber(options.padding, 4), 2, Math.min(width, height) / 3);
  const fitRatio = clamp(asFiniteNumber(options.fitRatio, 0.82), 0.45, 1);

  const rawPoints = buildRawPoints(directions);
  const points = fitPoints(rawPoints, width, height, padding, fitRatio);
  if (points.length <= 0) {
    return null;
  }
  const resampled = resamplePolylineByCount(points, 56);
  const densePoints = densifyPoints(resampled);
  const smoothedPoints = smoothPointsChaikin(densePoints, 2);
  const renderPoints = smoothedPoints.length >= 2 ? smoothedPoints : densePoints;

  return {
    width,
    height,
    directions,
    directionHint: directions.map((direction) => DIRECTION_ARROWS[direction]).join(''),
    path: svgPathFromPoints(renderPoints),
    startPoint: points[0],
    arrowPath: endArrowPath(renderPoints),
  };
}

export function buildGesturePreviewFromPoints(samplePoints, options = {}) {
  if (!Array.isArray(samplePoints) || samplePoints.length < 2) {
    return null;
  }

  const rawPoints = samplePoints
    .map((point) => ({
      x: asFiniteNumber(point?.x, NaN),
      y: asFiniteNumber(point?.y, NaN),
    }))
    .filter((point) => Number.isFinite(point.x) && Number.isFinite(point.y));
  if (rawPoints.length < 2) {
    return null;
  }

  const width = Math.max(40, asFiniteNumber(options.width, 88));
  const height = Math.max(24, asFiniteNumber(options.height, 40));
  const padding = clamp(asFiniteNumber(options.padding, 4), 2, Math.min(width, height) / 3);
  const fitRatio = clamp(asFiniteNumber(options.fitRatio, 0.82), 0.45, 1);
  const points = fitPoints(rawPoints, width, height, padding, fitRatio);
  if (points.length < 2) {
    return null;
  }
  const resampled = resamplePolylineByCount(points, 64);
  const densePoints = densifyPoints(resampled);
  const smoothedPoints = smoothPointsChaikin(densePoints, 2);
  const renderPoints = smoothedPoints.length >= 2 ? smoothedPoints : densePoints;

  return {
    width,
    height,
    directions: [],
    directionHint: '',
    path: svgPathFromPoints(renderPoints),
    startPoint: points[0],
    arrowPath: endArrowPath(renderPoints),
  };
}
