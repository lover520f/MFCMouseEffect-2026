import {
  DEFAULT_GESTURE_MAX_DIRECTIONS,
  DEFAULT_GESTURE_MIN_DISTANCE,
  DEFAULT_GESTURE_SAMPLE_STEP,
  DEFAULT_GESTURE_TRIGGER_BUTTON,
} from './defaults.js';
import { normalizeTriggerChain, serializeTriggerChain } from './trigger-chain.js';

function asObject(value) {
  return value && typeof value === 'object' ? value : {};
}

function asArray(value) {
  return Array.isArray(value) ? value : [];
}

function asText(value) {
  return `${value || ''}`.trim();
}

function asNumber(value, fallback) {
  const parsed = Number(value);
  return Number.isFinite(parsed) ? parsed : fallback;
}

export function textOf(i18n, key, fallback) {
  const table = asObject(i18n);
  return table[key] || fallback || '';
}

export function normalizeOptions(items) {
  const out = [];
  for (const item of asArray(items)) {
    const source = asObject(item);
    const value = asText(source.value);
    if (!value) {
      continue;
    }
    const label = asText(source.label) || value;
    out.push({ value, label });
  }
  return out;
}

export function defaultOptionValue(options, fallback) {
  if (options.length > 0 && options[0].value) {
    return options[0].value;
  }
  return fallback || '';
}

export function sanitizeOptionValue(value, options, fallback) {
  const text = asText(value);
  if (!text) {
    return fallback || '';
  }
  for (const option of options) {
    if (option.value === text) {
      return text;
    }
  }
  return fallback || '';
}

function normalizeBinding(item, options, fallbackTrigger) {
  const source = asObject(item);
  const triggerChain = normalizeTriggerChain(source.triggerChain || source.trigger, options, fallbackTrigger);
  return {
    enabled: source.enabled !== false,
    triggerChain,
    trigger: serializeTriggerChain(triggerChain, options, fallbackTrigger),
    keys: asText(source.keys),
  };
}

export function normalizeBindings(items, options, fallbackTrigger) {
  const out = [];
  for (const item of asArray(items)) {
    out.push(normalizeBinding(item, options, fallbackTrigger));
  }
  return out;
}

export function normalizeAutomationPayload(schema, payloadState) {
  const schemaObj = asObject(schema);
  const payload = asObject(payloadState);
  const gesture = asObject(payload.gesture);

  const mouseOptions = normalizeOptions(schemaObj.automation_mouse_actions);
  const gestureOptions = normalizeOptions(schemaObj.automation_gesture_patterns);
  const gestureButtonOptions = normalizeOptions(schemaObj.automation_gesture_buttons);

  const defaultMouseTrigger = defaultOptionValue(mouseOptions, '');
  const defaultGestureTrigger = defaultOptionValue(gestureOptions, '');
  const defaultGestureButton = defaultOptionValue(gestureButtonOptions, DEFAULT_GESTURE_TRIGGER_BUTTON);

  return {
    mouseOptions,
    gestureOptions,
    gestureButtonOptions,
    enabled: !!payload.enabled,
    mouseMappings: normalizeBindings(payload.mouse_mappings, mouseOptions, defaultMouseTrigger),
    gestureEnabled: !!gesture.enabled,
    gestureTriggerButton: sanitizeOptionValue(
      gesture.trigger_button,
      gestureButtonOptions,
      defaultGestureButton),
    gestureMinDistance: asNumber(gesture.min_stroke_distance_px, DEFAULT_GESTURE_MIN_DISTANCE),
    gestureSampleStep: asNumber(gesture.sample_step_px, DEFAULT_GESTURE_SAMPLE_STEP),
    gestureMaxDirections: asNumber(gesture.max_directions, DEFAULT_GESTURE_MAX_DIRECTIONS),
    gestureMappings: normalizeBindings(gesture.mappings, gestureOptions, defaultGestureTrigger),
    defaultMouseTrigger,
    defaultGestureTrigger,
    defaultGestureButton,
  };
}

