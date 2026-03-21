#pragma once

#include <array>

#include <gdiplus.h>

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderMotion.h"

namespace mousefx::windows {

struct Win32MouseCompanionPlaceholderGait {
    Gdiplus::RectF neckBridgeRect{};
    Gdiplus::RectF backStripeRect{};
    Gdiplus::RectF bellyStripeRect{};
    Gdiplus::RectF hipPatchRect{};
    Gdiplus::Color neckBridgeFill{};
    Gdiplus::Color stripeFill{};
    Gdiplus::Color stripeRearFill{};
    std::array<Gdiplus::PointF, 2> leftForeBridge{};
    std::array<Gdiplus::PointF, 2> rightForeBridge{};
    std::array<Gdiplus::PointF, 2> leftRearBridge{};
    std::array<Gdiplus::PointF, 2> rightRearBridge{};
    Gdiplus::Color bridgeColor{};
    float bridgeWidth{1.0f};
};

Win32MouseCompanionPlaceholderGait BuildWin32MouseCompanionPlaceholderGait(
    const Gdiplus::RectF& bodyRect,
    const Gdiplus::RectF& headRect,
    const Gdiplus::RectF& leftHandRect,
    const Gdiplus::RectF& rightHandRect,
    const Gdiplus::RectF& leftLegRect,
    const Gdiplus::RectF& rightLegRect,
    bool frontSideIsLeft,
    const Win32MouseCompanionPlaceholderMotion& motion,
    const Gdiplus::Color& bodyFill,
    const Gdiplus::Color& headFill,
    const Gdiplus::Color& bodyStroke);

} // namespace mousefx::windows
