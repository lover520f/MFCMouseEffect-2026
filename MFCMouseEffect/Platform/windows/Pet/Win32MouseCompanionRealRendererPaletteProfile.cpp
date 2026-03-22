#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPaletteProfile.h"

namespace mousefx::windows {

Win32MouseCompanionRealRendererPaletteProfile BuildWin32MouseCompanionRealRendererPaletteProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererStyleProfile& style) {
    const auto makeColor = [](BYTE a, BYTE r, BYTE g, BYTE b) {
        return Gdiplus::Color(a, r, g, b);
    };
    Win32MouseCompanionRealRendererPaletteProfile profile{};
    const auto semantics = BuildWin32MouseCompanionRealRendererAppearanceSemantics(runtime, style);
    const auto& theme = semantics.theme;
    profile.glowColor = theme.glowColor;
    profile.baseBodyFill = theme.baseBodyFill;
    profile.bodyStroke = theme.bodyStroke;
    profile.headFill = theme.headFill;
    profile.headFillRear = theme.headFillRear;
    profile.earFill = theme.earFill;
    profile.earFillRear = theme.earFillRear;
    profile.earInner = theme.earInner;
    profile.earInnerRear = theme.earInnerRear;
    profile.eyeFill = makeColor(255, 38, 44, 62);
    profile.mouthFill = makeColor(255, 106, 84, 114);
    profile.blushRgb = theme.blushRgb;
    profile.accentFill = theme.accentFill;
    profile.pedestalFill = theme.pedestalFill;
    profile.badgeReadyFill = makeColor(255, 111, 229, 178);
    profile.badgePendingFill = makeColor(255, 255, 189, 97);
    profile.accessoryFill = theme.accessoryFill;
    profile.accessoryStroke = theme.accessoryStroke;
    return profile;
}

} // namespace mousefx::windows
