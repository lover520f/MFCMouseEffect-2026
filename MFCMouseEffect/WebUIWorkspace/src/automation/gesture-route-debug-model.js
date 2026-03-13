import { isMacosPlatform } from './platform.js';

function asObject(value) {
  return value && typeof value === 'object' ? value : null;
}

function asText(value) {
  return `${value || ''}`.trim();
}

function asNumber(value, fallback = 0) {
  const parsed = Number(value);
  return Number.isFinite(parsed) ? parsed : fallback;
}

function asBool(value) {
  return value === true;
}

function asPreviewPoints(value) {
  if (!Array.isArray(value)) {
    return [];
  }
  const out = [];
  for (const item of value) {
    const point = asObject(item);
    if (!point) {
      continue;
    }
    const x = Number(point.x);
    const y = Number(point.y);
    if (!Number.isFinite(x) || !Number.isFinite(y)) {
      continue;
    }
    out.push({ x, y });
  }
  return out;
}

function normalizeGestureRouteEvent(value) {
  const source = asObject(value);
  if (!source) {
    return null;
  }
  const modifiers = asObject(source.modifiers) || {};
  const seq = asNumber(source.seq, 0);
  const stage = asText(source.stage);
  const reason = asText(source.reason);
  const triggerButton = asText(source.trigger_button);
  if (seq <= 0 && !stage && !reason) {
    return null;
  }
  return {
    seq,
    timestampMs: asNumber(source.timestamp_ms, 0),
    stage,
    reason,
    gestureId: asText(source.gesture_id),
    recognizedGestureId: asText(source.recognized_gesture_id || source.gesture_id),
    matchedGestureId: asText(source.matched_gesture_id),
    triggerButton,
    matched: asBool(source.matched),
    injected: asBool(source.injected),
    usedCustom: asBool(source.used_custom),
    usedPreset: asBool(source.used_preset),
    samplePointCount: asNumber(source.sample_point_count, 0),
    candidateCount: asNumber(source.candidate_count, 0),
    bestWindowStart: asNumber(source.best_window_start, -1),
    bestWindowEnd: asNumber(source.best_window_end, -1),
    runnerUpScore: asNumber(source.runner_up_score, -1),
    previewPathHash: asNumber(source.preview_path_hash, 0),
    previewPoints: asPreviewPoints(source.preview_points),
    modifiers: {
      primary: asBool(modifiers.primary),
      shift: asBool(modifiers.shift),
      alt: asBool(modifiers.alt),
    },
  };
}

export function normalizeGestureRouteStatus(state) {
  const source = asObject(state)?.input_automation_gesture_route_status;
  if (!asObject(source)) {
    return null;
  }
  const modifiers = asObject(source.last_modifiers) || {};
  return {
    automationEnabled: source.automation_enabled === true,
    gestureEnabled: source.gesture_enabled === true,
    buttonlessGestureEnabled: source.buttonless_gesture_enabled === true,
    pointerButtonDown: source.pointer_button_down === true,
    gestureMappingCount: asNumber(source.gesture_mapping_count, 0),
    buttonlessGestureMappingCount: asNumber(source.buttonless_gesture_mapping_count, 0),
    lastStage: asText(source.last_stage),
    lastReason: asText(source.last_reason),
    lastGestureId: asText(source.last_gesture_id),
    lastRecognizedGestureId: asText(source.last_recognized_gesture_id || source.last_gesture_id),
    lastMatchedGestureId: asText(source.last_matched_gesture_id),
    lastTriggerButton: asText(source.last_trigger_button),
    lastMatched: source.last_matched === true,
    lastInjected: source.last_injected === true,
    lastUsedCustom: source.last_used_custom === true,
    lastUsedPreset: source.last_used_preset === true,
    lastSamplePointCount: asNumber(source.last_sample_point_count, 0),
    lastCandidateCount: asNumber(source.last_candidate_count, 0),
    lastBestWindowStart: asNumber(source.last_best_window_start, -1),
    lastBestWindowEnd: asNumber(source.last_best_window_end, -1),
    lastRunnerUpScore: asNumber(source.last_runner_up_score, -1),
    lastPreviewPathHash: asNumber(source.last_preview_path_hash, 0),
    lastPreviewPoints: asPreviewPoints(source.last_preview_points),
    lastEventSeq: asNumber(source.last_event_seq, 0),
    modifiers: {
      primary: modifiers.primary === true,
      shift: modifiers.shift === true,
      alt: modifiers.alt === true,
    },
    recentEvents: (Array.isArray(source.recent_events) ? source.recent_events : [])
      .map(normalizeGestureRouteEvent)
      .filter((event) => !!event),
  };
}

function textByKey(texts, key, fallback) {
  const value = asText(asObject(texts)?.[key]);
  return value || fallback;
}

export function boolLabel(value, texts = {}) {
  return value
    ? textByKey(texts, 'yes', 'Yes')
    : textByKey(texts, 'no', 'No');
}

export function formatGestureRouteModifiers(modifiers, platform, texts = {}) {
  const source = asObject(modifiers) || {};
  const parts = [];
  if (source.primary) {
    parts.push(isMacosPlatform(platform) ? 'Cmd' : 'Ctrl');
  }
  if (source.shift) {
    parts.push('Shift');
  }
  if (source.alt) {
    parts.push(isMacosPlatform(platform) ? 'Option' : 'Alt');
  }
  if (parts.length > 0) {
    return parts.join('+');
  }
  return textByKey(texts, 'modifierEmpty', 'None');
}

export function gestureRouteSourceLabel(routeStatus, texts = {}) {
  if (routeStatus?.lastUsedCustom) {
    return textByKey(texts, 'sourceCustom', 'Custom');
  }
  if (routeStatus?.lastUsedPreset) {
    return textByKey(texts, 'sourcePreset', 'Preset');
  }
  return textByKey(texts, 'sourceUnknown', 'Unknown');
}

export function recentGestureRouteEvents(routeStatus, maxCount = 6) {
  const source = Array.isArray(routeStatus?.recentEvents) ? routeStatus.recentEvents : [];
  if (source.length <= 0) {
    return [];
  }
  return source.slice(-Math.max(1, maxCount)).reverse();
}

export function selectLatestGestureEvent(routeStatus) {
  const events = recentGestureRouteEvents(routeStatus, 30);
  for (const event of events) {
    if (asText(event?.gestureId)) {
      return event;
    }
  }
  return null;
}

function selectLatestEventByGestureField(routeStatus, fieldName) {
  const events = recentGestureRouteEvents(routeStatus, 30);
  for (const event of events) {
    if (asText(event?.[fieldName])) {
      return event;
    }
  }
  return null;
}

export function selectLatestRecognizedGestureEvent(routeStatus) {
  return selectLatestEventByGestureField(routeStatus, 'recognizedGestureId');
}

export function selectLatestMatchedGestureEvent(routeStatus) {
  return selectLatestEventByGestureField(routeStatus, 'matchedGestureId');
}
