#pragma once

namespace mousefx::config_json::keys::input {

inline constexpr const char kEnabled[] = "enabled";
inline constexpr const char kKeyboardEnabled[] = "keyboard_enabled";
inline constexpr const char kRenderMode[] = "render_mode";
inline constexpr const char kWasmFallbackToNative[] = "wasm_fallback_to_native";
inline constexpr const char kWasmManifestPath[] = "wasm_manifest_path";
inline constexpr const char kPositionMode[] = "position_mode";
inline constexpr const char kOffsetX[] = "offset_x";
inline constexpr const char kOffsetY[] = "offset_y";
inline constexpr const char kAbsoluteX[] = "absolute_x";
inline constexpr const char kAbsoluteY[] = "absolute_y";
inline constexpr const char kTargetMonitor[] = "target_monitor";
inline constexpr const char kKeyDisplayMode[] = "key_display_mode";
inline constexpr const char kKeyLabelLayoutMode[] = "key_label_layout_mode";
inline constexpr const char kSizePx[] = "size_px";
inline constexpr const char kDurationMs[] = "duration_ms";
inline constexpr const char kPerMonitorOverrides[] = "per_monitor_overrides";
inline constexpr const char kCursorDecoration[] = "cursor_decoration";

namespace cursor_decoration {
inline constexpr const char kEnabled[] = "enabled";
inline constexpr const char kPluginId[] = "plugin_id";
inline constexpr const char kColorHex[] = "color_hex";
inline constexpr const char kSizePx[] = "size_px";
inline constexpr const char kAlphaPercent[] = "alpha_percent";
} // namespace cursor_decoration

} // namespace mousefx::config_json::keys::input
