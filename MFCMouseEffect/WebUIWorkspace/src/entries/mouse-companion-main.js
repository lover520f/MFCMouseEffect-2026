const DEFAULT_SCHEMA = {
  size_px_range: { min: 48, max: 360, step: 1 },
  offset_range: { min: -1200, max: 1200, step: 1 },
  press_lift_px_range: { min: 0, max: 240, step: 1 },
  smoothing_percent_range: { min: 0, max: 95, step: 1 },
  follow_threshold_px_range: { min: 0, max: 32, step: 1 },
  release_hold_ms_range: { min: 0, max: 800, step: 10 },
  test_press_lift_px_range: { min: 0, max: 320, step: 1 },
  test_smoothing_percent_range: { min: 0, max: 95, step: 1 },
};

const DEFAULT_STATE = {
  enabled: false,
  model_path: 'MFCMouseEffect/Assets/Pet3D/source/pet-main.glb',
  action_library_path: 'MFCMouseEffect/Assets/Pet3D/source/pet-actions.json',
  appearance_profile_path: 'MFCMouseEffect/Assets/Pet3D/source/pet-appearance.json',
  size_px: 112,
  offset_x: 18,
  offset_y: 26,
  press_lift_px: 24,
  smoothing_percent: 68,
  follow_threshold_px: 2,
  release_hold_ms: 120,
  use_test_profile: false,
  test_press_lift_px: 48,
  test_smoothing_percent: 32,
};

const DEFAULT_RUNTIME_STATE = {
  config_enabled: false,
  runtime_present: false,
  visual_host_active: false,
  model_loaded: false,
  visual_model_loaded: false,
  action_library_loaded: false,
  appearance_profile_loaded: false,
  pose_binding_configured: false,
  skeleton_bone_count: 0,
  last_action_name: '',
  last_action_code: -1,
  last_action_intensity: 0,
  last_action_tick_ms: 0,
  loaded_model_path: '',
  visual_model_path: '',
  model_load_error: '',
  visual_model_load_error: '',
  action_coverage: {
    ready: false,
    expected_action_count: 0,
    covered_action_count: 0,
    missing_action_count: 0,
    skeleton_bone_count: 0,
    total_track_count: 0,
    mapped_track_count: 0,
    overall_coverage_ratio: 0,
    error: '',
    missing_actions: [],
    missing_bone_names: [],
    actions: [],
  },
};

let latestSchema = { ...DEFAULT_SCHEMA };
let latestState = { ...DEFAULT_STATE };
let latestRuntimeState = { ...DEFAULT_RUNTIME_STATE };

function byId(id) {
  return document.getElementById(id);
}

function clampInt(value, min, max, fallback) {
  const parsed = Number.parseInt(`${value ?? ''}`, 10);
  const safe = Number.isFinite(parsed) ? parsed : fallback;
  return Math.min(max, Math.max(min, safe));
}

function normalizeRange(value, fallback) {
  const source = value && typeof value === 'object' ? value : {};
  return {
    min: clampInt(source.min, fallback.min, fallback.max, fallback.min),
    max: clampInt(source.max, fallback.min, fallback.max, fallback.max),
    step: clampInt(source.step, 1, 1000, fallback.step),
  };
}

function normalizeSchema(value) {
  const source = value && typeof value === 'object' ? value : {};
  return {
    size_px_range: normalizeRange(source.size_px_range, DEFAULT_SCHEMA.size_px_range),
    offset_range: normalizeRange(source.offset_range, DEFAULT_SCHEMA.offset_range),
    press_lift_px_range: normalizeRange(source.press_lift_px_range, DEFAULT_SCHEMA.press_lift_px_range),
    smoothing_percent_range: normalizeRange(source.smoothing_percent_range, DEFAULT_SCHEMA.smoothing_percent_range),
    follow_threshold_px_range: normalizeRange(source.follow_threshold_px_range, DEFAULT_SCHEMA.follow_threshold_px_range),
    release_hold_ms_range: normalizeRange(source.release_hold_ms_range, DEFAULT_SCHEMA.release_hold_ms_range),
    test_press_lift_px_range: normalizeRange(source.test_press_lift_px_range, DEFAULT_SCHEMA.test_press_lift_px_range),
    test_smoothing_percent_range: normalizeRange(source.test_smoothing_percent_range, DEFAULT_SCHEMA.test_smoothing_percent_range),
  };
}

