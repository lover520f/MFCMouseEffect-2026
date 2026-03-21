#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderAdornment.h"

#include <algorithm>
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

} // namespace

Win32MouseCompanionPlaceholderAdornment BuildWin32MouseCompanionPlaceholderAdornment(
    const Gdiplus::RectF& bodyRect,
    const Gdiplus::RectF& headRect,
    float bodyLeanPx,
    float facingSign,
    const Win32MouseCompanionPlaceholderMotion& motion,
    const Gdiplus::Color& accent,
    const Gdiplus::Color& bodyStroke,
    const Gdiplus::Color& headFill) {
    Win32MouseCompanionPlaceholderAdornment adornment{};

    adornment.collarRect = Gdiplus::RectF(
        bodyRect.X + bodyRect.Width * 0.22f + bodyLeanPx * 0.08f,
        headRect.Y + headRect.Height * 0.86f,
        bodyRect.Width * 0.56f,
        bodyRect.Height * 0.10f);
    adornment.charmRect = Gdiplus::RectF(
        adornment.collarRect.X + adornment.collarRect.Width * 0.42f + facingSign * 1.5f,
        adornment.collarRect.Y + adornment.collarRect.Height * 0.52f + motion.chestBobPx * 0.5f,
        bodyRect.Width * 0.11f,
        bodyRect.Width * 0.11f);
    adornment.collarFill = BlendColor(accent, headFill, 0.32f);
    adornment.collarStroke = bodyStroke;
    adornment.charmFill = BlendColor(accent, Gdiplus::Color(255, 255, 236, 150), 0.45f);

    const float dustWeight =
        std::max(motion.reactive.dragTension, motion.reactive.scrollKick * 0.75f);
    adornment.dustAlpha = 42.0f + dustWeight * 68.0f;
    adornment.dustFill = BlendColor(accent, headFill, 0.58f);
    const float baseY = bodyRect.Y + bodyRect.Height * 0.88f;
    const float dustShift = facingSign * (-4.0f - dustWeight * 6.0f);
    adornment.dustRects[0] = Gdiplus::RectF(
        bodyRect.X + bodyRect.Width * 0.08f + dustShift,
        baseY + motion.frontLegLiftPx * 0.12f,
        5.0f + dustWeight * 4.2f,
        3.4f + dustWeight * 2.1f);
    adornment.dustRects[1] = Gdiplus::RectF(
        bodyRect.X + bodyRect.Width * 0.20f + dustShift * 0.7f,
        baseY - 3.8f - motion.rearLegLiftPx * 0.10f,
        4.4f + dustWeight * 3.0f,
        3.0f + dustWeight * 1.5f);
    adornment.dustRects[2] = Gdiplus::RectF(
        bodyRect.X + bodyRect.Width * 0.34f + dustShift * 0.45f,
        baseY + 1.4f,
        3.6f + dustWeight * 2.2f,
        2.8f + dustWeight * 1.3f);
    return adornment;
}

} // namespace mousefx::windows
