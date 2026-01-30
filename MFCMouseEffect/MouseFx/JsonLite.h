#pragma once

#include <string>

namespace mousefx {

// Minimal JSON-like parser: extracts a string value for a given key.
// Intended for small IPC/tray commands; not a full JSON parser.
std::string ExtractJsonStringValue(const std::string& json, const std::string& key);

} // namespace mousefx

