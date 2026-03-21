#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderExpression.h"

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

Win32MouseCompanionPlaceholderExpression BuildWin32MouseCompanionPlaceholderExpression(
    const Gdiplus::RectF& leftEyeRect,
    const Gdiplus::RectF& rightEyeRect,
    const Gdiplus::RectF& mouthRect,
    const Gdiplus::RectF& headRect,
    float bodyLeanPx,
    const Win32MouseCompanionPlaceholderMotion& motion,
    const Gdiplus::Color& eyeColor,
    const Gdiplus::Color& mouthColor) {
    Win32MouseCompanionPlaceholderExpression expression{};

    const float attention = motion.reactive.attentionFocus;
    const float scrollKick = motion.reactive.scrollKick;
    const float scrollDirection = motion.reactive.scrollDirection;
    const float pupilInsetX = 0.8f + attention * 0.6f;
    const float pupilInsetY = 0.9f + attention * 0.4f;
    const float pupilShiftX = scrollDirection * scrollKick * 1.2f + bodyLeanPx * 0.01f;
    const float pupilShiftY = -motion.cheekLiftPx * 0.06f;

    expression.leftPupilRect = Gdiplus::RectF(
        leftEyeRect.X + pupilInsetX + pupilShiftX,
        leftEyeRect.Y + pupilInsetY + pupilShiftY,
        std::max(1.6f, leftEyeRect.Width - pupilInsetX * 2.0f),
        std::max(1.2f, leftEyeRect.Height - pupilInsetY * 2.0f));
    expression.rightPupilRect = Gdiplus::RectF(
        rightEyeRect.X + pupilInsetX + pupilShiftX,
        rightEyeRect.Y + pupilInsetY + pupilShiftY,
        std::max(1.6f, rightEyeRect.Width - pupilInsetX * 2.0f),
        std::max(1.2f, rightEyeRect.Height - pupilInsetY * 2.0f));

    const float browY = headRect.Y + headRect.Height * 0.30f - attention * 1.2f;
    const float browTilt = scrollDirection * scrollKick * 1.8f + motion.reactive.clickImpact * 1.0f;
    const float browWidth = 6.6f + attention * 1.1f;
    expression.leftBrow = {{
        Gdiplus::PointF(leftEyeRect.X - 0.5f, browY - browTilt * 0.35f),
        Gdiplus::PointF(leftEyeRect.X - 0.5f + browWidth, browY + browTilt * 0.25f),
    }};
    expression.rightBrow = {{
        Gdiplus::PointF(rightEyeRect.X - 1.1f, browY + browTilt * 0.25f),
        Gdiplus::PointF(rightEyeRect.X - 1.1f + browWidth, browY - browTilt * 0.35f),
    }};

    expression.pupilColor = BlendColor(eyeColor, Gdiplus::Color(255, 24, 28, 38), 0.55f);
    expression.browColor = BlendColor(mouthColor, Gdiplus::Color(255, 46, 54, 70), 0.35f);
    expression.mouthStartDeg = 12.0f + scrollDirection * scrollKick * 8.0f;
    expression.mouthSweepDeg =
        156.0f - motion.reactive.mouthOpen * 18.0f + motion.reactive.holdCompression * 8.0f;
    expression.mouthStrokeWidth = 1.5f + attention * 0.35f;
    return expression;
}

} // namespace mousefx::windows
