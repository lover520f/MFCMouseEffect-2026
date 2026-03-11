#pragma once

namespace mousefx::config_json::keys::automation {

inline constexpr const char kEnabled[] = "enabled";
inline constexpr const char kMouseMappings[] = "mouse_mappings";
inline constexpr const char kGesture[] = "gesture";
inline constexpr const char kMappings[] = "mappings";
inline constexpr const char kTrigger[] = "trigger";
inline constexpr const char kAppScope[] = "app_scope";
inline constexpr const char kAppScopes[] = "app_scopes";
inline constexpr const char kModifiers[] = "modifiers";
inline constexpr const char kModifierMode[] = "mode";
inline constexpr const char kModifierPrimary[] = "primary";
inline constexpr const char kModifierShift[] = "shift";
inline constexpr const char kModifierAlt[] = "alt";
inline constexpr const char kGesturePattern[] = "gesture_pattern";
inline constexpr const char kGesturePatternMode[] = "mode";
inline constexpr const char kGestureCustomPoints[] = "custom_points";
inline constexpr const char kGestureCustomStrokes[] = "custom_strokes";
inline constexpr const char kGestureMatchThresholdPercent[] = "match_threshold_percent";
inline constexpr const char kPointX[] = "x";
inline constexpr const char kPointY[] = "y";
inline constexpr const char kKeys[] = "keys";
inline constexpr const char kTriggerButton[] = "trigger_button";
inline constexpr const char kMinStrokeDistancePx[] = "min_stroke_distance_px";
inline constexpr const char kSampleStepPx[] = "sample_step_px";
inline constexpr const char kMaxDirections[] = "max_directions";

} // namespace mousefx::config_json::keys::automation
