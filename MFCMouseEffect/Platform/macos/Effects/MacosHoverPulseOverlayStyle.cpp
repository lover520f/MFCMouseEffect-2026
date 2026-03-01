#include "pch.h"

#include "Platform/macos/Effects/MacosHoverPulseOverlayStyle.h"
#include "MouseFx/Utils/StringUtils.h"

namespace mousefx::macos_hover_pulse {

#if defined(__APPLE__)
namespace {

bool ContainsHoverToken(const std::string& value, const char* token) {
    return value.find(token) != std::string::npos;
}

} // namespace

std::string NormalizeHoverType(const std::string& effectType) {
    const std::string value = ToLowerAscii(effectType);
    if (value.empty() || value == "none") {
        return "glow";
    }
    if (ContainsHoverToken(value, "tube") ||
        ContainsHoverToken(value, "suspension") ||
        ContainsHoverToken(value, "helix")) {
        return "tubes";
    }
    if (ContainsHoverToken(value, "glow") || ContainsHoverToken(value, "breath")) {
        return "glow";
    }
    return "glow";
}

#endif

} // namespace mousefx::macos_hover_pulse
