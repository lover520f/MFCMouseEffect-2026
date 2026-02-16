#pragma once

namespace mousefx::config_json::keys {

inline constexpr const char kDefaultEffect[] = "default_effect";
inline constexpr const char kTheme[] = "theme";
inline constexpr const char kUiLanguage[] = "ui_language";
inline constexpr const char kHoldFollowMode[] = "hold_follow_mode";
inline constexpr const char kTrailStyle[] = "trail_style";
inline constexpr const char kActiveEffects[] = "active_effects";
inline constexpr const char kInputIndicator[] = "input_indicator";
inline constexpr const char kMouseIndicator[] = "mouse_indicator";
inline constexpr const char kTrailParams[] = "trail_params";
inline constexpr const char kTrailProfiles[] = "trail_profiles";
inline constexpr const char kEffects[] = "effects";

namespace active {
inline constexpr const char kClick[] = "click";
inline constexpr const char kTrail[] = "trail";
inline constexpr const char kScroll[] = "scroll";
inline constexpr const char kHover[] = "hover";
inline constexpr const char kHold[] = "hold";
} // namespace active

namespace input {
inline constexpr const char kEnabled[] = "enabled";
inline constexpr const char kKeyboardEnabled[] = "keyboard_enabled";
inline constexpr const char kPositionMode[] = "position_mode";
inline constexpr const char kOffsetX[] = "offset_x";
inline constexpr const char kOffsetY[] = "offset_y";
inline constexpr const char kAbsoluteX[] = "absolute_x";
inline constexpr const char kAbsoluteY[] = "absolute_y";
inline constexpr const char kTargetMonitor[] = "target_monitor";
inline constexpr const char kKeyDisplayMode[] = "key_display_mode";
inline constexpr const char kSizePx[] = "size_px";
inline constexpr const char kDurationMs[] = "duration_ms";
inline constexpr const char kPerMonitorOverrides[] = "per_monitor_overrides";
} // namespace input

namespace profile {
inline constexpr const char kLine[] = "line";
inline constexpr const char kStreamer[] = "streamer";
inline constexpr const char kElectric[] = "electric";
inline constexpr const char kMeteor[] = "meteor";
inline constexpr const char kTubes[] = "tubes";
inline constexpr const char kDurationMs[] = "duration_ms";
inline constexpr const char kMaxPoints[] = "max_points";
} // namespace profile

namespace trail_params {
inline constexpr const char kStreamer[] = "streamer";
inline constexpr const char kElectric[] = "electric";
inline constexpr const char kMeteor[] = "meteor";
inline constexpr const char kIdleFadeStartMs[] = "idle_fade_start_ms";
inline constexpr const char kIdleFadeEndMs[] = "idle_fade_end_ms";

namespace streamer {
inline constexpr const char kGlowWidthScale[] = "glow_width_scale";
inline constexpr const char kCoreWidthScale[] = "core_width_scale";
inline constexpr const char kHeadPower[] = "head_power";
} // namespace streamer

namespace electric {
inline constexpr const char kAmplitudeScale[] = "amplitude_scale";
inline constexpr const char kForkChance[] = "fork_chance";
} // namespace electric

namespace meteor {
inline constexpr const char kSparkRateScale[] = "spark_rate_scale";
inline constexpr const char kSparkSpeedScale[] = "spark_speed_scale";
} // namespace meteor
} // namespace trail_params

namespace effect {
inline constexpr const char kRipple[] = "ripple";
inline constexpr const char kTrail[] = "trail";
inline constexpr const char kIconStar[] = "icon_star";
inline constexpr const char kTextClick[] = "text_click";
inline constexpr const char kDurationMs[] = "duration_ms";
inline constexpr const char kStartRadius[] = "start_radius";
inline constexpr const char kEndRadius[] = "end_radius";
inline constexpr const char kStrokeWidth[] = "stroke_width";
inline constexpr const char kWindowSize[] = "window_size";
inline constexpr const char kLineWidth[] = "line_width";
inline constexpr const char kColor[] = "color";
inline constexpr const char kFloatDistance[] = "float_distance";
inline constexpr const char kFontSize[] = "font_size";
inline constexpr const char kFontFamily[] = "font_family";
inline constexpr const char kTexts[] = "texts";
inline constexpr const char kColors[] = "colors";

namespace click {
inline constexpr const char kLeft[] = "left_click";
inline constexpr const char kRight[] = "right_click";
inline constexpr const char kMiddle[] = "middle_click";
inline constexpr const char kFill[] = "fill";
inline constexpr const char kStroke[] = "stroke";
inline constexpr const char kGlow[] = "glow";
} // namespace click
} // namespace effect

} // namespace mousefx::config_json::keys
