#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderPalette.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace mousefx::windows {
namespace {

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

std::string NormalizeAscii(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

Gdiplus::Color ResolveVariantBaseColor(const std::string& skinVariantId) {
    const std::string normalized = NormalizeAscii(skinVariantId);
    if (normalized == "strawberry" || normalized == "pink") {
        return Gdiplus::Color(244, 255, 236, 240);
    }
    if (normalized == "midnight" || normalized == "navy") {
        return Gdiplus::Color(244, 228, 236, 250);
    }
    if (normalized == "mint" || normalized == "casual") {
        return Gdiplus::Color(244, 236, 255, 244);
    }
    return Gdiplus::Color(244, 252, 253, 255);
}

} // namespace

Win32MouseCompanionPlaceholderPalette BuildWin32MouseCompanionPlaceholderPalette(
    const std::string& skinVariantId,
    float headTintAmount,
    bool scrollActive,
    float signedScrollIntensity) {
    Win32MouseCompanionPlaceholderPalette palette{};

    const float tint = ClampUnit(headTintAmount);
    const Gdiplus::Color variantBase = ResolveVariantBaseColor(skinVariantId);
    palette.bodyStroke = Gdiplus::Color(220, 89, 109, 141);
    palette.bodyFill = BlendColor(
        BlendColor(Gdiplus::Color(236, 245, 247, 255), variantBase, 0.65f),
        Gdiplus::Color(244, 255, 227, 227),
        tint * 0.55f);
    palette.headFill = BlendColor(
        variantBase,
        Gdiplus::Color(252, 255, 214, 214),
        tint * 0.75f);
    palette.headFillRear = BlendColor(palette.headFill, palette.bodyStroke, 0.18f);
    palette.earInner = BlendColor(
        Gdiplus::Color(188, 255, 205, 214),
        Gdiplus::Color(212, 255, 150, 158),
        tint);
    palette.earInnerRear = BlendColor(palette.earInner, palette.bodyStroke, 0.22f);
    palette.accent = scrollActive && signedScrollIntensity < 0.0f
        ? Gdiplus::Color(220, 125, 154, 255)
        : Gdiplus::Color(220, 116, 204, 178);
    palette.accentGlow = BlendColor(palette.accent, palette.headFill, 0.25f);
    palette.eyeColor = Gdiplus::Color(220, 78, 84, 111);
    palette.mouthColor = Gdiplus::Color(190, 98, 107, 140);
    palette.blushColor = BlendColor(
        Gdiplus::Color(72, 255, 184, 196),
        Gdiplus::Color(128, 255, 112, 122),
        tint);
    palette.chestFill = BlendColor(palette.headFill, Gdiplus::Color(180, 255, 250, 244), 0.52f);
    palette.tailFill = BlendColor(palette.bodyFill, palette.headFill, 0.35f);
    palette.tailTipFill = BlendColor(palette.tailFill, palette.accent, 0.16f);
    palette.pawPadFill = BlendColor(palette.blushColor, palette.headFill, 0.22f);
    palette.shadowColor = Gdiplus::Color(64, 68, 86, 118);
    return palette;
}

} // namespace mousefx::windows
