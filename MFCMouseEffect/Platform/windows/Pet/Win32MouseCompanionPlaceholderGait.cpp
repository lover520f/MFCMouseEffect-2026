#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderGait.h"

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

Gdiplus::PointF RectAnchor(const Gdiplus::RectF& rect, float nx, float ny) {
    return Gdiplus::PointF(rect.X + rect.Width * nx, rect.Y + rect.Height * ny);
}

} // namespace

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
    const Gdiplus::Color& bodyStroke) {
    Win32MouseCompanionPlaceholderGait gait{};

    gait.neckBridgeRect = Gdiplus::RectF(
        headRect.X + headRect.Width * 0.22f,
        headRect.Y + headRect.Height * 0.76f,
        bodyRect.Width * 0.34f,
        bodyRect.Height * 0.18f);
    gait.backStripeRect = Gdiplus::RectF(
        bodyRect.X + bodyRect.Width * 0.18f,
        bodyRect.Y + bodyRect.Height * 0.14f - motion.bodyBobPx * 0.08f,
        bodyRect.Width * 0.52f,
        bodyRect.Height * 0.14f);
    gait.bellyStripeRect = Gdiplus::RectF(
        bodyRect.X + bodyRect.Width * 0.24f,
        bodyRect.Y + bodyRect.Height * 0.58f,
        bodyRect.Width * 0.36f,
        bodyRect.Height * 0.11f);
    gait.hipPatchRect = Gdiplus::RectF(
        bodyRect.X + bodyRect.Width * 0.62f,
        bodyRect.Y + bodyRect.Height * 0.44f,
        bodyRect.Width * 0.15f,
        bodyRect.Height * 0.18f);
    gait.neckBridgeFill = BlendColor(headFill, bodyFill, 0.42f);
    gait.stripeFill = BlendColor(bodyFill, bodyStroke, frontSideIsLeft ? 0.18f : 0.12f);
    gait.stripeRearFill = BlendColor(bodyFill, bodyStroke, frontSideIsLeft ? 0.12f : 0.18f);
    gait.bridgeColor = BlendColor(bodyStroke, headFill, 0.16f);
    gait.bridgeWidth = 1.4f + motion.reactive.dragTension * 0.25f;

    gait.leftForeBridge = {{
        RectAnchor(bodyRect, 0.22f, 0.58f),
        RectAnchor(leftHandRect, 0.54f, 0.06f),
    }};
    gait.rightForeBridge = {{
        RectAnchor(bodyRect, 0.74f, 0.58f),
        RectAnchor(rightHandRect, 0.46f, 0.06f),
    }};
    gait.leftRearBridge = {{
        RectAnchor(bodyRect, 0.30f, 0.84f),
        RectAnchor(leftLegRect, 0.52f, 0.08f),
    }};
    gait.rightRearBridge = {{
        RectAnchor(bodyRect, 0.66f, 0.84f),
        RectAnchor(rightLegRect, 0.48f, 0.08f),
    }};
    return gait;
}

} // namespace mousefx::windows
