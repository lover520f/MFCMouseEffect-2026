import {
  MOUSE_COMPANION_CHECKBOX_FIELDS,
  MOUSE_COMPANION_FLOAT_FIELDS,
  MOUSE_COMPANION_NUMBER_FIELDS,
  MOUSE_COMPANION_RANGE_BINDINGS,
  MOUSE_COMPANION_TEXT_FIELDS,
} from '../mouse-companion/form-contract.js';
import { getMouseCompanionSectionMarkup } from '../mouse-companion/section-template.js';

const DEFAULT_SCHEMA = {
  position_modes: [
    { value: 'relative', label: 'Relative To Cursor' },
    { value: 'absolute', label: 'Absolute Screen Position' },
    { value: 'fixed_bottom_left', label: 'Fixed Bottom Left (Legacy Compatibility)' },
    { value: 'follow', label: 'Follow Cursor (Legacy Compatibility)' },
  ],
  target_monitor_options: [
    { value: 'cursor', label: 'Follow Cursor Screen' },
    { value: 'primary', label: 'Primary Monitor' },
  ],
  edge_clamp_modes: [
    { value: 'soft', label: 'Soft Edge (Recommended)' },
    { value: 'free', label: 'Free (No Clamp)' },
    { value: 'strict', label: 'Strict (In Screen)' },
  ],
  size_px_range: { min: 48, max: 360, step: 1 },
  offset_range: { min: -1200, max: 1200, step: 1 },
  absolute_range: { min: -20000, max: 20000, step: 1 },
  press_lift_px_range: { min: 0, max: 240, step: 1 },
  smoothing_percent_range: { min: 0, max: 95, step: 1 },
  follow_threshold_px_range: { min: 0, max: 32, step: 1 },
  release_hold_ms_range: { min: 0, max: 800, step: 10 },
  click_streak_break_ms_range: { min: 120, max: 3000, step: 10 },
  head_tint_per_click_range: { min: 0.01, max: 1.0, step: 0.01 },
  head_tint_max_range: { min: 0.01, max: 1.0, step: 0.01 },
  head_tint_decay_per_second_range: { min: 0.05, max: 4.0, step: 0.01 },
  test_press_lift_px_range: { min: 0, max: 320, step: 1 },
  test_smoothing_percent_range: { min: 0, max: 95, step: 1 },
  test_click_streak_break_ms_range: { min: 120, max: 3000, step: 10 },
  test_head_tint_per_click_range: { min: 0.01, max: 1.0, step: 0.01 },
  test_head_tint_max_range: { min: 0.01, max: 1.0, step: 0.01 },
  test_head_tint_decay_per_second_range: { min: 0.05, max: 4.0, step: 0.01 },
};

const DEFAULT_STATE = {
  enabled: false,
  model_path: 'MFCMouseEffect/Assets/Pet3D/source/pet-main.glb',
  action_library_path: 'MFCMouseEffect/Assets/Pet3D/source/pet-actions.json',
  appearance_profile_path: 'MFCMouseEffect/Assets/Pet3D/source/pet-appearance.json',
  position_mode: 'fixed_bottom_left',
  edge_clamp_mode: 'soft',
  size_px: 112,
  offset_x: 18,
  offset_y: 26,
  absolute_x: 40,
  absolute_y: 40,
  target_monitor: 'cursor',
  press_lift_px: 24,
  smoothing_percent: 68,
  follow_threshold_px: 2,
  release_hold_ms: 120,
  click_streak_break_ms: 650,
  head_tint_per_click: 0.11,
  head_tint_max: 0.7,
  head_tint_decay_per_second: 0.36,
  use_test_profile: false,
  test_press_lift_px: 48,
  test_smoothing_percent: 32,
  test_click_streak_break_ms: 1200,
  test_head_tint_per_click: 0.2,
  test_head_tint_max: 0.8,
  test_head_tint_decay_per_second: 0.15,
};

let latestSchema = { ...DEFAULT_SCHEMA };
let latestState = { ...DEFAULT_STATE };
let latestRuntimeState = {};

function byId(id) {
  return document.getElementById(id);
}

function normalizeRuntimeText(value, fallback = '-') {
  const text = `${value ?? ''}`.trim();
  return text || fallback;
}

function buildDefaultLaneVerdict(runtimeState) {
  const candidate = `${runtimeState?.default_lane_candidate ?? ''}`.trim();
  const source = `${runtimeState?.default_lane_source ?? ''}`.trim();
  const rolloutStatus = `${runtimeState?.default_lane_rollout_status ?? ''}`.trim();
  const candidateTier = `${runtimeState?.renderer_runtime_default_lane_candidate_tier ?? ''}`.trim();

  const tierLabel =
    candidateTier === 'ship_default_candidate'
      ? 'ship'
      : candidateTier === 'experimental_style_candidate'
        ? 'experimental'
        : candidateTier === 'baseline_reference_candidate'
          ? 'baseline'
          : '';

  if (rolloutStatus === 'candidate_pending_manual_confirmation' && candidate) {
    return tierLabel
      ? `${candidate} ${tierLabel} candidate pending manual confirmation`
      : `${candidate} candidate pending manual confirmation`;
  }
  if (rolloutStatus === 'stay_on_builtin') {
    if (source === 'env_builtin_forced') {
      return 'Stay on builtin (env forced)';
    }
    if (source === 'env_wasm_fallback_builtin') {
      return 'Stay on builtin (wasm fallback)';
    }
    return 'Stay on builtin';
  }
  if (candidate) {
    return `Current candidate: ${candidate}`;
  }
  return '-';
}

