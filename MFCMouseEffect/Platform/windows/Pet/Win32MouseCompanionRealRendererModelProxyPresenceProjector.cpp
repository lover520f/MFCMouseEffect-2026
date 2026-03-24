#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyPresenceProjector.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

Gdiplus::RectF BuildHullBounds(const std::vector<Gdiplus::PointF>& points) {
    if (points.empty()) {
        return Gdiplus::RectF{};
    }
    float minX = points.front().X;
    float maxX = points.front().X;
    float minY = points.front().Y;
    float maxY = points.front().Y;
    for (const auto& point : points) {
        minX = std::min(minX, point.X);
        maxX = std::max(maxX, point.X);
        minY = std::min(minY, point.Y);
        maxY = std::max(maxY, point.Y);
    }
    return Gdiplus::RectF(minX, minY, maxX - minX, maxY - minY);
}

float Blend(float current, float target, float mix) {
    return current + (target - current) * mix;
}

} // namespace

void ApplyWin32MouseCompanionRealRendererModelProxyPresenceProjector(
    Win32MouseCompanionRealRendererScene& scene) {
    scene.proxyDominance = 0.0f;
    scene.previewBodyAlphaScale = 1.0f;
    scene.previewHeadAlphaScale = 1.0f;
    scene.previewAppendageAlphaScale = 1.0f;
    scene.previewTailAlphaScale = 1.0f;
    scene.previewHandAlphaScale = 1.0f;
    scene.previewLegAlphaScale = 1.0f;
    scene.previewTailStrokeScale = 1.0f;
    scene.previewHandStrokeScale = 1.0f;
    scene.previewLegStrokeScale = 1.0f;
    scene.previewDetailAlphaScale = 1.0f;
    scene.previewAdornmentAlphaScale = 1.0f;

    if (!scene.modelProxyVisible || scene.modelProxyHull.size() < 3) {
        return;
    }

    const Gdiplus::RectF hullBounds = BuildHullBounds(scene.modelProxyHull);
    if (hullBounds.Width <= 0.0f || hullBounds.Height <= 0.0f) {
        return;
    }

    const float hullArea = hullBounds.Width * hullBounds.Height;
    const float previewArea = std::max(
        1.0f,
        scene.bodyRect.Width * scene.bodyRect.Height +
            scene.headRect.Width * scene.headRect.Height +
            scene.tailRect.Width * scene.tailRect.Height * 0.45f);
    const float coverage = std::clamp(hullArea / previewArea, 0.20f, 1.28f);
    const float nodeDensity = std::clamp(
        static_cast<float>(scene.modelProxyNodes.size()) / 5.0f,
        0.0f,
        1.0f);
    const float dominance = std::clamp(
        coverage * 0.58f + nodeDensity * 0.32f,
        0.18f,
        0.92f);
    scene.proxyDominance = dominance;

    scene.previewBodyAlphaScale = Blend(1.0f, 0.54f, dominance);
    scene.previewHeadAlphaScale = Blend(1.0f, 0.58f, dominance);
    scene.previewAppendageAlphaScale = Blend(1.0f, 0.50f, dominance);
    scene.previewDetailAlphaScale = Blend(1.0f, 0.64f, dominance);
    scene.previewAdornmentAlphaScale = Blend(1.0f, 0.60f, dominance);
}

} // namespace mousefx::windows
