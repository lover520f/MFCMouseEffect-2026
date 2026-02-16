#pragma once

namespace mousefx::config_json::keys::effect {

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

} // namespace mousefx::config_json::keys::effect