function clampInt(value, min, max, fallback) {
  const parsed = Number.parseInt(`${value ?? ''}`, 10);
  const safe = Number.isFinite(parsed) ? parsed : fallback;
  return Math.min(max, Math.max(min, safe));
}

function clampNumber(value, min, max, fallback) {
  const parsed = Number.parseFloat(`${value ?? ''}`);
  const safe = Number.isFinite(parsed) ? parsed : fallback;
  return Math.min(max, Math.max(min, safe));
}

function normalizeRange(value, fallback) {
  const source = value && typeof value === 'object' ? value : {};
  return {
    min: clampNumber(source.min, fallback.min, fallback.max, fallback.min),
    max: clampNumber(source.max, fallback.min, fallback.max, fallback.max),
    step: clampNumber(source.step, 0.001, 1000, fallback.step),
  };
}

function normalizeSchema(value) {
  const source = value && typeof value === 'object' ? value : {};
  const normalizeModeOptions = (input, fallback) => {
    if (!Array.isArray(input)) {
      return [...fallback];
    }
    const normalized = input
      .map((item) => {
        const sourceItem = item && typeof item === 'object' ? item : {};
        const optionValue = `${sourceItem.value ?? ''}`.trim().toLowerCase();
        const label = `${sourceItem.label ?? ''}`.trim();
        if (!optionValue) {
          return null;
        }
        return {
          value: optionValue,
          label: label || optionValue,
        };
      })
      .filter((item) => !!item);
    return normalized.length > 0 ? normalized : [...fallback];
  };
  const normalizeTargetMonitorOptions = (input, fallback) => {
    const normalized = normalizeModeOptions(input, fallback).filter((item) => item.value !== 'custom');
    return normalized.length > 0 ? normalized : [...fallback];
  };
  return {
    position_modes: normalizeModeOptions(source.position_modes, DEFAULT_SCHEMA.position_modes),
    target_monitor_options: normalizeTargetMonitorOptions(
      source.target_monitor_options,
      DEFAULT_SCHEMA.target_monitor_options,
    ),
    edge_clamp_modes: normalizeModeOptions(source.edge_clamp_modes, DEFAULT_SCHEMA.edge_clamp_modes),
    size_px_range: normalizeRange(source.size_px_range, DEFAULT_SCHEMA.size_px_range),
    offset_range: normalizeRange(source.offset_range, DEFAULT_SCHEMA.offset_range),
    absolute_range: normalizeRange(source.absolute_range, DEFAULT_SCHEMA.absolute_range),
    press_lift_px_range: normalizeRange(source.press_lift_px_range, DEFAULT_SCHEMA.press_lift_px_range),
    smoothing_percent_range: normalizeRange(source.smoothing_percent_range, DEFAULT_SCHEMA.smoothing_percent_range),
    follow_threshold_px_range: normalizeRange(source.follow_threshold_px_range, DEFAULT_SCHEMA.follow_threshold_px_range),
    release_hold_ms_range: normalizeRange(source.release_hold_ms_range, DEFAULT_SCHEMA.release_hold_ms_range),
    click_streak_break_ms_range: normalizeRange(
      source.click_streak_break_ms_range,
      DEFAULT_SCHEMA.click_streak_break_ms_range,
    ),
    head_tint_per_click_range: normalizeRange(
      source.head_tint_per_click_range,
      DEFAULT_SCHEMA.head_tint_per_click_range,
    ),
    head_tint_max_range: normalizeRange(source.head_tint_max_range, DEFAULT_SCHEMA.head_tint_max_range),
    head_tint_decay_per_second_range: normalizeRange(
      source.head_tint_decay_per_second_range,
      DEFAULT_SCHEMA.head_tint_decay_per_second_range,
    ),
    test_press_lift_px_range: normalizeRange(
      source.test_press_lift_px_range,
      DEFAULT_SCHEMA.test_press_lift_px_range,
    ),
    test_smoothing_percent_range: normalizeRange(
      source.test_smoothing_percent_range,
      DEFAULT_SCHEMA.test_smoothing_percent_range,
    ),
    test_click_streak_break_ms_range: normalizeRange(
      source.test_click_streak_break_ms_range,
      DEFAULT_SCHEMA.test_click_streak_break_ms_range,
    ),
    test_head_tint_per_click_range: normalizeRange(
      source.test_head_tint_per_click_range,
      DEFAULT_SCHEMA.test_head_tint_per_click_range,
    ),
    test_head_tint_max_range: normalizeRange(
      source.test_head_tint_max_range,
      DEFAULT_SCHEMA.test_head_tint_max_range,
    ),
    test_head_tint_decay_per_second_range: normalizeRange(
      source.test_head_tint_decay_per_second_range,
      DEFAULT_SCHEMA.test_head_tint_decay_per_second_range,
    ),
  };
}