function normalizeState(value) {
  const source = value && typeof value === 'object' ? value : {};
  const modelPath = `${source.model_path || ''}`.trim();
  const actionLibraryPath = `${source.action_library_path || ''}`.trim();
  const appearanceProfilePath = `${source.appearance_profile_path || ''}`.trim();
  return {
    enabled: !!source.enabled,
    model_path: modelPath || DEFAULT_STATE.model_path,
    action_library_path: actionLibraryPath || DEFAULT_STATE.action_library_path,
    appearance_profile_path: appearanceProfilePath || DEFAULT_STATE.appearance_profile_path,
    size_px: clampInt(source.size_px, 48, 360, DEFAULT_STATE.size_px),
    offset_x: clampInt(source.offset_x, -1200, 1200, DEFAULT_STATE.offset_x),
    offset_y: clampInt(source.offset_y, -1200, 1200, DEFAULT_STATE.offset_y),
    press_lift_px: clampInt(source.press_lift_px, 0, 240, DEFAULT_STATE.press_lift_px),
    smoothing_percent: clampInt(source.smoothing_percent, 0, 95, DEFAULT_STATE.smoothing_percent),
    follow_threshold_px: clampInt(source.follow_threshold_px, 0, 32, DEFAULT_STATE.follow_threshold_px),
    release_hold_ms: clampInt(source.release_hold_ms, 0, 800, DEFAULT_STATE.release_hold_ms),
    use_test_profile: !!source.use_test_profile,
    test_press_lift_px: clampInt(source.test_press_lift_px, 0, 320, DEFAULT_STATE.test_press_lift_px),
    test_smoothing_percent: clampInt(source.test_smoothing_percent, 0, 95, DEFAULT_STATE.test_smoothing_percent),
  };
}

