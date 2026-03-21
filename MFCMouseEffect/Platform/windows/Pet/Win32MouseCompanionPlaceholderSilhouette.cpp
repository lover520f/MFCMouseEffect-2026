#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderSilhouette.h"

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

Win32MouseCompanionPlaceholderSilhouette BuildWin32MouseCompanionPlaceholderSilhouette(
    const Gdiplus::RectF& bodyRect,
    const Gdiplus::RectF& headRect,
    const Gdiplus::RectF& tailRect,
    const Gdiplus::RectF& chestRect,
    bool frontSideIsLeft,
    float facingSign,
    float silhouetteNudgePx,
    float frontDepthBiasPx,
    float rearDepthBiasPx,
    float shoulderPatchScale,
    float hipPatchScale,
    float earRootScale,
    const Gdiplus::Color& bodyFill,
    const Gdiplus::Color& headFill,
    const Gdiplus::Color& bodyStroke) {
    Win32MouseCompanionPlaceholderSilhouette silhouette{};

    silhouette.rootFill = BlendColor(headFill, bodyFill, 0.32f);
    silhouette.patchFill = BlendColor(bodyFill, bodyStroke, frontSideIsLeft ? 0.18f : 0.12f);
    silhouette.rearPatchFill = BlendColor(bodyFill, bodyStroke, frontSideIsLeft ? 0.12f : 0.20f);

    silhouette.leftEarRootRect = Gdiplus::RectF(
        headRect.X + headRect.Width * 0.12f + silhouetteNudgePx * 0.12f,
        headRect.Y + headRect.Height * 0.02f,
        headRect.Width * 0.14f * earRootScale,
        headRect.Height * 0.16f * earRootScale);
    silhouette.rightEarRootRect = Gdiplus::RectF(
        headRect.X + headRect.Width * 0.74f + silhouetteNudgePx * 0.12f,
        headRect.Y + headRect.Height * 0.02f,
        headRect.Width * 0.14f * earRootScale,
        headRect.Height * 0.16f * earRootScale);
    silhouette.tailRootRect = Gdiplus::RectF(
        tailRect.X + tailRect.Width * (facingSign > 0.0f ? 0.52f : 0.06f),
        tailRect.Y + tailRect.Height * 0.20f - chestRect.Height * 0.08f,
        tailRect.Width * 0.48f,
        tailRect.Height * 0.52f);

    silhouette.shoulderPatchRect = Gdiplus::RectF(
        bodyRect.X + bodyRect.Width * 0.14f + silhouetteNudgePx * 0.10f,
        bodyRect.Y + bodyRect.Height * 0.26f,
        bodyRect.Width * 0.18f * shoulderPatchScale,
        bodyRect.Height * 0.20f * shoulderPatchScale);
    silhouette.hipPatchRect = Gdiplus::RectF(
        bodyRect.X + bodyRect.Width * 0.70f + silhouetteNudgePx * 0.06f,
        bodyRect.Y + bodyRect.Height * 0.52f,
        bodyRect.Width * 0.16f * hipPatchScale,
        bodyRect.Height * 0.18f * hipPatchScale);
    silhouette.frontDepthPatchRect = Gdiplus::RectF(
        bodyRect.X + (frontSideIsLeft ? bodyRect.Width * 0.08f : bodyRect.Width * 0.54f) + frontDepthBiasPx,
        bodyRect.Y + bodyRect.Height * 0.34f - std::abs(frontDepthBiasPx) * 0.10f,
        bodyRect.Width * 0.18f,
        bodyRect.Height * 0.16f);
    silhouette.rearDepthPatchRect = Gdiplus::RectF(
        bodyRect.X + (frontSideIsLeft ? bodyRect.Width * 0.56f : bodyRect.Width * 0.10f) + rearDepthBiasPx,
        bodyRect.Y + bodyRect.Height * 0.42f - std::abs(rearDepthBiasPx) * 0.08f,
        bodyRect.Width * 0.16f,
        bodyRect.Height * 0.15f);
    return silhouette;
}

} // namespace mousefx::windows
