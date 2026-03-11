#pragma once

namespace mousefx::config_json::keys::wasm {

inline constexpr const char kEnabled[] = "enabled";
inline constexpr const char kFallbackToBuiltinClick[] = "fallback_to_builtin_click";
inline constexpr const char kManifestPath[] = "manifest_path";
inline constexpr const char kManifestPathClick[] = "manifest_path_click";
inline constexpr const char kManifestPathTrail[] = "manifest_path_trail";
inline constexpr const char kManifestPathScroll[] = "manifest_path_scroll";
inline constexpr const char kManifestPathHold[] = "manifest_path_hold";
inline constexpr const char kManifestPathHover[] = "manifest_path_hover";
inline constexpr const char kCatalogRootPath[] = "catalog_root_path";
inline constexpr const char kOutputBufferBytes[] = "output_buffer_bytes";
inline constexpr const char kMaxCommands[] = "max_commands";
inline constexpr const char kMaxExecutionMs[] = "max_execution_ms";

} // namespace mousefx::config_json::keys::wasm