function normalizeRuntimeState(value) {
  const source = value && typeof value === 'object' ? value : {};
  const parseIntSafe = (input, fallback) => {
    const parsed = Number.parseInt(`${input ?? ''}`, 10);
    return Number.isFinite(parsed) ? parsed : fallback;
  };
  const parseStringArray = (input) => {
    if (!Array.isArray(input)) {
      return [];
    }
    return input
      .map((value) => `${value ?? ''}`.trim())
      .filter((value) => value.length > 0);
  };
  const parseCoverageActions = (input) => {
    if (!Array.isArray(input)) {
      return [];
    }
    return input
      .map((entry) => {
        const sourceEntry = entry && typeof entry === 'object' ? entry : {};
        return {
          action_name: textOrEmpty(sourceEntry.action_name),
          clip_present: !!sourceEntry.clip_present,
          track_count: Math.max(0, parseIntSafe(sourceEntry.track_count, 0)),
          mapped_track_count: Math.max(0, parseIntSafe(sourceEntry.mapped_track_count, 0)),
          coverage_ratio: Number.isFinite(Number(sourceEntry.coverage_ratio))
            ? Number(sourceEntry.coverage_ratio)
            : 0,
          missing_bone_tracks: parseStringArray(sourceEntry.missing_bone_tracks),
        };
      })
      .filter((entry) => entry.action_name.length > 0);
  };
  const textOrEmpty = (input) => `${input ?? ''}`.trim();
  const coverageSource =
    source.action_coverage && typeof source.action_coverage === 'object'
      ? source.action_coverage
      : {};
  return {
    config_enabled: !!source.config_enabled,
    runtime_present: !!source.runtime_present,
    visual_host_active: !!source.visual_host_active,
    model_loaded: !!source.model_loaded,
    visual_model_loaded: !!source.visual_model_loaded,
    action_library_loaded: !!source.action_library_loaded,
    appearance_profile_loaded: !!source.appearance_profile_loaded,
    pose_binding_configured: !!source.pose_binding_configured,
    skeleton_bone_count: Math.max(0, parseIntSafe(source.skeleton_bone_count, 0)),
    last_action_name: textOrEmpty(source.last_action_name),
    last_action_code: parseIntSafe(source.last_action_code, -1),
    last_action_intensity: Number.isFinite(Number(source.last_action_intensity))
      ? Number(source.last_action_intensity)
      : 0,
    last_action_tick_ms: Math.max(0, parseIntSafe(source.last_action_tick_ms, 0)),
    loaded_model_path: textOrEmpty(source.loaded_model_path),
    visual_model_path: textOrEmpty(source.visual_model_path),
    model_load_error: textOrEmpty(source.model_load_error),
    visual_model_load_error: textOrEmpty(source.visual_model_load_error),
    action_coverage: {
      ready: !!coverageSource.ready,
      expected_action_count: Math.max(0, parseIntSafe(coverageSource.expected_action_count, 0)),
      covered_action_count: Math.max(0, parseIntSafe(coverageSource.covered_action_count, 0)),
      missing_action_count: Math.max(0, parseIntSafe(coverageSource.missing_action_count, 0)),
      skeleton_bone_count: Math.max(0, parseIntSafe(coverageSource.skeleton_bone_count, 0)),
      total_track_count: Math.max(0, parseIntSafe(coverageSource.total_track_count, 0)),
      mapped_track_count: Math.max(0, parseIntSafe(coverageSource.mapped_track_count, 0)),
      overall_coverage_ratio: Number.isFinite(Number(coverageSource.overall_coverage_ratio))
        ? Number(coverageSource.overall_coverage_ratio)
        : 0,
      error: textOrEmpty(coverageSource.error),
      missing_actions: parseStringArray(coverageSource.missing_actions),
      missing_bone_names: parseStringArray(coverageSource.missing_bone_names),
      actions: parseCoverageActions(coverageSource.actions),
    },
  };
}

function readChecked(id) {
  const node = byId(id);
  return !!(node && node.checked);
}

