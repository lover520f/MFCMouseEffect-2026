export const MOUSE_COMPANION_CHECKBOX_FIELDS = [
  { key: 'enabled', id: 'mc_enabled' },
  { key: 'face_pointer_enabled', id: 'mc_face_pointer_enabled' },
  { key: 'use_test_profile', id: 'mc_use_test_profile' },
];

export const MOUSE_COMPANION_TEXT_FIELDS = [
  { key: 'model_path', id: 'mc_model_path' },
  { key: 'action_library_path', id: 'mc_action_library_path' },
  { key: 'appearance_profile_path', id: 'mc_appearance_profile_path' },
];

export const MOUSE_COMPANION_NUMBER_FIELDS = [
  { key: 'size_px', id: 'mc_size_px' },
  { key: 'offset_x', id: 'mc_offset_x' },
  { key: 'offset_y', id: 'mc_offset_y' },
  { key: 'absolute_x', id: 'mc_absolute_x' },
  { key: 'absolute_y', id: 'mc_absolute_y' },
  { key: 'press_lift_px', id: 'mc_press_lift_px' },
  { key: 'smoothing_percent', id: 'mc_smoothing_percent' },
  { key: 'follow_threshold_px', id: 'mc_follow_threshold_px' },
  { key: 'release_hold_ms', id: 'mc_release_hold_ms' },
  { key: 'click_streak_break_ms', id: 'mc_click_streak_break_ms' },
  { key: 'test_press_lift_px', id: 'mc_test_press_lift_px' },
  { key: 'test_smoothing_percent', id: 'mc_test_smoothing_percent' },
  { key: 'test_click_streak_break_ms', id: 'mc_test_click_streak_break_ms' },
];

export const MOUSE_COMPANION_FLOAT_FIELDS = [
  { key: 'head_tint_per_click', id: 'mc_head_tint_per_click' },
  { key: 'head_tint_max', id: 'mc_head_tint_max' },
  { key: 'head_tint_decay_per_second', id: 'mc_head_tint_decay_per_second' },
  { key: 'test_head_tint_per_click', id: 'mc_test_head_tint_per_click' },
  { key: 'test_head_tint_max', id: 'mc_test_head_tint_max' },
  { key: 'test_head_tint_decay_per_second', id: 'mc_test_head_tint_decay_per_second' },
];

export const MOUSE_COMPANION_RANGE_BINDINGS = [
  { id: 'mc_size_px', schemaKey: 'size_px_range' },
  { id: 'mc_offset_x', schemaKey: 'offset_range' },
  { id: 'mc_offset_y', schemaKey: 'offset_range' },
  { id: 'mc_absolute_x', schemaKey: 'absolute_range' },
  { id: 'mc_absolute_y', schemaKey: 'absolute_range' },
  { id: 'mc_press_lift_px', schemaKey: 'press_lift_px_range' },
  { id: 'mc_smoothing_percent', schemaKey: 'smoothing_percent_range' },
  { id: 'mc_follow_threshold_px', schemaKey: 'follow_threshold_px_range' },
  { id: 'mc_release_hold_ms', schemaKey: 'release_hold_ms_range' },
  { id: 'mc_click_streak_break_ms', schemaKey: 'click_streak_break_ms_range' },
  { id: 'mc_head_tint_per_click', schemaKey: 'head_tint_per_click_range' },
  { id: 'mc_head_tint_max', schemaKey: 'head_tint_max_range' },
  { id: 'mc_head_tint_decay_per_second', schemaKey: 'head_tint_decay_per_second_range' },
  { id: 'mc_test_press_lift_px', schemaKey: 'test_press_lift_px_range' },
  { id: 'mc_test_smoothing_percent', schemaKey: 'test_smoothing_percent_range' },
  { id: 'mc_test_click_streak_break_ms', schemaKey: 'test_click_streak_break_ms_range' },
  { id: 'mc_test_head_tint_per_click', schemaKey: 'test_head_tint_per_click_range' },
  { id: 'mc_test_head_tint_max', schemaKey: 'test_head_tint_max_range' },
  { id: 'mc_test_head_tint_decay_per_second', schemaKey: 'test_head_tint_decay_per_second_range' },
];
