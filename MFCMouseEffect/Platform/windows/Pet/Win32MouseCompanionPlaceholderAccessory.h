#pragma once

#include <array>
#include <string>
#include <vector>

#include <gdiplus.h>

namespace mousefx::windows {

struct Win32MouseCompanionPlaceholderAccessory {
    bool visible{false};
    bool starVisible{false};
    Gdiplus::RectF anchorRect{};
    Gdiplus::RectF gemRect{};
    std::array<Gdiplus::PointF, 5> starPoints{};
    Gdiplus::Color fill{};
    Gdiplus::Color stroke{};
};

Win32MouseCompanionPlaceholderAccessory BuildWin32MouseCompanionPlaceholderAccessory(
    const std::vector<std::string>& enabledAccessoryIds,
    const Gdiplus::RectF& headRect,
    float bodyLeanPx,
    float facingSign,
    const Gdiplus::Color& accent,
    const Gdiplus::Color& bodyStroke);

} // namespace mousefx::windows
