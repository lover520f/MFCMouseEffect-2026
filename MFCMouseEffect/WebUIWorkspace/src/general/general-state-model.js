const DEFAULT_GENERAL_STATE = Object.freeze({
  ui_language: '',
  theme: '',
  theme_catalog_root_path: '',
  overlay_target_fps: 0,
  launch_at_startup: false,
  hold_follow_mode: 'smooth',
  hold_presenter_backend: 'auto',
});

const OVERLAY_TARGET_FPS_MIN = 0;
const OVERLAY_TARGET_FPS_MAX = 360;

function normalizeOverlayTargetFps(value) {
  const parsed = Number.parseInt(`${value ?? ''}`, 10);
  if (!Number.isFinite(parsed)) {
    return DEFAULT_GENERAL_STATE.overlay_target_fps;
  }
  return Math.max(OVERLAY_TARGET_FPS_MIN, Math.min(OVERLAY_TARGET_FPS_MAX, parsed));
}

export function normalizeGeneralState(input) {
  const value = input && typeof input === 'object' ? input : {};
  return {
    ui_language: value.ui_language || DEFAULT_GENERAL_STATE.ui_language,
    theme: value.theme || DEFAULT_GENERAL_STATE.theme,
    theme_catalog_root_path: value.theme_catalog_root_path || DEFAULT_GENERAL_STATE.theme_catalog_root_path,
    overlay_target_fps: normalizeOverlayTargetFps(value.overlay_target_fps),
    launch_at_startup: !!value.launch_at_startup,
    hold_follow_mode: value.hold_follow_mode || DEFAULT_GENERAL_STATE.hold_follow_mode,
    hold_presenter_backend: value.hold_presenter_backend || DEFAULT_GENERAL_STATE.hold_presenter_backend,
  };
}
