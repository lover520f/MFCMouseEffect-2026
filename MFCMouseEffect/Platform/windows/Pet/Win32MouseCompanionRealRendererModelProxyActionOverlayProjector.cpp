#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyActionOverlayProjector.h"

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

Gdiplus::PointF BlendPoint(
    const Gdiplus::PointF& current,
    const Gdiplus::PointF& target,
    float mix) {
    return Gdiplus::PointF(
        Blend(current.X, target.X, mix),
        Blend(current.Y, target.Y, mix));
}

} // namespace

void ApplyWin32MouseCompanionRealRendererModelProxyActionOverlayProjector(
    Win32MouseCompanionRealRendererScene& scene) {
    if (!scene.modelProxyVisible || scene.modelProxyHull.size() < 3) {
        return;
    }

    const Gdiplus::RectF hullBounds = BuildHullBounds(scene.modelProxyHull);
    if (hullBounds.Width <= 0.0f || hullBounds.Height <= 0.0f) {
        return;
    }

    const Gdiplus::PointF hullCenter(
        hullBounds.X + hullBounds.Width * 0.5f,
        hullBounds.Y + hullBounds.Height * 0.34f);
    const float width = std::max(scene.actionOverlay.clickRingRect.Width, hullBounds.Width * 0.42f);
    const float height = std::max(scene.actionOverlay.clickRingRect.Height, hullBounds.Height * 0.42f);

    if (scene.actionOverlay.clickRingVisible) {
        scene.actionOverlay.clickRingRect = Gdiplus::RectF(
            hullCenter.X - width * 0.5f,
            hullCenter.Y - height * 0.5f,
            width,
            height);
    }

    if (scene.actionOverlay.scrollArcVisible) {
        scene.actionOverlay.scrollArcRect = Gdiplus::RectF(
            hullCenter.X - hullBounds.Width * 0.34f,
            hullCenter.Y - hullBounds.Height * 0.34f,
            std::max(scene.actionOverlay.scrollArcRect.Width, hullBounds.Width * 0.68f),
            std::max(scene.actionOverlay.scrollArcRect.Height, hullBounds.Height * 0.68f));
    }

    if (scene.actionOverlay.holdBandVisible) {
        scene.actionOverlay.holdBandRect = Gdiplus::RectF(
            hullBounds.X + hullBounds.Width * 0.16f,
            hullBounds.GetBottom() - hullBounds.Height * 0.18f,
            std::max(scene.actionOverlay.holdBandRect.Width, hullBounds.Width * 0.68f),
            std::max(scene.actionOverlay.holdBandRect.Height, hullBounds.Height * 0.12f));
    }

    if (scene.actionOverlay.dragLineVisible) {
        scene.actionOverlay.dragLineStart = BlendPoint(
            scene.actionOverlay.dragLineStart,
            Gdiplus::PointF(hullBounds.X + hullBounds.Width * 0.18f, hullCenter.Y),
            0.55f);
        scene.actionOverlay.dragLineEnd = BlendPoint(
            scene.actionOverlay.dragLineEnd,
            Gdiplus::PointF(hullBounds.GetRight() - hullBounds.Width * 0.12f, hullCenter.Y),
            0.55f);
    }

    if (scene.actionOverlay.followTrailVisible) {
        for (size_t i = 0; i < scene.actionOverlay.followTrailRects.size(); ++i) {
            const float step = static_cast<float>(i);
            scene.actionOverlay.followTrailRects[i] = Gdiplus::RectF(
                hullBounds.X + hullBounds.Width * (0.12f + step * 0.10f),
                hullBounds.GetBottom() - hullBounds.Height * (0.24f - step * 0.04f),
                std::max(8.0f, hullBounds.Width * (0.22f - step * 0.03f)),
                std::max(6.0f, hullBounds.Height * (0.14f - step * 0.02f)));
        }
    }
}

} // namespace mousefx::windows
