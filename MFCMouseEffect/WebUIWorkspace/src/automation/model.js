import {
  DEFAULT_GESTURE_MAX_DIRECTIONS,
  DEFAULT_GESTURE_MATCH_THRESHOLD_PERCENT,
  DEFAULT_GESTURE_MIN_DISTANCE,
  DEFAULT_GESTURE_PATTERN_MODE,
  DEFAULT_GESTURE_SAMPLE_STEP,
  DEFAULT_GESTURE_TRIGGER_BUTTON,
} from './defaults.js';
import {
  defaultProcessSuffix,
  normalizeRuntimePlatform,
  PLATFORM_WINDOWS,
} from './platform.js';
import { normalizeTriggerChain, serializeTriggerChain } from './trigger-chain.js';

const APP_SCOPE_ALL = 'all';
const APP_SCOPE_SELECTED = 'selected';
const APP_SCOPE_PROCESS = 'process';
const PROCESS_SCOPE_PREFIX = 'process:';
const MAX_CUSTOM_GESTURE_STROKES = 4;

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

function normalizeScopeMode(value) {
  const mode = asText(value).toLowerCase();
  if (mode === APP_SCOPE_SELECTED || mode === APP_SCOPE_PROCESS) {
    return APP_SCOPE_SELECTED;
  }
  return APP_SCOPE_ALL;
}

function normalizeAppProcessName(value, platform = PLATFORM_WINDOWS) {
  const trimmed = asText(value).toLowerCase().replace(/\\/g, '/');
  if (!trimmed) {
    return '';
  }
  const parts = trimmed.split('/');
  const tail = `${parts[parts.length - 1] || ''}`.trim();
  if (!tail) {
    return '';
  }
  if (tail.includes('.')) {
    return tail;
  }
  const suffix = defaultProcessSuffix(platform);
  if (!suffix) {
    return tail;
  }
  return `${tail}.${suffix}`;
}

function normalizeAppList(items, platform = PLATFORM_WINDOWS) {
  const source = Array.isArray(items) ? items : [items];
  const out = [];
  for (const item of source) {
    const text = asText(item);
    if (!text) {
      continue;
    }
    const segments = text.split(/[;,]/);
    for (const segment of segments) {
      const normalized = normalizeAppProcessName(segment, platform);
      if (!normalized) {
        continue;
      }
      if (!out.includes(normalized)) {
        out.push(normalized);
      }
    }
  }
  return out;
}

function parseScopeTokens(rawScopes, platform = PLATFORM_WINDOWS) {
  const tokens = Array.isArray(rawScopes) ? rawScopes : [rawScopes];
  const apps = [];

  for (const token of tokens) {
    const text = asText(token).toLowerCase();
    if (!text) {
      continue;
    }
    if (text === APP_SCOPE_ALL || text === 'global' || text === '*') {
      return {
        mode: APP_SCOPE_ALL,
        apps: [],
        appScopes: [APP_SCOPE_ALL],
      };
    }

    const processSource = text.startsWith(PROCESS_SCOPE_PREFIX)
      ? text.slice(PROCESS_SCOPE_PREFIX.length)
      : text;
    const process = normalizeAppProcessName(processSource, platform);
    if (!process) {
      continue;
    }
    if (!apps.includes(process)) {
      apps.push(process);
    }
  }

  if (apps.length === 0) {
    return {
      mode: APP_SCOPE_ALL,
      apps: [],
      appScopes: [APP_SCOPE_ALL],
    };
  }
  return {
    mode: APP_SCOPE_SELECTED,
    apps,
    appScopes: apps.map((app) => `${PROCESS_SCOPE_PREFIX}${app}`),
  };
}

