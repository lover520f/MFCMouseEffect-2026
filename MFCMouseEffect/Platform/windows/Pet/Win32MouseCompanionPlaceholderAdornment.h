#pragma once

#include <array>

#include <gdiplus.h>

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderMotion.h"

namespace mousefx::windows {

struct Win32MouseCompanionPlaceholderAdornment {
    Gdiplus::RectF collarRect{};
    Gdiplus::RectF charmRect{};
    Gdiplus::Color collarFill{};
    Gdiplus::Color collarStroke{};
    Gdiplus::Color charmFill{};
    std::array<Gdiplus::RectF, 3> dustRects{};
    Gdiplus::Color dustFill{};
    float dustAlpha{0.0f};
};

Win32MouseCompanionPlaceholderAdornment BuildWin32MouseCompanionPlaceholderAdornment(
    const Gdiplus::RectF& bodyRect,
    const Gdiplus::RectF& headRect,
    float bodyLeanPx,
    float facingSign,
    const Win32MouseCompanionPlaceholderMotion& motion,
    const Gdiplus::Color& accent,
    const Gdiplus::Color& bodyStroke,
    const Gdiplus::Color& headFill);

} // namespace mousefx::windows