function normalizeState(value) {
  const source = value && typeof value === 'object' ? value : {};
  const modelPath = `${source.model_path || ''}`.trim();
  const actionLibraryPath = `${source.action_library_path || ''}`.trim();
  const appearanceProfilePath = `${source.appearance_profile_path || ''}`.trim();
  const positionMode = `${source.position_mode ?? ''}`.trim().toLowerCase();
  const edgeClampMode = `${source.edge_clamp_mode ?? ''}`.trim().toLowerCase();
  const prodTintPerClick = clampNumber(
    source.head_tint_per_click,
    0.01,
    1.0,
    DEFAULT_STATE.head_tint_per_click,
  );
  const testTintPerClick = clampNumber(
    source.test_head_tint_per_click,
    0.01,
    1.0,
    DEFAULT_STATE.test_head_tint_per_click,
  );
  return {
    enabled: !!source.enabled,
    model_path: modelPath || DEFAULT_STATE.model_path,
    action_library_path: actionLibraryPath || DEFAULT_STATE.action_library_path,
    appearance_profile_path: appearanceProfilePath || DEFAULT_STATE.appearance_profile_path,
    position_mode:
      positionMode === 'relative' ||
      positionMode === 'absolute' ||
      positionMode === 'follow' ||
      positionMode === 'fixed_bottom_left'
        ? positionMode
        : DEFAULT_STATE.position_mode,
    edge_clamp_mode:
      edgeClampMode === 'strict' || edgeClampMode === 'free' || edgeClampMode === 'soft'
        ? edgeClampMode
        : DEFAULT_STATE.edge_clamp_mode,
    size_px: clampInt(source.size_px, 48, 360, DEFAULT_STATE.size_px),
    offset_x: clampInt(source.offset_x, -1200, 1200, DEFAULT_STATE.offset_x),
    offset_y: clampInt(source.offset_y, -1200, 1200, DEFAULT_STATE.offset_y),
    absolute_x: clampInt(source.absolute_x, -20000, 20000, DEFAULT_STATE.absolute_x),
    absolute_y: clampInt(source.absolute_y, -20000, 20000, DEFAULT_STATE.absolute_y),
    target_monitor: `${source.target_monitor ?? ''}`.trim().toLowerCase() || DEFAULT_STATE.target_monitor,
    press_lift_px: clampInt(source.press_lift_px, 0, 240, DEFAULT_STATE.press_lift_px),
    smoothing_percent: clampInt(source.smoothing_percent, 0, 95, DEFAULT_STATE.smoothing_percent),
    follow_threshold_px: clampInt(source.follow_threshold_px, 0, 32, DEFAULT_STATE.follow_threshold_px),
    release_hold_ms: clampInt(source.release_hold_ms, 0, 800, DEFAULT_STATE.release_hold_ms),
    click_streak_break_ms: clampInt(
      source.click_streak_break_ms,
      120,
      3000,
      DEFAULT_STATE.click_streak_break_ms,
    ),
    head_tint_per_click: prodTintPerClick,
    head_tint_max: clampNumber(source.head_tint_max, prodTintPerClick, 1.0, DEFAULT_STATE.head_tint_max),
    head_tint_decay_per_second: clampNumber(
      source.head_tint_decay_per_second,
      0.05,
      4.0,
      DEFAULT_STATE.head_tint_decay_per_second,
    ),
    use_test_profile: !!source.use_test_profile,
    test_press_lift_px: clampInt(source.test_press_lift_px, 0, 320, DEFAULT_STATE.test_press_lift_px),
    test_smoothing_percent: clampInt(
      source.test_smoothing_percent,
      0,
      95,
      DEFAULT_STATE.test_smoothing_percent,
    ),
    test_click_streak_break_ms: clampInt(
      source.test_click_streak_break_ms,
      120,
      3000,
      DEFAULT_STATE.test_click_streak_break_ms,
    ),
    test_head_tint_per_click: testTintPerClick,
    test_head_tint_max: clampNumber(
      source.test_head_tint_max,
      testTintPerClick,
      1.0,
      DEFAULT_STATE.test_head_tint_max,
    ),
    test_head_tint_decay_per_second: clampNumber(
      source.test_head_tint_decay_per_second,
      0.05,
      4.0,
      DEFAULT_STATE.test_head_tint_decay_per_second,
    ),
  };
}

function readChecked(id, fallback) {
  const node = byId(id);
  return node ? !!node.checked : !!fallback;
}

function readText(id, fallback) {
  const node = byId(id);
  return node ? `${node.value || ''}`.trim() : `${fallback ?? ''}`;
}

function readNumber(id, fallback) {
  const node = byId(id);
  return node ? clampInt(node.value, -20000, 20000, fallback) : fallback;
}

function readFloat(id, fallback) {
  const node = byId(id);
  return node ? clampNumber(node.value, -20000, 20000, fallback) : fallback;
}

function writeChecked(id, value) {
  const node = byId(id);
  if (node) {
    node.checked = !!value;
  }
}

function writeInputValue(id, value) {
  const node = byId(id);
  if (node) {
    node.value = `${value ?? ''}`;
  }
}

function writeTextValue(id, value) {
  const node = byId(id);
  if (node) {
    node.textContent = normalizeRuntimeText(value);
  }
}

function applyRange(inputId, range) {
  const node = byId(inputId);
  if (!node || !range) {
    return;
  }
  node.min = `${range.min}`;
  node.max = `${range.max}`;
  node.step = `${range.step}`;
}

function applySelectOptions(selectId, options, selected, fallbackValue, fallbackOptions) {
  const node = byId(selectId);
  if (!node) {
    return;
  }
  const normalized = Array.isArray(options) && options.length > 0
    ? options
    : [...(Array.isArray(fallbackOptions) ? fallbackOptions : [])];
  const selectedValue = `${selected ?? ''}`.trim().toLowerCase();
  node.innerHTML = '';
  for (const option of normalized) {
    const value = `${option?.value ?? ''}`.trim().toLowerCase();
    if (!value) {
      continue;
    }
    const item = document.createElement('option');
    item.value = value;
    item.textContent = `${option?.label ?? value}`;
    node.appendChild(item);
  }
  node.value = selectedValue || `${fallbackValue ?? ''}`;
}

