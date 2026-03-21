#pragma once

#include <gdiplus.h>

namespace mousefx::windows {

struct Win32MouseCompanionPlaceholderSilhouette {
    Gdiplus::RectF leftEarRootRect{};
    Gdiplus::RectF rightEarRootRect{};
    Gdiplus::RectF tailRootRect{};
    Gdiplus::RectF shoulderPatchRect{};
    Gdiplus::RectF hipPatchRect{};
    Gdiplus::RectF frontDepthPatchRect{};
    Gdiplus::RectF rearDepthPatchRect{};
    Gdiplus::Color rootFill{};
    Gdiplus::Color patchFill{};
    Gdiplus::Color rearPatchFill{};
};

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
    const Gdiplus::Color& bodyStroke);

} // namespace mousefx::windows
