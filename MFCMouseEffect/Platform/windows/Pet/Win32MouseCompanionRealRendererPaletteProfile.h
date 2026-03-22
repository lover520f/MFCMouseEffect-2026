#pragma once

#include <gdiplus.h>

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererStyleProfile.h"

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererPaletteProfile final {
    Gdiplus::Color glowColor{};
    Gdiplus::Color baseBodyFill{};
    Gdiplus::Color bodyStroke{};
    Gdiplus::Color headFill{};
    Gdiplus::Color headFillRear{};
    Gdiplus::Color earFill{};
    Gdiplus::Color earFillRear{};
    Gdiplus::Color earInner{};
    Gdiplus::Color earInnerRear{};
    Gdiplus::Color eyeFill{};
    Gdiplus::Color mouthFill{};
    Gdiplus::Color blushRgb{};
    Gdiplus::Color accentFill{};
    Gdiplus::Color pedestalFill{};
    Gdiplus::Color badgeReadyFill{};
    Gdiplus::Color badgePendingFill{};
    Gdiplus::Color accessoryFill{};
    Gdiplus::Color accessoryStroke{};
};

Win32MouseCompanionRealRendererPaletteProfile BuildWin32MouseCompanionRealRendererPaletteProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererStyleProfile& style);

} // namespace mousefx::windows
