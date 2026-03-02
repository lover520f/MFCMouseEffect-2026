const DEFAULT_GENERAL_STATE = Object.freeze({
  ui_language: '',
  theme: '',
  theme_catalog_root_path: '',
  hold_follow_mode: 'smooth',
  hold_presenter_backend: 'auto',
});

export function normalizeGeneralState(input) {
  const value = input && typeof input === 'object' ? input : {};
  return {
    ui_language: value.ui_language || DEFAULT_GENERAL_STATE.ui_language,
    theme: value.theme || DEFAULT_GENERAL_STATE.theme,
    theme_catalog_root_path: value.theme_catalog_root_path || DEFAULT_GENERAL_STATE.theme_catalog_root_path,
    hold_follow_mode: value.hold_follow_mode || DEFAULT_GENERAL_STATE.hold_follow_mode,
    hold_presenter_backend: value.hold_presenter_backend || DEFAULT_GENERAL_STATE.hold_presenter_backend,
  };
}