function parseBindingScope(source, platform = PLATFORM_WINDOWS) {
  const obj = asObject(source);

  // Editor row state has explicit mode fields. Keep mode semantics even when
  // current app list is empty, so UI can render "selected apps" controls.
  if (obj.appScopeMode !== undefined || obj.appScopeType !== undefined) {
    const mode = normalizeScopeMode(obj.appScopeMode || obj.appScopeType);
    if (mode === APP_SCOPE_ALL) {
      return {
        mode: APP_SCOPE_ALL,
        apps: [],
        appScopes: [APP_SCOPE_ALL],
      };
    }
    const apps = normalizeAppList(obj.appScopeApps || obj.appScopeProcess, platform);
    return {
      mode: APP_SCOPE_SELECTED,
      apps,
      appScopes: apps.map((app) => `${PROCESS_SCOPE_PREFIX}${app}`),
    };
  }

  if (obj.app_scopes !== undefined || obj.appScopes !== undefined) {
    return parseScopeTokens(obj.app_scopes !== undefined ? obj.app_scopes : obj.appScopes, platform);
  }
  if (obj.app_scope !== undefined || obj.appScope !== undefined) {
    return parseScopeTokens(obj.app_scope !== undefined ? obj.app_scope : obj.appScope, platform);
  }

  return parseScopeTokens(APP_SCOPE_ALL, platform);
}

function canonicalScopeKey(scope) {
  if (!scope || scope.mode !== APP_SCOPE_SELECTED) {
    return APP_SCOPE_ALL;
  }
  return scope.appScopes.slice().sort().join('|');
}

function clampNumber(value, min, max, fallback) {
  const parsed = Number(value);
  if (!Number.isFinite(parsed)) {
    return fallback;
  }
  return Math.min(max, Math.max(min, Math.round(parsed)));
}

function normalizeModifierMode(value) {
  const mode = asText(value).toLowerCase();
  if (mode === 'none' || mode === 'exact') {
    return mode;
  }
  return 'any';
}

function normalizeModifierFlags(source) {
  const obj = asObject(source);
  return {
    mode: normalizeModifierMode(obj.mode),
    primary: !!obj.primary,
    shift: !!obj.shift,
    alt: !!obj.alt,
  };
}

function canonicalModifierKey(modifiers) {
  const normalized = normalizeModifierFlags(modifiers);
  if (normalized.mode !== 'exact') {
    return normalized.mode;
  }
  return [
    normalized.primary ? 'primary' : '',
    normalized.shift ? 'shift' : '',
    normalized.alt ? 'alt' : '',
  ].filter(Boolean).join('+');
}

function normalizeGesturePatternMode(value) {
  const mode = asText(value).toLowerCase();
  return mode === 'custom' ? 'custom' : DEFAULT_GESTURE_PATTERN_MODE;
}

function normalizeGestureTriggerButton(value, fallback = DEFAULT_GESTURE_TRIGGER_BUTTON) {
  const button = asText(value).toLowerCase();
  if (button === 'left' || button === 'middle' || button === 'right') {
    return button;
  }
  return fallback;
}

function normalizeGestureCustomPoints(value) {
  const points = [];
  for (const item of asArray(value)) {
    const source = asObject(item);
    const x = clampNumber(source.x, 0, 100, 0);
    const y = clampNumber(source.y, 0, 100, 0);
    points.push({ x, y });
  }
  return points;
}

function normalizeGestureCustomStrokes(value) {
  const out = [];
  for (const stroke of asArray(value)) {
    const points = normalizeGestureCustomPoints(Array.isArray(stroke) ? stroke : stroke?.points);
    if (points.length === 0) {
      continue;
    }
    out.push(points);
    if (out.length >= MAX_CUSTOM_GESTURE_STROKES) {
      break;
    }
  }
  return out;
}

function flattenGestureCustomStrokes(strokes) {
  const out = [];
  for (const stroke of asArray(strokes)) {
    for (const point of asArray(stroke)) {
      out.push(point);
    }
  }
  return out;
}

function normalizeGesturePattern(source) {
  const obj = asObject(source);
  const customStrokes = normalizeGestureCustomStrokes(
    obj.customStrokes !== undefined
      ? obj.customStrokes
      : obj.custom_strokes);
  const legacyCustomPoints = normalizeGestureCustomPoints(
    obj.customPoints !== undefined
      ? obj.customPoints
      : obj.custom_points);
  const normalizedStrokes = customStrokes.length > 0
    ? customStrokes
    : (legacyCustomPoints.length > 0 ? [legacyCustomPoints] : []);
  const customPoints = flattenGestureCustomStrokes(normalizedStrokes);
  return {
    mode: normalizeGesturePatternMode(obj.mode),
    matchThresholdPercent: clampNumber(
      obj.matchThresholdPercent !== undefined ? obj.matchThresholdPercent : obj.match_threshold_percent,
      50,
      95,
      DEFAULT_GESTURE_MATCH_THRESHOLD_PERCENT),
    customPoints,
    customStrokes: normalizedStrokes,
  };
}