function readNumber(id, fallback) {
  const node = byId(id);
  if (!node) {
    return fallback;
  }
  return clampInt(node.value, -20000, 20000, fallback);
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

function syncTestFieldState() {
  const useTestProfile = readChecked('mc_use_test_profile');
  const testPressLift = byId('mc_test_press_lift_px');
  const testSmoothing = byId('mc_test_smoothing_percent');
  if (testPressLift) {
    testPressLift.disabled = !useTestProfile;
  }
  if (testSmoothing) {
    testSmoothing.disabled = !useTestProfile;
  }

  const textNode = byId('mc_use_test_profile_text');
  if (textNode) {
    textNode.setAttribute(
      'data-i18n',
      useTestProfile ? 'text_mouse_companion_test_on' : 'text_mouse_companion_test_off',
    );
    textNode.textContent = useTestProfile ? 'Testing' : 'Production';
  }
}

function syncEnabledText() {
  const enabled = readChecked('mc_enabled');
  const textNode = byId('mc_enabled_text');
  if (!textNode) {
    return;
  }
  textNode.setAttribute('data-i18n', enabled ? 'text_mouse_companion_on' : 'text_mouse_companion_off');
  textNode.textContent = enabled ? 'Enabled' : 'Disabled';
}

function syncRuntimeFlagText(id, value) {
  const node = byId(id);
  if (!node) {
    return;
  }
  const key = value ? 'text_mouse_companion_runtime_true' : 'text_mouse_companion_runtime_false';
  node.setAttribute('data-i18n', key);
  node.textContent = value ? 'Yes' : 'No';
  node.classList.toggle('is-true', !!value);
  node.classList.toggle('is-false', !value);
}

function writeRuntimeToDom(runtimeState) {
  syncRuntimeFlagText('mc_runtime_runtime_present', runtimeState.runtime_present);
  syncRuntimeFlagText('mc_runtime_visual_host_active', runtimeState.visual_host_active);
  syncRuntimeFlagText('mc_runtime_model_loaded', runtimeState.model_loaded);
  syncRuntimeFlagText('mc_runtime_visual_model_loaded', runtimeState.visual_model_loaded);
  syncRuntimeFlagText('mc_runtime_action_library_loaded', runtimeState.action_library_loaded);
  syncRuntimeFlagText('mc_runtime_appearance_profile_loaded', runtimeState.appearance_profile_loaded);
  syncRuntimeFlagText('mc_runtime_pose_binding_configured', runtimeState.pose_binding_configured);

  const boneCount = byId('mc_runtime_skeleton_bone_count');
  if (boneCount) {
    boneCount.textContent = `${runtimeState.skeleton_bone_count}`;
  }

  const lastActionName = byId('mc_runtime_last_action_name');
  if (lastActionName) {
    lastActionName.textContent = runtimeState.last_action_name || '-';
  }

  const lastActionCode = byId('mc_runtime_last_action_code');
  if (lastActionCode) {
    lastActionCode.textContent = `${runtimeState.last_action_code}`;
  }

  const lastActionIntensity = byId('mc_runtime_last_action_intensity');
  if (lastActionIntensity) {
    lastActionIntensity.textContent = `${runtimeState.last_action_intensity.toFixed(3)}`;
  }

  const lastActionTick = byId('mc_runtime_last_action_tick_ms');
  if (lastActionTick) {
    lastActionTick.textContent = `${runtimeState.last_action_tick_ms}`;
  }

  const loadedModelPath = byId('mc_runtime_loaded_model_path');
  if (loadedModelPath) {
    loadedModelPath.textContent = runtimeState.loaded_model_path || '-';
    loadedModelPath.title = runtimeState.loaded_model_path || '';
  }

  const visualModelPath = byId('mc_runtime_visual_model_path');
  if (visualModelPath) {
    visualModelPath.textContent = runtimeState.visual_model_path || '-';
    visualModelPath.title = runtimeState.visual_model_path || '';
  }

  const modelLoadError = byId('mc_runtime_model_load_error');
  if (modelLoadError) {
    modelLoadError.textContent = runtimeState.model_load_error || '-';
    modelLoadError.title = runtimeState.model_load_error || '';
    modelLoadError.classList.toggle('is-error', !!runtimeState.model_load_error);
  }

  const visualModelLoadError = byId('mc_runtime_visual_model_load_error');
  if (visualModelLoadError) {
    visualModelLoadError.textContent = runtimeState.visual_model_load_error || '-';
    visualModelLoadError.title = runtimeState.visual_model_load_error || '';
    visualModelLoadError.classList.toggle('is-error', !!runtimeState.visual_model_load_error);
  }

  const actionCoverage = runtimeState.action_coverage || DEFAULT_RUNTIME_STATE.action_coverage;
  syncRuntimeFlagText('mc_runtime_action_coverage_ready', actionCoverage.ready);

  const coverageRatio = byId('mc_runtime_action_coverage_ratio');
  if (coverageRatio) {
    coverageRatio.textContent = `${(actionCoverage.overall_coverage_ratio * 100).toFixed(1)}%`;
  }

  const coverageTracks = byId('mc_runtime_action_coverage_tracks');
  if (coverageTracks) {
    coverageTracks.textContent =
      `${actionCoverage.mapped_track_count}/${actionCoverage.total_track_count}`;
  }

  const coverageActions = byId('mc_runtime_action_coverage_actions');
  if (coverageActions) {
    coverageActions.textContent =
      `${actionCoverage.covered_action_count}/${actionCoverage.expected_action_count}`;
  }

  const coverageMissingActions = byId('mc_runtime_action_coverage_missing_actions');
  if (coverageMissingActions) {
    coverageMissingActions.textContent =
      actionCoverage.missing_actions.length > 0 ? actionCoverage.missing_actions.join(', ') : '-';
  }

  const coverageMissingBones = byId('mc_runtime_action_coverage_missing_bones');
  if (coverageMissingBones) {
    coverageMissingBones.textContent =
      actionCoverage.missing_bone_names.length > 0 ? actionCoverage.missing_bone_names.join(', ') : '-';
  }

  const coverageError = byId('mc_runtime_action_coverage_error');
  if (coverageError) {
    coverageError.textContent = actionCoverage.error || '-';
    coverageError.title = actionCoverage.error || '';
    coverageError.classList.toggle('is-error', !!actionCoverage.error);
  }

  const coverageActionDetails = byId('mc_runtime_action_coverage_action_details');
  if (coverageActionDetails) {
    if (actionCoverage.actions.length <= 0) {
      coverageActionDetails.textContent = '-';
      coverageActionDetails.title = '';
    } else {
      const detailText = actionCoverage.actions
        .map((entry) => {
          const ratio = `${(entry.coverage_ratio * 100).toFixed(1)}%`;
          const missingTracks =
            entry.missing_bone_tracks.length > 0
              ? `, missing=[${entry.missing_bone_tracks.join('|')}]`
              : '';
          return `${entry.action_name}: ${entry.mapped_track_count}/${entry.track_count}, ${ratio}, clip=${entry.clip_present ? 'yes' : 'no'}${missingTracks}`;
        })
        .join(' ; ');
      coverageActionDetails.textContent = detailText;
      coverageActionDetails.title = detailText;
    }
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

  mount.innerHTML = `
    <div class="grid">
      <label for="mc_enabled" class="label-with-tip"><span data-i18n="label_mouse_companion_enabled">Enable Mouse Companion</span></label>
      <label class="startup-toggle" for="mc_enabled">
        <input id="mc_enabled" class="startup-toggle__input" type="checkbox" />
        <span class="startup-toggle__switch" aria-hidden="true"><span class="startup-toggle__thumb"></span></span>
        <span id="mc_enabled_text" class="startup-toggle__text" data-i18n="text_mouse_companion_off">Disabled</span>
      </label>

      <label for="mc_model_path" class="label-with-tip"><span data-i18n="label_mouse_companion_model_path">Model Path (USDZ/GLB)</span></label>
      <input id="mc_model_path" type="text" data-i18n-placeholder="placeholder_mouse_companion_model_path" placeholder="MFCMouseEffect/Assets/Pet3D/source/pet-main.glb" />

      <label for="mc_action_library_path" class="label-with-tip"><span data-i18n="label_mouse_companion_action_library_path">Action Library Path (JSON)</span></label>
      <input id="mc_action_library_path" type="text" data-i18n-placeholder="placeholder_mouse_companion_action_library_path" placeholder="MFCMouseEffect/Assets/Pet3D/source/pet-actions.json" />

      <label for="mc_appearance_profile_path" class="label-with-tip"><span data-i18n="label_mouse_companion_appearance_profile_path">Appearance Profile Path (JSON)</span></label>
      <input id="mc_appearance_profile_path" type="text" data-i18n-placeholder="placeholder_mouse_companion_appearance_profile_path" placeholder="MFCMouseEffect/Assets/Pet3D/source/pet-appearance.json" />

      <label for="mc_size_px" data-i18n="label_mouse_companion_size_px">Companion Size (px)</label>
      <input id="mc_size_px" type="number" />

      <label for="mc_offset_x" data-i18n="label_mouse_companion_offset_x">Offset X</label>
      <input id="mc_offset_x" type="number" />

      <label for="mc_offset_y" data-i18n="label_mouse_companion_offset_y">Offset Y</label>
      <input id="mc_offset_y" type="number" />

      <label for="mc_press_lift_px" data-i18n="label_mouse_companion_press_lift_px">Press Lift (px)</label>
      <input id="mc_press_lift_px" type="number" />

      <label for="mc_smoothing_percent" data-i18n="label_mouse_companion_smoothing_percent">Follow Smoothing (%)</label>
      <input id="mc_smoothing_percent" type="number" />

      <label for="mc_follow_threshold_px" data-i18n="label_mouse_companion_follow_threshold_px">Follow Threshold (px)</label>
      <input id="mc_follow_threshold_px" type="number" />

      <label for="mc_release_hold_ms" data-i18n="label_mouse_companion_release_hold_ms">Release Hold (ms)</label>
      <input id="mc_release_hold_ms" type="number" />

      <label for="mc_use_test_profile" class="label-with-tip"><span data-i18n="label_mouse_companion_use_test_profile">Use Test Profile</span></label>
      <label class="startup-toggle" for="mc_use_test_profile">
        <input id="mc_use_test_profile" class="startup-toggle__input" type="checkbox" />
        <span class="startup-toggle__switch" aria-hidden="true"><span class="startup-toggle__thumb"></span></span>
        <span id="mc_use_test_profile_text" class="startup-toggle__text" data-i18n="text_mouse_companion_test_off">Production</span>
      </label>

      <label for="mc_test_press_lift_px" data-i18n="label_mouse_companion_test_press_lift_px">Test Press Lift (px)</label>
      <input id="mc_test_press_lift_px" type="number" />

      <label for="mc_test_smoothing_percent" data-i18n="label_mouse_companion_test_smoothing_percent">Test Smoothing (%)</label>
      <input id="mc_test_smoothing_percent" type="number" />
    </div>
    <div class="mouse-companion-runtime grid-offset-top">
      <div class="mouse-companion-runtime__title" data-i18n="title_mouse_companion_runtime">Runtime Diagnostics</div>
      <div class="mouse-companion-runtime__grid">
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_runtime_present">Runtime Present</div>
          <div id="mc_runtime_runtime_present" class="mouse-companion-runtime__value">No</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_visual_host_active">Visual Host Active</div>
          <div id="mc_runtime_visual_host_active" class="mouse-companion-runtime__value">No</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_model_loaded">Runtime Model Loaded</div>
          <div id="mc_runtime_model_loaded" class="mouse-companion-runtime__value">No</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_visual_model_loaded">Visual Model Loaded</div>
          <div id="mc_runtime_visual_model_loaded" class="mouse-companion-runtime__value">No</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_action_library_loaded">Action Library Loaded</div>
          <div id="mc_runtime_action_library_loaded" class="mouse-companion-runtime__value">No</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_appearance_profile_loaded">Appearance Loaded</div>
          <div id="mc_runtime_appearance_profile_loaded" class="mouse-companion-runtime__value">No</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_pose_binding_configured">Pose Binding</div>
          <div id="mc_runtime_pose_binding_configured" class="mouse-companion-runtime__value">No</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_skeleton_bone_count">Skeleton Bones</div>
          <div id="mc_runtime_skeleton_bone_count" class="mouse-companion-runtime__value">0</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_last_action_name">Last Action</div>
          <div id="mc_runtime_last_action_name" class="mouse-companion-runtime__value">-</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_last_action_code">Last Action Code</div>
          <div id="mc_runtime_last_action_code" class="mouse-companion-runtime__value">-1</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_last_action_intensity">Last Action Intensity</div>
          <div id="mc_runtime_last_action_intensity" class="mouse-companion-runtime__value">0.000</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_last_action_tick_ms">Last Action Tick (ms)</div>
          <div id="mc_runtime_last_action_tick_ms" class="mouse-companion-runtime__value">0</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_action_coverage_ready">Action Coverage Ready</div>
          <div id="mc_runtime_action_coverage_ready" class="mouse-companion-runtime__value">No</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_action_coverage_ratio">Action Coverage Ratio</div>
          <div id="mc_runtime_action_coverage_ratio" class="mouse-companion-runtime__value">0.0%</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_action_coverage_tracks">Mapped Tracks</div>
          <div id="mc_runtime_action_coverage_tracks" class="mouse-companion-runtime__value">0/0</div>
        </div>
        <div class="mouse-companion-runtime__item">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_action_coverage_actions">Covered Actions</div>
          <div id="mc_runtime_action_coverage_actions" class="mouse-companion-runtime__value">0/0</div>
        </div>
        <div class="mouse-companion-runtime__item mouse-companion-runtime__item--wide">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_action_coverage_action_details">Action Coverage Details</div>
          <div id="mc_runtime_action_coverage_action_details" class="mouse-companion-runtime__value mouse-companion-runtime__value--path">-</div>
        </div>
        <div class="mouse-companion-runtime__item mouse-companion-runtime__item--wide">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_loaded_model_path">Runtime Model Path</div>
          <div id="mc_runtime_loaded_model_path" class="mouse-companion-runtime__value mouse-companion-runtime__value--path">-</div>
        </div>
        <div class="mouse-companion-runtime__item mouse-companion-runtime__item--wide">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_visual_model_path">Visual Model Path</div>
          <div id="mc_runtime_visual_model_path" class="mouse-companion-runtime__value mouse-companion-runtime__value--path">-</div>
        </div>
        <div class="mouse-companion-runtime__item mouse-companion-runtime__item--wide">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_model_load_error">Runtime Model Error</div>
          <div id="mc_runtime_model_load_error" class="mouse-companion-runtime__value mouse-companion-runtime__value--path">-</div>
        </div>
        <div class="mouse-companion-runtime__item mouse-companion-runtime__item--wide">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_visual_model_load_error">Visual Model Error</div>
          <div id="mc_runtime_visual_model_load_error" class="mouse-companion-runtime__value mouse-companion-runtime__value--path">-</div>
        </div>
        <div class="mouse-companion-runtime__item mouse-companion-runtime__item--wide">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_action_coverage_missing_actions">Missing Actions</div>
          <div id="mc_runtime_action_coverage_missing_actions" class="mouse-companion-runtime__value mouse-companion-runtime__value--path">-</div>
        </div>
        <div class="mouse-companion-runtime__item mouse-companion-runtime__item--wide">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_action_coverage_missing_bones">Missing Bone Tracks</div>
          <div id="mc_runtime_action_coverage_missing_bones" class="mouse-companion-runtime__value mouse-companion-runtime__value--path">-</div>
        </div>
        <div class="mouse-companion-runtime__item mouse-companion-runtime__item--wide">
          <div class="mouse-companion-runtime__label" data-i18n="label_mouse_companion_action_coverage_error">Action Coverage Error</div>
          <div id="mc_runtime_action_coverage_error" class="mouse-companion-runtime__value mouse-companion-runtime__value--path">-</div>
        </div>
      </div>
    </div>
  `;

  const enabledToggle = byId('mc_enabled');
  if (enabledToggle) {
    enabledToggle.addEventListener('change', syncEnabledText);
  }
  const testToggle = byId('mc_use_test_profile');
  if (testToggle) {
    testToggle.addEventListener('change', syncTestFieldState);
  }

  mount.dataset.mfxMouseCompanionMounted = '1';
  return mount;
}

function readFromDom() {
  return normalizeState({
    enabled: readChecked('mc_enabled'),
    model_path: (byId('mc_model_path')?.value || '').trim(),
    action_library_path: (byId('mc_action_library_path')?.value || '').trim(),
    appearance_profile_path: (byId('mc_appearance_profile_path')?.value || '').trim(),
    size_px: readNumber('mc_size_px', DEFAULT_STATE.size_px),
    offset_x: readNumber('mc_offset_x', DEFAULT_STATE.offset_x),
    offset_y: readNumber('mc_offset_y', DEFAULT_STATE.offset_y),
    press_lift_px: readNumber('mc_press_lift_px', DEFAULT_STATE.press_lift_px),
    smoothing_percent: readNumber('mc_smoothing_percent', DEFAULT_STATE.smoothing_percent),
    follow_threshold_px: readNumber('mc_follow_threshold_px', DEFAULT_STATE.follow_threshold_px),
    release_hold_ms: readNumber('mc_release_hold_ms', DEFAULT_STATE.release_hold_ms),
    use_test_profile: readChecked('mc_use_test_profile'),
    test_press_lift_px: readNumber('mc_test_press_lift_px', DEFAULT_STATE.test_press_lift_px),
    test_smoothing_percent: readNumber('mc_test_smoothing_percent', DEFAULT_STATE.test_smoothing_percent),
  });
}

function writeToDom(state, schema) {
  const enabled = byId('mc_enabled');
  if (enabled) {
    enabled.checked = !!state.enabled;
  }
  const modelPath = byId('mc_model_path');
  if (modelPath) {
    modelPath.value = state.model_path || DEFAULT_STATE.model_path;
  }
  const actionLibraryPath = byId('mc_action_library_path');
  if (actionLibraryPath) {
    actionLibraryPath.value = state.action_library_path || DEFAULT_STATE.action_library_path;
  }
  const appearanceProfilePath = byId('mc_appearance_profile_path');
  if (appearanceProfilePath) {
    appearanceProfilePath.value =
      state.appearance_profile_path || DEFAULT_STATE.appearance_profile_path;
  }

  const size = byId('mc_size_px');
  if (size) {
    size.value = `${state.size_px}`;
  }
  const offsetX = byId('mc_offset_x');
  if (offsetX) {
    offsetX.value = `${state.offset_x}`;
  }
  const offsetY = byId('mc_offset_y');
  if (offsetY) {
    offsetY.value = `${state.offset_y}`;
  }
  const pressLift = byId('mc_press_lift_px');
  if (pressLift) {
    pressLift.value = `${state.press_lift_px}`;
  }
  const smoothing = byId('mc_smoothing_percent');
  if (smoothing) {
    smoothing.value = `${state.smoothing_percent}`;
  }
  const followThreshold = byId('mc_follow_threshold_px');
  if (followThreshold) {
    followThreshold.value = `${state.follow_threshold_px}`;
  }
  const releaseHold = byId('mc_release_hold_ms');
  if (releaseHold) {
    releaseHold.value = `${state.release_hold_ms}`;
  }

  const useTestProfile = byId('mc_use_test_profile');
  if (useTestProfile) {
    useTestProfile.checked = !!state.use_test_profile;
  }
  const testPressLift = byId('mc_test_press_lift_px');
  if (testPressLift) {
    testPressLift.value = `${state.test_press_lift_px}`;
  }
  const testSmoothing = byId('mc_test_smoothing_percent');
  if (testSmoothing) {
    testSmoothing.value = `${state.test_smoothing_percent}`;
  }

  applyRange('mc_size_px', schema.size_px_range);
  applyRange('mc_offset_x', schema.offset_range);
  applyRange('mc_offset_y', schema.offset_range);
  applyRange('mc_press_lift_px', schema.press_lift_px_range);
  applyRange('mc_smoothing_percent', schema.smoothing_percent_range);
  applyRange('mc_follow_threshold_px', schema.follow_threshold_px_range);
  applyRange('mc_release_hold_ms', schema.release_hold_ms_range);
  applyRange('mc_test_press_lift_px', schema.test_press_lift_px_range);
  applyRange('mc_test_smoothing_percent', schema.test_smoothing_percent_range);

  syncEnabledText();
  syncTestFieldState();
}

function render(payload) {
  latestSchema = normalizeSchema(payload?.schema?.mouse_companion || payload?.schema || {});
  latestState = normalizeState(payload?.state?.mouse_companion || payload?.state || {});
  latestRuntimeState = normalizeRuntimeState(payload?.state?.mouse_companion_runtime || {});
  const mount = mountIfNeeded();
  if (!mount) {
    return;
  }
  writeToDom(latestState, latestSchema);
  writeRuntimeToDom(latestRuntimeState);
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