function syncEnabledText() {
  const enabled = readChecked('mc_enabled', latestState.enabled);
  const textNode = byId('mc_enabled_text');
  if (!textNode) {
    return;
  }
  textNode.setAttribute('data-i18n', enabled ? 'text_mouse_companion_on' : 'text_mouse_companion_off');
  textNode.textContent = enabled ? 'Enabled' : 'Disabled';
}

function isRelativePositionMode(mode) {
  return mode === 'relative' || mode === 'follow';
}

function isAbsolutePositionMode(mode) {
  return mode === 'absolute';
}

function syncPositionFieldState() {
  const positionMode = `${byId('mc_position_mode')?.value || latestState.position_mode || ''}`.trim().toLowerCase();
  const relativePair = byId('mc_relative_offset_pair');
  const absolutePair = byId('mc_absolute_pair');
  const targetMonitor = byId('mc_target_monitor');
  const relativeActive = isRelativePositionMode(positionMode);
  const absoluteActive = isAbsolutePositionMode(positionMode);

  if (relativePair) {
    relativePair.style.opacity = relativeActive ? '1' : '0.45';
  }
  for (const id of ['mc_offset_x', 'mc_offset_y']) {
    const node = byId(id);
    if (node) {
      node.disabled = !relativeActive;
    }
  }

  if (absolutePair) {
    absolutePair.style.opacity = absoluteActive ? '1' : '0.45';
  }
  for (const id of ['mc_absolute_x', 'mc_absolute_y']) {
    const node = byId(id);
    if (node) {
      node.disabled = !absoluteActive;
    }
  }

  if (targetMonitor) {
    targetMonitor.disabled = !absoluteActive;
    targetMonitor.style.opacity = absoluteActive ? '1' : '0.45';
  }
}

function mountIfNeeded() {
  const mount = byId('mouse_companion_settings_mount');
  if (!mount) {
    return null;
  }
  if (mount.dataset.mfxMouseCompanionMounted === '1') {
    return mount;
  }

  mount.innerHTML = getMouseCompanionSectionMarkup();

  const enabledToggle = byId('mc_enabled');
  if (enabledToggle) {
    enabledToggle.addEventListener('change', syncEnabledText);
  }
  const positionMode = byId('mc_position_mode');
  if (positionMode) {
    positionMode.addEventListener('change', syncPositionFieldState);
  }

  mount.dataset.mfxMouseCompanionMounted = '1';
  return mount;
}

function readFromDom() {
  const draft = {};
  for (const field of MOUSE_COMPANION_CHECKBOX_FIELDS) {
    draft[field.key] = readChecked(field.id, latestState[field.key] ?? DEFAULT_STATE[field.key]);
  }
  for (const field of MOUSE_COMPANION_TEXT_FIELDS) {
    draft[field.key] = readText(field.id, latestState[field.key] ?? DEFAULT_STATE[field.key]);
  }
  for (const field of MOUSE_COMPANION_NUMBER_FIELDS) {
    draft[field.key] = readNumber(field.id, latestState[field.key] ?? DEFAULT_STATE[field.key]);
  }
  for (const field of MOUSE_COMPANION_FLOAT_FIELDS) {
    draft[field.key] = readFloat(field.id, latestState[field.key] ?? DEFAULT_STATE[field.key]);
  }

  draft.position_mode = readText('mc_position_mode', latestState.position_mode ?? DEFAULT_STATE.position_mode);
  draft.target_monitor = readText('mc_target_monitor', latestState.target_monitor ?? DEFAULT_STATE.target_monitor);
  draft.edge_clamp_mode = readText('mc_edge_clamp_mode', latestState.edge_clamp_mode ?? DEFAULT_STATE.edge_clamp_mode);

  return normalizeState(draft);
}

function writeToDom(state, schema) {
  for (const field of MOUSE_COMPANION_CHECKBOX_FIELDS) {
    writeChecked(field.id, state[field.key]);
  }
  for (const field of MOUSE_COMPANION_TEXT_FIELDS) {
    writeInputValue(field.id, state[field.key] || DEFAULT_STATE[field.key]);
  }
  for (const field of MOUSE_COMPANION_NUMBER_FIELDS) {
    writeInputValue(field.id, state[field.key]);
  }
  for (const field of MOUSE_COMPANION_FLOAT_FIELDS) {
    writeInputValue(field.id, state[field.key]);
  }

  applySelectOptions(
    'mc_position_mode',
    schema.position_modes,
    state.position_mode,
    DEFAULT_STATE.position_mode,
    DEFAULT_SCHEMA.position_modes,
  );
  applySelectOptions(
    'mc_target_monitor',
    schema.target_monitor_options,
    state.target_monitor,
    DEFAULT_STATE.target_monitor,
    DEFAULT_SCHEMA.target_monitor_options,
  );
  applySelectOptions(
    'mc_edge_clamp_mode',
    schema.edge_clamp_modes,
    state.edge_clamp_mode,
    DEFAULT_STATE.edge_clamp_mode,
    DEFAULT_SCHEMA.edge_clamp_modes,
  );

  for (const binding of MOUSE_COMPANION_RANGE_BINDINGS) {
    applyRange(binding.id, schema[binding.schemaKey]);
  }

  syncEnabledText();
  syncPositionFieldState();
}

