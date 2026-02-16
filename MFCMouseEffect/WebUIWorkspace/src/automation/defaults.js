export const DEFAULT_GESTURE_TRIGGER_BUTTON = 'right';
export const DEFAULT_GESTURE_MIN_DISTANCE = 80;
export const DEFAULT_GESTURE_SAMPLE_STEP = 10;
export const DEFAULT_GESTURE_MAX_DIRECTIONS = 4;

export function createDefaultAutomationState() {
  return {
    enabled: false,
    mouse_mappings: [],
    gesture: {
      enabled: false,
      trigger_button: DEFAULT_GESTURE_TRIGGER_BUTTON,
      min_stroke_distance_px: DEFAULT_GESTURE_MIN_DISTANCE,
      sample_step_px: DEFAULT_GESTURE_SAMPLE_STEP,
      max_directions: DEFAULT_GESTURE_MAX_DIRECTIONS,
      mappings: [],
    },
  };
}
