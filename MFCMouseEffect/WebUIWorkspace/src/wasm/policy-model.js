const DEFAULT_POLICY_RANGES = {
  output_buffer_bytes: {
    min: 1024,
    max: 262144,
    step: 1024,
    defaultValue: 16384,
  },
  max_commands: {
    min: 1,
    max: 2048,
    step: 1,
    defaultValue: 256,
  },
  max_execution_ms: {
    min: 0.1,
    max: 20.0,
    step: 0.1,
    defaultValue: 1.0,
  },
};

function parseFiniteNumber(value, fallback) {
  const parsed = Number(value);
  return Number.isFinite(parsed) ? parsed : fallback;
}

function clampNumber(value, min, max) {
  return Math.max(min, Math.min(max, value));
}

function snapToStep(value, min, step) {
  if (!Number.isFinite(step) || step <= 0) {
    return value;
  }
  const offset = (value - min) / step;
  return min + (Math.round(offset) * step);
}

export function normalizeRange(input, fallback) {
  const source = input || {};
  const rangeFallback = fallback || {};
  const min = parseFiniteNumber(source.min, parseFiniteNumber(rangeFallback.min, 0));
  const max = parseFiniteNumber(source.max, parseFiniteNumber(rangeFallback.max, min));
  const step = parseFiniteNumber(source.step, parseFiniteNumber(rangeFallback.step, 1));
  const defaultValue = parseFiniteNumber(
    source.default,
    parseFiniteNumber(source.defaultValue, parseFiniteNumber(rangeFallback.defaultValue, min)),
  );
  const normalizedMin = Math.min(min, max);
  const normalizedMax = Math.max(min, max);
  return {
    min: normalizedMin,
    max: normalizedMax,
    step: step > 0 ? step : 1,
    defaultValue: clampNumber(defaultValue, normalizedMin, normalizedMax),
  };
}

export function normalizePolicyRanges(input) {
  const value = input || {};
  return {
    output_buffer_bytes: normalizeRange(
      value.output_buffer_bytes,
      DEFAULT_POLICY_RANGES.output_buffer_bytes,
    ),
    max_commands: normalizeRange(
      value.max_commands,
      DEFAULT_POLICY_RANGES.max_commands,
    ),
    max_execution_ms: normalizeRange(
      value.max_execution_ms,
      DEFAULT_POLICY_RANGES.max_execution_ms,
    ),
  };
}

export function clampIntByRange(value, range) {
  const base = Number.parseInt(String(value || ""), 10);
  const parsed = Number.isFinite(base) ? base : range.defaultValue;
  const snapped = snapToStep(parsed, range.min, range.step);
  const bounded = clampNumber(snapped, range.min, range.max);
  return Math.round(bounded);
}

export function clampFloatByRange(value, range) {
  const parsedRaw = Number.parseFloat(String(value || ""));
  const parsed = Number.isFinite(parsedRaw) ? parsedRaw : range.defaultValue;
  const snapped = snapToStep(parsed, range.min, range.step);
  const bounded = clampNumber(snapped, range.min, range.max);
  const precision = (() => {
    const stepString = `${range.step}`;
    const dot = stepString.indexOf(".");
    return dot >= 0 ? Math.max(0, stepString.length - dot - 1) : 3;
  })();
  return Number(bounded.toFixed(precision));
}

export function resolvePolicyInputValue(value, range) {
  const parsed = Number(value);
  if (!Number.isFinite(parsed)) {
    return range.defaultValue;
  }
  return clampNumber(parsed, range.min, range.max);
}

export { DEFAULT_POLICY_RANGES };
