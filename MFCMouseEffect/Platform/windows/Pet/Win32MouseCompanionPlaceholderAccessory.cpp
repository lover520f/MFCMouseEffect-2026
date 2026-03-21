#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderAccessory.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace mousefx::windows {
namespace {

std::string NormalizeAscii(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

bool HasToken(const std::vector<std::string>& values, const char* token) {
    if (!token) {
        return false;
    }
    const std::string needle = NormalizeAscii(token);
    for (const auto& value : values) {
        const std::string normalized = NormalizeAscii(value);
        if (normalized.find(needle) != std::string::npos) {
            return true;
        }
    }
    return false;
}

float ClampUnit(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

Gdiplus::Color BlendColor(
    const Gdiplus::Color& base,
    const Gdiplus::Color& target,
    float t) {
    const float clamped = ClampUnit(t);
    const auto lerp = [clamped](BYTE a, BYTE b) -> BYTE {
        return static_cast<BYTE>(std::lround(
            static_cast<double>(a) + (static_cast<double>(b) - static_cast<double>(a)) * clamped));
    };
    return Gdiplus::Color(
        lerp(base.GetA(), target.GetA()),
        lerp(base.GetR(), target.GetR()),
        lerp(base.GetG(), target.GetG()),
        lerp(base.GetB(), target.GetB()));
}

} // namespace

Win32MouseCompanionPlaceholderAccessory BuildWin32MouseCompanionPlaceholderAccessory(
    const std::vector<std::string>& enabledAccessoryIds,
    const Gdiplus::RectF& headRect,
    float bodyLeanPx,
    float facingSign,
    const Gdiplus::Color& accent,
    const Gdiplus::Color& bodyStroke) {
    Win32MouseCompanionPlaceholderAccessory accessory{};
    accessory.visible = !enabledAccessoryIds.empty();
    if (!accessory.visible) {
        return accessory;
    }

    accessory.starVisible = HasToken(enabledAccessoryIds, "star") || HasToken(enabledAccessoryIds, "spark");
    accessory.anchorRect = Gdiplus::RectF(
        headRect.X + headRect.Width * (facingSign < 0.0f ? 0.18f : 0.62f) + bodyLeanPx * 0.18f,
        headRect.Y - headRect.Height * 0.14f,
        headRect.Width * 0.18f,
        headRect.Height * 0.18f);
    accessory.gemRect = Gdiplus::RectF(
        accessory.anchorRect.X + accessory.anchorRect.Width * 0.08f,
        accessory.anchorRect.Y + accessory.anchorRect.Height * 0.08f,
        accessory.anchorRect.Width * 0.84f,
        accessory.anchorRect.Height * 0.84f);
    accessory.fill = BlendColor(accent, Gdiplus::Color(255, 255, 241, 178), 0.36f);
    accessory.stroke = bodyStroke;

    const float cx = accessory.anchorRect.X + accessory.anchorRect.Width * 0.5f;
    const float cy = accessory.anchorRect.Y + accessory.anchorRect.Height * 0.5f;
    const float outer = accessory.anchorRect.Width * 0.54f;
    const float inner = outer * 0.46f;
    for (size_t index = 0; index < accessory.starPoints.size(); ++index) {
        const double angle = -1.57079632679 + static_cast<double>(index) * 1.25663706144;
        const float radius = (index % 2 == 0) ? outer : inner;
        accessory.starPoints[index] = Gdiplus::PointF(
            cx + static_cast<float>(std::cos(angle) * radius),
            cy + static_cast<float>(std::sin(angle) * radius));
    }
    return accessory;
}

} // namespace mousefx::windows
