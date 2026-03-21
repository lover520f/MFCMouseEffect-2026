#pragma once

#include <array>

#include <gdiplus.h>

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderMotion.h"

namespace mousefx::windows {

struct Win32MouseCompanionPlaceholderExpression {
    Gdiplus::RectF leftPupilRect{};
    Gdiplus::RectF rightPupilRect{};
    std::array<Gdiplus::PointF, 2> leftBrow{};
    std::array<Gdiplus::PointF, 2> rightBrow{};
    Gdiplus::Color pupilColor{};
    Gdiplus::Color browColor{};
    float mouthStartDeg{10.0f};
    float mouthSweepDeg{160.0f};
    float mouthStrokeWidth{1.6f};
};

Win32MouseCompanionPlaceholderExpression BuildWin32MouseCompanionPlaceholderExpression(
    const Gdiplus::RectF& leftEyeRect,
    const Gdiplus::RectF& rightEyeRect,
    const Gdiplus::RectF& mouthRect,
    const Gdiplus::RectF& headRect,
    float bodyLeanPx,
    const Win32MouseCompanionPlaceholderMotion& motion,
    const Gdiplus::Color& eyeColor,
    const Gdiplus::Color& mouthColor);

} // namespace mousefx::windows