function writeRuntimeDiagnostics(runtimeState) {
  writeTextValue('mc_runtime_default_lane_verdict', buildDefaultLaneVerdict(runtimeState));
  writeTextValue('mc_runtime_default_lane_candidate', runtimeState.default_lane_candidate);
  writeTextValue('mc_runtime_default_lane_source', runtimeState.default_lane_source);
  writeTextValue('mc_runtime_default_lane_rollout_status', runtimeState.default_lane_rollout_status);
  writeTextValue(
    'mc_runtime_default_lane_style_intent',
    runtimeState.default_lane_style_intent || runtimeState.renderer_runtime_default_lane_style_intent,
  );
  writeTextValue(
    'mc_runtime_default_lane_candidate_tier',
    runtimeState.renderer_runtime_default_lane_candidate_tier,
  );
  writeTextValue(
    'mc_runtime_appearance_plugin_sample_tier',
    runtimeState.renderer_runtime_appearance_plugin_sample_tier,
  );
  writeTextValue(
    'mc_runtime_appearance_plugin_contract_brief',
    runtimeState.renderer_runtime_appearance_plugin_contract_brief,
  );
  writeTextValue(
    'mc_runtime_scene_runtime_adapter_mode',
    runtimeState.renderer_runtime_scene_runtime_adapter_mode,
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_scene_adapter_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_model_scene_adapter_brief,
      'preview_only/unknown/runtime_only',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_scene_seam_readiness',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_model_scene_seam_readiness, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_adapter_brief',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_model_node_adapter_brief, 'preview_only/0.00'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_adapter_influence',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_model_node_adapter_influence, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_channel_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_model_node_channel_brief,
      'body:0.00|face:0.00|appendage:0.00|overlay:0.00|grounding:0.00',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_graph_brief',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_model_node_graph_brief, 'preview_only/0/0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_graph_bound_count',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_model_node_graph_bound_node_count, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_binding_brief',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_model_node_binding_brief, 'preview_only/0/0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_binding_bound_count',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_model_node_binding_bound_entry_count, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_binding_weight_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_model_node_binding_weight_brief,
      'body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_slot_brief',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_model_node_slot_brief, 'preview_only/0/0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_ready_slot_count',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_model_node_ready_slot_count, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_slot_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_model_node_slot_name_brief,
      'body:body_root|head:head_anchor|appendage:appendage_anchor|overlay:overlay_anchor|grounding:grounding_anchor',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_registry_brief',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_model_node_registry_brief, 'preview_only/0/0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_registry_resolved_count',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_model_node_registry_resolved_entry_count, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_registry_asset_node_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_model_node_registry_asset_node_brief,
      'body:asset.body.root|head:asset.head.anchor|appendage:asset.appendage.anchor|overlay:asset.overlay.anchor|grounding:asset.grounding.anchor',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_model_node_registry_weight_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_model_node_registry_weight_brief,
      'body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_binding_brief',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_asset_node_binding_brief, 'preview_only/0/0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_binding_resolved_count',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_asset_node_binding_resolved_entry_count, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_binding_path_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_binding_path_brief,
      'body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_binding_weight_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_binding_weight_brief,
      'body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_transform_brief',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_asset_node_transform_brief, 'preview_only/0/0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_transform_resolved_count',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_asset_node_transform_resolved_entry_count, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_transform_path_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_transform_path_brief,
      'body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_transform_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_transform_value_brief,
      'body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_anchor_brief',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_asset_node_anchor_brief, 'preview_only/0/0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_anchor_resolved_count',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_asset_node_anchor_resolved_entry_count, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_anchor_point_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_anchor_point_brief,
      'body:(0.0,0.0)|head:(0.0,0.0)|appendage:(0.0,0.0)|overlay:(0.0,0.0)|grounding:(0.0,0.0)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_anchor_scale_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_anchor_scale_brief,
      'body:1.00|head:1.00|appendage:1.00|overlay:1.00|grounding:1.00',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_resolver_brief',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_asset_node_resolver_brief, 'preview_only/0/0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_resolver_resolved_count',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_asset_node_resolver_resolved_entry_count, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_resolver_parent_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_resolver_parent_brief,
      'body:root|head:body|appendage:body|overlay:head|grounding:body',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_resolver_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_resolver_value_brief,
      'body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_parent_space_brief',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_asset_node_parent_space_brief, 'preview_only/0/0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_parent_space_resolved_count',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_asset_node_parent_space_resolved_entry_count, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_parent_space_parent_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_parent_space_parent_brief,
      'body:root|head:body|appendage:body|overlay:head|grounding:body',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_parent_space_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_parent_space_value_brief,
      'body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_target_brief',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_asset_node_target_brief, 'preview_only/0/0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_target_resolved_count',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_asset_node_target_resolved_entry_count, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_target_kind_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_target_kind_brief,
      'body:body_target|head:head_target|appendage:appendage_target|overlay:overlay_target|grounding:grounding_target',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_target_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_target_value_brief,
      'body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_target_resolver_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_target_resolver_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_target_resolver_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_target_resolver_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_target_resolver_path_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_target_resolver_path_brief,
      'body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_target_resolver_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_target_resolver_value_brief,
      'body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_world_space_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_world_space_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_world_space_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_world_space_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_world_space_path_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_world_space_path_brief,
      'body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_world_space_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_world_space_value_brief,
      'body:(0.0,0.0,1.00)|head:(0.0,0.0,1.00)|appendage:(0.0,0.0,1.00)|overlay:(0.0,0.0,1.00)|grounding:(0.0,0.0,1.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_path_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_path_brief,
      'body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_value_brief,
      'body:(0.0,0.0,1.00,0.0)|head:(0.0,0.0,1.00,0.0)|appendage:(0.0,0.0,1.00,0.0)|overlay:(0.0,0.0,1.00,0.0)|grounding:(0.0,0.0,1.00,0.0)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_resolver_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_resolver_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_resolver_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_resolver_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_resolver_path_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_resolver_path_brief,
      'body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_resolver_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_resolver_value_brief,
      'body:(0.0,0.0,1.00,0.0)|head:(0.0,0.0,1.00,0.0)|appendage:(0.0,0.0,1.00,0.0)|overlay:(0.0,0.0,1.00,0.0)|grounding:(0.0,0.0,1.00,0.0)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_registry_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_registry_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_registry_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_registry_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_registry_node_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_registry_node_brief,
      'body:pose.body.root|head:pose.head.anchor|appendage:pose.appendage.anchor|overlay:pose.overlay.anchor|grounding:pose.grounding.anchor',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_registry_weight_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_registry_weight_brief,
      'body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_channel_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_channel_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_channel_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_channel_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_channel_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_channel_name_brief,
      'body:channel.body.posture|head:channel.head.expression|appendage:channel.appendage.motion|overlay:channel.overlay.fx|grounding:channel.grounding.shadow',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_channel_weight_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_channel_weight_brief,
      'body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_constraint_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_constraint_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_constraint_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_constraint_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_constraint_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_constraint_name_brief,
      'body:constraint.body.posture|head:constraint.head.expression|appendage:constraint.appendage.motion|overlay:constraint.overlay.fx|grounding:constraint.grounding.shadow',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_constraint_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_constraint_value_brief,
      'body:(0.00,0.0,0.0,1.00,0.0)|head:(0.00,0.0,0.0,1.00,0.0)|appendage:(0.00,0.0,0.0,1.00,0.0)|overlay:(0.00,0.0,0.0,1.00,0.0)|grounding:(0.00,0.0,0.0,1.00,0.0)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_solve_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_solve_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_solve_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_solve_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_solve_path_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_solve_path_brief,
      'body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_solve_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_solve_value_brief,
      'body:(0.00,0.0,0.0,1.00,0.0)|head:(0.00,0.0,0.0,1.00,0.0)|appendage:(0.00,0.0,0.0,1.00,0.0)|overlay:(0.00,0.0,0.0,1.00,0.0)|grounding:(0.00,0.0,0.0,1.00,0.0)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_joint_hint_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_joint_hint_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_joint_hint_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_joint_hint_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_joint_hint_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_joint_hint_name_brief,
      'body:joint.body.spine|head:joint.head.look|appendage:joint.appendage.reach|overlay:joint.overlay.fx|grounding:joint.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_joint_hint_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_joint_hint_value_brief,
      'body:(0.00,0.0,0.0,0.0)|head:(0.00,0.0,0.0,0.0)|appendage:(0.00,0.0,0.0,0.0)|overlay:(0.00,0.0,0.0,0.0)|grounding:(0.00,0.0,0.0,0.0)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_articulation_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_articulation_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_articulation_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_articulation_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_articulation_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_articulation_name_brief,
      'body:articulation.body.spine|head:articulation.head.look|appendage:articulation.appendage.reach|overlay:articulation.overlay.fx|grounding:articulation.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_articulation_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_articulation_value_brief,
      'body:(0.00,0.0,1.00,0.0)|head:(0.00,0.0,1.00,0.0)|appendage:(0.00,0.0,1.00,0.0)|overlay:(0.00,0.0,1.00,0.0)|grounding:(0.00,0.0,1.00,0.0)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_local_joint_registry_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_local_joint_registry_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_local_joint_registry_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_local_joint_registry_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_local_joint_registry_joint_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_local_joint_registry_joint_brief,
      'body:local.body.spine|head:local.head.look|appendage:local.appendage.reach|overlay:local.overlay.fx|grounding:local.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_local_joint_registry_weight_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_local_joint_registry_weight_brief,
      'body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_articulation_map_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_articulation_map_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_articulation_map_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_articulation_map_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_articulation_map_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_articulation_map_name_brief,
      'body:map.body.spine|head:map.head.look|appendage:map.appendage.reach|overlay:map.overlay.fx|grounding:map.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_articulation_map_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_articulation_map_value_brief,
      'body:(0.00,0.0,0.00)|head:(0.00,0.0,0.00)|appendage:(0.00,0.0,0.00)|overlay:(0.00,0.0,0.00)|grounding:(0.00,0.0,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_control_rig_hint_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_control_rig_hint_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_control_rig_hint_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_control_rig_hint_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_control_rig_hint_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_control_rig_hint_name_brief,
      'body:rig.body.spine|head:rig.head.look|appendage:rig.appendage.reach|overlay:rig.overlay.fx|grounding:rig.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_control_rig_hint_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_control_rig_hint_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_rig_channel_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_rig_channel_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_rig_channel_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_rig_channel_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_rig_channel_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_rig_channel_name_brief,
      'body:rig.channel.body.spine|head:rig.channel.head.look|appendage:rig.channel.appendage.reach|overlay:rig.channel.overlay.fx|grounding:rig.channel.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_rig_channel_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_rig_channel_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_control_surface_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_control_surface_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_control_surface_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_control_surface_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_control_surface_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_control_surface_name_brief,
      'body:surface.body.spine|head:surface.head.look|appendage:surface.appendage.reach|overlay:surface.overlay.fx|grounding:surface.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_control_surface_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_control_surface_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_rig_driver_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_rig_driver_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_rig_driver_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_rig_driver_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_rig_driver_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_rig_driver_name_brief,
      'body:rig.driver.body.spine|head:rig.driver.head.look|appendage:rig.driver.appendage.reach|overlay:rig.driver.overlay.fx|grounding:rig.driver.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_rig_driver_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_rig_driver_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_driver_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_driver_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_driver_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_driver_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_driver_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_driver_name_brief,
      'body:surface.driver.body.spine|head:surface.driver.head.look|appendage:surface.driver.appendage.reach|overlay:surface.driver.overlay.fx|grounding:surface.driver.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_driver_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_driver_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_bus_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_bus_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_bus_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_bus_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_bus_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_bus_name_brief,
      'body:pose.bus.body.spine|head:pose.bus.head.look|appendage:pose.bus.appendage.reach|overlay:pose.bus.overlay.fx|grounding:pose.bus.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_pose_bus_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_pose_bus_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_table_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_table_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_table_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_table_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_table_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_table_name_brief,
      'body:controller.body.spine|head:controller.head.look|appendage:controller.appendage.reach|overlay:controller.overlay.fx|grounding:controller.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_table_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_table_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_registry_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_registry_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_registry_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_registry_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_registry_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_registry_name_brief,
      'body:registry.body.spine|head:registry.head.look|appendage:registry.appendage.reach|overlay:registry.overlay.fx|grounding:registry.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_registry_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_registry_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_driver_bus_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_driver_bus_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_driver_bus_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_driver_bus_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_driver_bus_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_driver_bus_name_brief,
      'body:driver.bus.body.spine|head:driver.bus.head.look|appendage:driver.bus.appendage.reach|overlay:driver.bus.overlay.fx|grounding:driver.bus.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_driver_bus_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_driver_bus_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_driver_registry_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_driver_registry_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_driver_registry_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_driver_registry_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_driver_registry_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_driver_registry_name_brief,
      'body:controller.driver.body.spine|head:controller.driver.head.look|appendage:controller.driver.appendage.reach|overlay:controller.driver.overlay.fx|grounding:controller.driver.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_driver_registry_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_driver_registry_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_lane_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_lane_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_lane_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_lane_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_lane_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_lane_name_brief,
      'body:execution.lane.body.spine|head:execution.lane.head.look|appendage:execution.lane.appendage.reach|overlay:execution.lane.overlay.fx|grounding:execution.lane.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_lane_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_lane_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_phase_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_phase_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_phase_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_phase_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_phase_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_phase_name_brief,
      'body:controller.phase.body.spine|head:controller.phase.head.look|appendage:controller.phase.appendage.reach|overlay:controller.phase.overlay.fx|grounding:controller.phase.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_phase_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_phase_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_surface_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_surface_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_surface_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_surface_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_surface_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_surface_name_brief,
      'body:execution.surface.body.shell|head:execution.surface.head.mask|appendage:execution.surface.appendage.trim|overlay:execution.surface.overlay.fx|grounding:execution.surface.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_surface_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_surface_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_phase_registry_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_phase_registry_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_phase_registry_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_phase_registry_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_phase_registry_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_phase_registry_name_brief,
      'body:phase.registry.body.spine|head:phase.registry.head.look|appendage:phase.registry.appendage.reach|overlay:phase.registry.overlay.fx|grounding:phase.registry.grounding.balance',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_controller_phase_registry_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_controller_phase_registry_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_composition_bus_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_composition_bus_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_composition_bus_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_composition_bus_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_composition_bus_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_composition_bus_name_brief,
      'body:surface.bus.body.shell|head:surface.bus.head.mask|appendage:surface.bus.appendage.trim|overlay:surface.bus.overlay.fx|grounding:surface.bus.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_composition_bus_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_composition_bus_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_stack_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_stack_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_stack_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_stack_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_stack_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_stack_name_brief,
      'body:execution.stack.body.shell|head:execution.stack.head.mask|appendage:execution.stack.appendage.trim|overlay:execution.stack.overlay.fx|grounding:execution.stack.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_stack_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_stack_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_stack_router_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_stack_router_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_stack_router_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_stack_router_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_stack_router_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_stack_router_name_brief,
      'body:execution.stack.router.body.shell|head:execution.stack.router.head.mask|appendage:execution.stack.router.appendage.trim|overlay:execution.stack.router.overlay.fx|grounding:execution.stack.router.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_stack_router_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_stack_router_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_stack_router_registry_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_stack_router_registry_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_stack_router_registry_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_stack_router_registry_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_stack_router_registry_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_stack_router_registry_name_brief,
      'body:execution.stack.router.registry.body.shell|head:execution.stack.router.registry.head.mask|appendage:execution.stack.router.registry.appendage.trim|overlay:execution.stack.router.registry.overlay.fx|grounding:execution.stack.router.registry.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_stack_router_registry_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_stack_router_registry_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_composition_registry_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_composition_registry_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_composition_registry_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_composition_registry_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_composition_registry_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_composition_registry_name_brief,
      'body:composition.registry.body.shell|head:composition.registry.head.mask|appendage:composition.registry.appendage.trim|overlay:composition.registry.overlay.fx|grounding:composition.registry.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_composition_registry_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_composition_registry_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_name_brief,
      'body:surface.route.body.shell|head:surface.route.head.mask|appendage:surface.route.appendage.trim|overlay:surface.route.overlay.fx|grounding:surface.route.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_registry_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_registry_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_registry_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_registry_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_registry_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_registry_name_brief,
      'body:surface.route.registry.body.shell|head:surface.route.registry.head.mask|appendage:surface.route.registry.appendage.trim|overlay:surface.route.registry.overlay.fx|grounding:surface.route.registry.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_registry_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_registry_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_router_bus_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_router_bus_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_router_bus_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_router_bus_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_router_bus_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_router_bus_name_brief,
      'body:surface.route.router.bus.body.shell|head:surface.route.router.bus.head.mask|appendage:surface.route.router.bus.appendage.trim|overlay:surface.route.router.bus.overlay.fx|grounding:surface.route.router.bus.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_router_bus_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_router_bus_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_registry_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_registry_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_registry_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_registry_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_registry_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_registry_name_brief,
      'body:surface.route.bus.registry.body.shell|head:surface.route.bus.registry.head.mask|appendage:surface.route.bus.registry.appendage.trim|overlay:surface.route.bus.registry.overlay.fx|grounding:surface.route.bus.registry.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_registry_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_registry_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_name_brief,
      'body:surface.route.bus.driver.body.shell|head:surface.route.bus.driver.head.mask|appendage:surface.route.bus.driver.appendage.trim|overlay:surface.route.bus.driver.overlay.fx|grounding:surface.route.bus.driver.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_name_brief,
      'body:surface.route.bus.driver.registry.body.shell|head:surface.route.bus.driver.registry.head.mask|appendage:surface.route.bus.driver.registry.appendage.trim|overlay:surface.route.bus.driver.registry.overlay.fx|grounding:surface.route.bus.driver.registry.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_name_brief,
      'body:surface.route.bus.driver.registry.router.body.shell|head:surface.route.bus.driver.registry.router.head.mask|appendage:surface.route.bus.driver.registry.router.appendage.trim|overlay:surface.route.bus.driver.registry.router.overlay.fx|grounding:surface.route.bus.driver.registry.router.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_table_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_table_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_table_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_table_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_table_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_table_name_brief,
      'body:execution.driver.body.shell|head:execution.driver.head.mask|appendage:execution.driver.appendage.trim|overlay:execution.driver.overlay.fx|grounding:execution.driver.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_table_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_table_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_table_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_table_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_table_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_table_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_table_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_table_name_brief,
      'body:execution.driver.router.body.shell|head:execution.driver.router.head.mask|appendage:execution.driver.router.appendage.trim|overlay:execution.driver.router.overlay.fx|grounding:execution.driver.router.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_table_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_table_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_name_brief,
      'body:execution.driver.router.registry.body.shell|head:execution.driver.router.registry.head.mask|appendage:execution.driver.router.registry.appendage.trim|overlay:execution.driver.router.registry.overlay.fx|grounding:execution.driver.router.registry.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_name_brief,
      'body:execution.driver.router.registry.bus.body.shell|head:execution.driver.router.registry.bus.head.mask|appendage:execution.driver.router.registry.bus.appendage.trim|overlay:execution.driver.router.registry.bus.overlay.fx|grounding:execution.driver.router.registry.bus.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_brief,
      'preview_only/0/0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_resolved_count',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_resolved_entry_count,
      '0',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_name_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_name_brief,
      'body:execution.driver.router.registry.bus.registry.body.shell|head:execution.driver.router.registry.bus.registry.head.mask|appendage:execution.driver.router.registry.bus.registry.appendage.trim|overlay:execution.driver.router.registry.bus.registry.overlay.fx|grounding:execution.driver.router.registry.bus.registry.grounding.base',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_value_brief',
    normalizeRuntimeText(
      runtimeState.renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_value_brief,
      'body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)',
    ),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_pose_adapter_brief',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_pose_adapter_brief, 'runtime_only/0.00/0.00'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_pose_adapter_influence',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_pose_adapter_influence, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_pose_readability_bias',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_pose_readability_bias, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_pose_sample_count',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_pose_sample_count, '0'),
  );
  writeTextValue(
    'mc_runtime_scene_runtime_bound_pose_sample_count',
    normalizeRuntimeText(runtimeState.renderer_runtime_scene_runtime_bound_pose_sample_count, '0'),
  );
  writeTextValue('mc_runtime_appearance_plugin_kind', runtimeState.renderer_runtime_appearance_plugin_kind);
  writeTextValue(
    'mc_runtime_appearance_semantics_mode',
    runtimeState.renderer_runtime_appearance_semantics_mode,
  );
  writeTextValue(
    'mc_runtime_appearance_plugin_selection_reason',
    runtimeState.renderer_runtime_appearance_plugin_selection_reason,
  );
}

function render(payload) {
  const rootSchema = payload?.schema || {};
  const companionSchema = rootSchema.mouse_companion || rootSchema || {};
  latestSchema = normalizeSchema({
    ...companionSchema,
    target_monitor_options: rootSchema.target_monitor_options ?? companionSchema.target_monitor_options,
    monitors: rootSchema.monitors ?? companionSchema.monitors,
  });
  latestState = normalizeState(payload?.state?.mouse_companion || payload?.state || {});
  latestRuntimeState = payload?.state?.mouse_companion_runtime || {};
  const mount = mountIfNeeded();
  if (!mount) {
    return;
  }
  writeToDom(latestState, latestSchema);
  writeRuntimeDiagnostics(latestRuntimeState);
}

function read() {
  const mount = mountIfNeeded();
  if (!mount) {
    return normalizeState(latestState);
  }
  latestState = readFromDom();
  return latestState;
}

function onAction(_action) {
  // Reserved for future interaction hooks.
}

window.MfxMouseCompanionSection = {
  render,
  read,
  onAction,
};
