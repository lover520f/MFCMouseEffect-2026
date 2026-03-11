function clamp(value, min, max) {
  return Math.min(max, Math.max(min, value));
}

function asPoint(value) {
  if (!value || typeof value !== 'object') {
    return null;
  }
  const x = Number(value.x);
  const y = Number(value.y);
  if (!Number.isFinite(x) || !Number.isFinite(y)) {
    return null;
  }
  return { x, y };
}

function round2(value) {
  return Math.round(value * 100) / 100;
}

function compactPoints(points, epsilon = 0.05) {
  const compacted = [];
  for (const point of points) {
    const prev = compacted[compacted.length - 1];
    if (!prev) {
      compacted.push(point);
      continue;
    }
    if (Math.abs(prev.x - point.x) < epsilon && Math.abs(prev.y - point.y) < epsilon) {
      continue;
    }
    compacted.push(point);
  }
  return compacted;
}

export function normalizeGesturePoints(points) {
  const source = normalizeCanvasPoints(points);
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

  const width = Math.max(1, maxX - minX);
  const height = Math.max(1, maxY - minY);
  return compactPoints(source.map((point) => ({
    x: clamp(round2(((point.x - minX) / width) * 100), 0, 100),
    y: clamp(round2(((point.y - minY) / height) * 100), 0, 100),
  })));
}

export function normalizeCanvasPoints(points) {
  const source = Array.isArray(points) ? points.map(asPoint).filter(Boolean) : [];
  if (source.length === 0) {
    return [];
  }
  return compactPoints(source.map((point) => ({
    x: clamp(round2(point.x), 0, 100),
    y: clamp(round2(point.y), 0, 100),
  })));
}

export function normalizeGestureStrokes(strokes, maxStrokes = Number.POSITIVE_INFINITY) {
  const source = Array.isArray(strokes) ? strokes : [];
  const out = [];
  for (const stroke of source) {
    const points = normalizeCanvasPoints(Array.isArray(stroke) ? stroke : stroke?.points);
    if (points.length === 0) {
      continue;
    }
    out.push(points);
    if (out.length >= maxStrokes) {
      break;
    }
  }
  return out;
}

export function flattenGestureStrokes(strokes, maxStrokes = Number.POSITIVE_INFINITY) {
  const source = normalizeGestureStrokes(strokes, maxStrokes);
  const out = [];
  for (const stroke of source) {
    out.push(...stroke);
  }
  return out;
}

function svgPathFromNormalizedPoints(points, width, height, padding) {
  if (!Array.isArray(points) || points.length === 0) {
    return '';
  }
  const drawWidth = Math.max(1, width - padding * 2);
  const drawHeight = Math.max(1, height - padding * 2);
  return points.map((point, index) => {
    const x = padding + (point.x / 100) * drawWidth;
    const y = padding + (point.y / 100) * drawHeight;
    return `${index === 0 ? 'M' : 'L'} ${x.toFixed(1)} ${y.toFixed(1)}`;
  }).join(' ');
}

export function svgPathFromGesturePoints(points, width = 168, height = 112, padding = 12) {
  return svgPathFromNormalizedPoints(normalizeGesturePoints(points), width, height, padding);
}

export function svgPathFromCanvasPoints(points, width = 168, height = 112, padding = 12) {
  return svgPathFromNormalizedPoints(normalizeCanvasPoints(points), width, height, padding);
}

export function gesturePointCount(points) {
  return normalizeGesturePoints(points).length;
}

export function gestureStrokePointCount(strokes) {
  const source = normalizeGestureStrokes(strokes);
  let total = 0;
  for (const stroke of source) {
    total += stroke.length;
  }
  return total;
}

function arrowFromDelta(dx, dy) {
  const x = Number(dx) || 0;
  const y = Number(dy) || 0;
  if (Math.abs(x) < 0.6 && Math.abs(y) < 0.6) {
    return '';
  }

  const angle = Math.atan2(y, x) * (180 / Math.PI);
  if (angle >= -22.5 && angle < 22.5) return '→';
  if (angle >= 22.5 && angle < 67.5) return '↘';
  if (angle >= 67.5 && angle < 112.5) return '↓';
  if (angle >= 112.5 && angle < 157.5) return '↙';
  if (angle >= 157.5 || angle < -157.5) return '←';
  if (angle >= -157.5 && angle < -112.5) return '↖';
  if (angle >= -112.5 && angle < -67.5) return '↑';
  return '↗';
}

export function gestureDirectionHint(points, maxSegments = 6) {
  const normalized = normalizeCanvasPoints(points);
  if (normalized.length < 2) {
    return '';
  }
  const arrows = [];
  for (let i = 1; i < normalized.length; i += 1) {
    const prev = normalized[i - 1];
    const next = normalized[i];
    const arrow = arrowFromDelta(next.x - prev.x, next.y - prev.y);
    if (!arrow) {
      continue;
    }
    if (arrows.length > 0 && arrows[arrows.length - 1] === arrow) {
      continue;
    }
    arrows.push(arrow);
    if (arrows.length >= maxSegments) {
      break;
    }
  }
  return arrows.join('');
}

export function gestureStrokeDirections(strokes, maxSegments = 6) {
  const normalized = normalizeGestureStrokes(strokes);
  return normalized.map((stroke) => gestureDirectionHint(stroke, maxSegments));
}
