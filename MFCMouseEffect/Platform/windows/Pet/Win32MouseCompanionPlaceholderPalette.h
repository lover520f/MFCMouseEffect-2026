#pragma once

#include <string>

#include <gdiplus.h>

namespace mousefx::windows {

struct Win32MouseCompanionPlaceholderPalette {
    Gdiplus::Color bodyFill{};
    Gdiplus::Color bodyStroke{};
    Gdiplus::Color headFill{};
    Gdiplus::Color headFillRear{};
    Gdiplus::Color earInner{};
    Gdiplus::Color earInnerRear{};
    Gdiplus::Color accent{};
    Gdiplus::Color accentGlow{};
    Gdiplus::Color eyeColor{};
    Gdiplus::Color mouthColor{};
    Gdiplus::Color blushColor{};
    Gdiplus::Color chestFill{};
    Gdiplus::Color tailFill{};
    Gdiplus::Color tailTipFill{};
    Gdiplus::Color pawPadFill{};
    Gdiplus::Color shadowColor{};
};

Win32MouseCompanionPlaceholderPalette BuildWin32MouseCompanionPlaceholderPalette(
    const std::string& skinVariantId,
    float headTintAmount,
    bool scrollActive,
    float signedScrollIntensity);

} // namespace mousefx::windows
