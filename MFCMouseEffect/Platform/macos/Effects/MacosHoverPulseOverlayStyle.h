#pragma once

#include <string>

namespace mousefx::macos_hover_pulse {

#if defined(__APPLE__)
std::string NormalizeHoverType(const std::string& effectType);
#endif

} // namespace mousefx::macos_hover_pulse