function canonicalGesturePatternKey(pattern, trigger) {
  const normalized = normalizeGesturePattern(pattern);
  if (normalized.mode !== 'custom') {
    return `preset:${asText(trigger).toLowerCase()}`;
  }
  const strokesKey = normalized.customStrokes
    .map((stroke) => stroke.map((point) => `${point.x},${point.y}`).join(';'))
    .join('|');
  return `custom:${normalized.matchThresholdPercent}:${strokesKey}`;
}

export function parseAppScopes(value, platform = PLATFORM_WINDOWS) {
  return parseScopeTokens(value, platform);
}

export function serializeAppScopes(mode, apps, platform = PLATFORM_WINDOWS) {
  if (normalizeScopeMode(mode) !== APP_SCOPE_SELECTED) {
    return [APP_SCOPE_ALL];
  }
  const normalizedApps = normalizeAppList(apps, platform);
  return normalizedApps.map((app) => `${PROCESS_SCOPE_PREFIX}${app}`);
}

export function serializeLegacyAppScope(mode, apps, platform = PLATFORM_WINDOWS) {
  if (normalizeScopeMode(mode) !== APP_SCOPE_SELECTED) {
    return APP_SCOPE_ALL;
  }
  const normalized = serializeAppScopes(mode, apps, platform);
  if (normalized.length === 0) {
    return PROCESS_SCOPE_PREFIX;
  }
  return normalized[0];
}