export function readMappings(rows, options, fallbackTrigger) {
  const normalizedFallback = defaultOptionValue(options, fallbackTrigger || '');
  const out = [];
  for (const row of asArray(rows)) {
    const source = asObject(row);
    const keys = asText(source.keys);
    if (!keys) {
      continue;
    }
    const triggerChain = normalizeTriggerChain(
      source.triggerChain || source.trigger,
      options,
      normalizedFallback);
    out.push({
      enabled: source.enabled !== false,
      trigger: serializeTriggerChain(triggerChain, options, normalizedFallback),
      keys,
    });
  }
  return out;
}

export function evaluateRows(rows, options, fallbackTrigger, messages) {
  const normalizedFallback = defaultOptionValue(options, fallbackTrigger || '');
  const missingMessage = asText(messages?.missing);
  const duplicateMessage = asText(messages?.duplicate);

  const nextRows = [];
  const buckets = new Map();
  let hasMissingShortcut = false;
  let hasDuplicateTrigger = false;

  for (const row of asArray(rows)) {
    const source = asObject(row);
    const triggerChain = normalizeTriggerChain(
      source.triggerChain || source.trigger,
      options,
      normalizedFallback);
    const trigger = serializeTriggerChain(triggerChain, options, normalizedFallback);
    const next = {
      ...source,
      enabled: source.enabled !== false,
      triggerChain,
      trigger,
      keys: asText(source.keys),
      note: '',
      hasConflict: false,
    };
    nextRows.push(next);

    if (!next.enabled) {
      continue;
    }
    if (!next.keys) {
      hasMissingShortcut = true;
      next.hasConflict = true;
      next.note = missingMessage;
      continue;
    }
    if (!buckets.has(next.trigger)) {
      buckets.set(next.trigger, []);
    }
    buckets.get(next.trigger).push(next);
  }

  for (const groupedRows of buckets.values()) {
    if (groupedRows.length <= 1) {
      continue;
    }
    hasDuplicateTrigger = true;
    for (const row of groupedRows) {
      row.hasConflict = true;
      row.note = duplicateMessage;
    }
  }

  return {
    rows: nextRows,
    hasMissingShortcut,
    hasDuplicateTrigger,
  };
}

export function listTemplateOptions(provider, kind, translate) {
  if (!provider || typeof provider.list !== 'function') {
    return [];
  }
  const raw = provider.list(kind, translate);
  const out = [];
  for (const item of asArray(raw)) {
    const source = asObject(item);
    const id = asText(source.id);
    if (!id) {
      continue;
    }
    const label = asText(source.label) || id;
    out.push({ id, label });
  }
  return out;
}

export function readTemplateBindings(provider, kind, templateId, options, fallbackTrigger) {
  if (!provider || typeof provider.mappings !== 'function') {
    return [];
  }
  const id = asText(templateId);
  if (!id) {
    return [];
  }
  const raw = provider.mappings(kind, id);
  return normalizeBindings(raw, options, fallbackTrigger);
}

export function upsertRowsByTrigger(rows, templateBindings, options, fallbackTrigger, createRow) {
  let nextRows = asArray(rows).map((row) => ({ ...row }));
  const normalizedFallback = defaultOptionValue(options, fallbackTrigger || '');

  for (const binding of asArray(templateBindings)) {
    const triggerChain = normalizeTriggerChain(
      binding.triggerChain || binding.trigger,
      options,
      normalizedFallback);
    const trigger = serializeTriggerChain(triggerChain, options, normalizedFallback);
    const keys = asText(binding.keys);
    if (!trigger || !keys) {
      continue;
    }

    const index = nextRows.findIndex((row) => {
      const source = asObject(row);
      const text = serializeTriggerChain(
        source.triggerChain || source.trigger,
        options,
        normalizedFallback);
      return text === trigger;
    });
    if (index >= 0) {
      nextRows[index] = {
        ...nextRows[index],
        enabled: binding.enabled !== false,
        triggerChain,
        trigger,
        keys,
      };
      continue;
    }
    nextRows.push(createRow(binding));
  }
  return nextRows;
}