function normalizeAppScopeOptions(items) {
  const base = normalizeOptions(items);
  const out = [];
  const seen = new Set();

  for (const item of base) {
    const value = normalizeScopeMode(item.value);
    if (seen.has(value)) {
      continue;
    }
    seen.add(value);
    out.push({
      ...item,
      value,
    });
  }

  if (!seen.has(APP_SCOPE_ALL)) {
    out.unshift({ value: APP_SCOPE_ALL, label: 'All Apps' });
  }
  if (!seen.has(APP_SCOPE_SELECTED)) {
    out.push({ value: APP_SCOPE_SELECTED, label: 'Selected Apps (Multi)' });
  }
  return out;
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

function normalizeBinding(item, options, fallbackTrigger, platform = PLATFORM_WINDOWS, gestureTriggerButtonFallback = DEFAULT_GESTURE_TRIGGER_BUTTON) {
  const source = asObject(item);
  const triggerChain = normalizeTriggerChain(source.triggerChain || source.trigger, options, fallbackTrigger);
  const scope = parseBindingScope(source, platform);
  const gesturePattern = normalizeGesturePattern(source.gesturePattern !== undefined ? source.gesturePattern : source.gesture_pattern);
  return {
    enabled: source.enabled !== false,
    triggerChain,
    trigger: serializeTriggerChain(triggerChain, options, fallbackTrigger),
    appScopeMode: scope.mode,
    appScopeApps: scope.apps,
    appScopes: scope.appScopes,
    appScope: scope.appScopes[0] || APP_SCOPE_ALL,
    appScopeDraft: asText(source.appScopeDraft),
    triggerButton: normalizeGestureTriggerButton(
      source.triggerButton !== undefined ? source.triggerButton : source.trigger_button,
      gestureTriggerButtonFallback),
    gesturePattern,
    modifiers: normalizeModifierFlags(source.modifiers),
    keys: asText(source.keys),
  };
}

export function normalizeBindings(items, options, fallbackTrigger, platform = PLATFORM_WINDOWS, gestureTriggerButtonFallback = DEFAULT_GESTURE_TRIGGER_BUTTON) {
  const out = [];
  for (const item of asArray(items)) {
    out.push(normalizeBinding(item, options, fallbackTrigger, platform, gestureTriggerButtonFallback));
  }
  return out;
}

export function normalizeAutomationPayload(schema, payloadState) {
  const schemaObj = asObject(schema);
  const payload = asObject(payloadState);
  const gesture = asObject(payload.gesture);
  const runtimePlatform = normalizeRuntimePlatform(schemaObj?.capabilities?.platform);

  const mouseOptions = normalizeOptions(schemaObj.automation_mouse_actions);
  const appScopeOptions = normalizeAppScopeOptions(schemaObj.automation_app_scopes);
  const modifierModeOptions = normalizeOptions(schemaObj.automation_modifier_modes);
  const gestureOptions = normalizeOptions(schemaObj.automation_gesture_patterns);
  const gestureButtonOptions = normalizeOptions(schemaObj.automation_gesture_buttons);

  const defaultMouseTrigger = defaultOptionValue(mouseOptions, '');
  const defaultGestureTrigger = defaultOptionValue(gestureOptions, '');
  const defaultGestureButton = defaultOptionValue(gestureButtonOptions, DEFAULT_GESTURE_TRIGGER_BUTTON);

  return {
    mouseOptions,
    appScopeOptions,
    modifierModeOptions,
    gestureOptions,
    gestureButtonOptions,
    platform: runtimePlatform,
    enabled: !!payload.enabled,
    mouseMappings: normalizeBindings(payload.mouse_mappings, mouseOptions, defaultMouseTrigger, runtimePlatform),
    gestureEnabled: !!gesture.enabled,
    gestureTriggerButton: sanitizeOptionValue(
      gesture.trigger_button,
      gestureButtonOptions,
      defaultGestureButton),
    gestureMinDistance: asNumber(gesture.min_stroke_distance_px, DEFAULT_GESTURE_MIN_DISTANCE),
    gestureSampleStep: asNumber(gesture.sample_step_px, DEFAULT_GESTURE_SAMPLE_STEP),
    gestureMaxDirections: asNumber(gesture.max_directions, DEFAULT_GESTURE_MAX_DIRECTIONS),
    gestureMappings: normalizeBindings(
      gesture.mappings,
      gestureOptions,
      defaultGestureTrigger,
      runtimePlatform,
      sanitizeOptionValue(gesture.trigger_button, gestureButtonOptions, defaultGestureButton)),
    defaultMouseTrigger,
    defaultGestureTrigger,
    defaultGestureButton,
  };
}

export function readMappings(rows, options, fallbackTrigger, platform = PLATFORM_WINDOWS) {
  const normalizedFallback = defaultOptionValue(options, fallbackTrigger || '');
  const out = [];
  for (const row of asArray(rows)) {
    const source = asObject(row);

    const triggerChain = normalizeTriggerChain(
      source.triggerChain || source.trigger,
      options,
      normalizedFallback);
    const scopeMode = normalizeScopeMode(source.appScopeMode || source.appScopeType);
    const scopeApps = normalizeAppList(source.appScopeApps || source.appScopeProcess, platform);
    const appScopes = serializeAppScopes(scopeMode, scopeApps, platform);

    out.push({
      enabled: source.enabled !== false,
      trigger: serializeTriggerChain(triggerChain, options, normalizedFallback),
      app_scope: serializeLegacyAppScope(scopeMode, scopeApps, platform),
      app_scopes: appScopes,
      trigger_button: normalizeGestureTriggerButton(source.triggerButton),
      gesture_pattern: normalizeGesturePattern(source.gesturePattern),
      modifiers: normalizeModifierFlags(source.modifiers),
      keys: asText(source.keys),
    });
  }
  return out;
}

export function evaluateRows(rows, options, fallbackTrigger, messages, platform = PLATFORM_WINDOWS) {
  const normalizedFallback = defaultOptionValue(options, fallbackTrigger || '');
  const duplicateMessage = asText(messages?.duplicate);
  const invalidScopeMessage = asText(messages?.invalidScope);

  const nextRows = [];
  const buckets = new Map();
  let hasMissingShortcut = false;
  let hasDuplicateTrigger = false;
  let hasInvalidScope = false;

  for (const row of asArray(rows)) {
    const source = asObject(row);
    const triggerChain = normalizeTriggerChain(
      source.triggerChain || source.trigger,
      options,
      normalizedFallback);
    const trigger = serializeTriggerChain(triggerChain, options, normalizedFallback);
    const scope = parseBindingScope(source, platform);
    const next = {
      ...source,
      enabled: source.enabled !== false,
      triggerChain,
      trigger,
      appScopeMode: scope.mode,
      appScopeApps: scope.apps,
      appScopes: scope.appScopes,
      appScope: scope.appScopes[0] || APP_SCOPE_ALL,
      appScopeDraft: asText(source.appScopeDraft),
      triggerButton: normalizeGestureTriggerButton(source.triggerButton),
      gesturePattern: normalizeGesturePattern(source.gesturePattern),
      modifiers: normalizeModifierFlags(source.modifiers),
      keys: asText(source.keys),
      note: '',
      hasConflict: false,
    };
    nextRows.push(next);

    if (!next.enabled) {
      continue;
    }
    if (next.appScopeMode === APP_SCOPE_SELECTED && next.appScopeApps.length === 0) {
      hasInvalidScope = true;
      next.hasConflict = true;
      next.note = invalidScopeMessage;
      continue;
    }
    if (!next.keys) {
      continue;
    }

    if (next.appScopeMode === APP_SCOPE_SELECTED) {
      for (const app of next.appScopeApps) {
        const bucketKey = `${canonicalGesturePatternKey(next.gesturePattern, next.trigger)}|${normalizeGestureTriggerButton(next.triggerButton)}|${PROCESS_SCOPE_PREFIX}${app}|${canonicalModifierKey(next.modifiers)}`;
        if (!buckets.has(bucketKey)) {
          buckets.set(bucketKey, []);
        }
        buckets.get(bucketKey).push(next);
      }
      continue;
    }

    const bucketKey = `${canonicalGesturePatternKey(next.gesturePattern, next.trigger)}|${normalizeGestureTriggerButton(next.triggerButton)}|all|${canonicalModifierKey(next.modifiers)}`;
    if (!buckets.has(bucketKey)) {
      buckets.set(bucketKey, []);
    }
    buckets.get(bucketKey).push(next);
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
    hasInvalidScope,
  };
}

export function listTemplateOptions(provider, kind, translate, platform = PLATFORM_WINDOWS) {
  if (!provider || typeof provider.list !== 'function') {
    return [];
  }
  const raw = provider.list(kind, translate, normalizeRuntimePlatform(platform));
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

export function readTemplateBindings(provider, kind, templateId, options, fallbackTrigger, platform = PLATFORM_WINDOWS, gestureTriggerButtonFallback = DEFAULT_GESTURE_TRIGGER_BUTTON) {
  if (!provider || typeof provider.mappings !== 'function') {
    return [];
  }
  const id = asText(templateId);
  if (!id) {
    return [];
  }
  const raw = provider.mappings(kind, id);
  return normalizeBindings(raw, options, fallbackTrigger, platform, gestureTriggerButtonFallback);
}

export function upsertRowsByTrigger(rows, templateBindings, options, fallbackTrigger, createRow, platform = PLATFORM_WINDOWS) {
  let nextRows = asArray(rows).map((row) => ({ ...row }));
  const normalizedFallback = defaultOptionValue(options, fallbackTrigger || '');

  for (const binding of asArray(templateBindings)) {
    const triggerChain = normalizeTriggerChain(
      binding.triggerChain || binding.trigger,
      options,
      normalizedFallback);
    const trigger = serializeTriggerChain(triggerChain, options, normalizedFallback);
    const scope = parseBindingScope(binding, platform);
    const scopeKey = canonicalScopeKey(scope);
    const modifierKey = canonicalModifierKey(binding.modifiers);
    const gesturePatternKey = canonicalGesturePatternKey(binding.gesturePattern, trigger);
    const triggerButtonKey = normalizeGestureTriggerButton(binding.triggerButton);
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
      const rowScope = parseBindingScope(source, platform);
      return text === trigger &&
        canonicalScopeKey(rowScope) === scopeKey &&
        canonicalGesturePatternKey(source.gesturePattern, text) === gesturePatternKey &&
        normalizeGestureTriggerButton(source.triggerButton) === triggerButtonKey &&
        canonicalModifierKey(source.modifiers) === modifierKey;
    });
    if (index >= 0) {
      nextRows[index] = {
        ...nextRows[index],
        enabled: binding.enabled !== false,
        triggerChain,
        trigger,
        appScopeMode: scope.mode,
        appScopeApps: scope.apps,
        appScopes: scope.appScopes,
        appScope: scope.appScopes[0] || APP_SCOPE_ALL,
        triggerButton: triggerButtonKey,
        gesturePattern: normalizeGesturePattern(binding.gesturePattern),
        modifiers: normalizeModifierFlags(binding.modifiers),
        keys,
      };
      continue;
    }
    nextRows.push(createRow(binding));
  }
  return nextRows;
}
