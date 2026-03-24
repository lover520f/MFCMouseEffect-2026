#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyActionLayerProjector.h"

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

Gdiplus::RectF InflateRect(const Gdiplus::RectF& rect, float dx, float dy) {
    return Gdiplus::RectF(
        rect.X - dx,
        rect.Y - dy,
        rect.Width + dx * 2.0f,
        rect.Height + dy * 2.0f);
}

Gdiplus::RectF MakeCenteredRect(float centerX, float centerY, float width, float height) {
    return Gdiplus::RectF(centerX - width * 0.5f, centerY - height * 0.5f, width, height);
}

float ClampAlpha(float value) {
    return std::clamp(value, 0.0f, 255.0f);
}

} // namespace

void ApplyWin32MouseCompanionRealRendererModelProxyActionLayerProjector(
    Win32MouseCompanionRealRendererScene& scene) {
    scene.modelProxyActionLayer = Win32MouseCompanionRealRendererModelProxyActionLayer{};
    scene.modelProxyActionLayer.accentColor = scene.actionOverlay.accentColor;

    if (!scene.modelProxyVisible || scene.modelProxyHull.size() < 3) {
        return;
    }

    const Gdiplus::RectF hullBounds = BuildHullBounds(scene.modelProxyHull);
    if (hullBounds.Width <= 0.0f || hullBounds.Height <= 0.0f) {
        return;
    }

    const float dominanceMix = 0.60f + scene.proxyDominance * 0.40f;

    if (scene.actionOverlay.clickRingVisible) {
        scene.modelProxyActionLayer.clickShellVisible = true;
        scene.modelProxyActionLayer.clickShellStrokeWidth =
            scene.actionOverlay.clickRingStrokeWidth * (1.08f + 0.18f * dominanceMix);
        scene.modelProxyActionLayer.clickShellAlpha =
            ClampAlpha(scene.actionOverlay.clickRingAlpha * (0.78f + 0.12f * dominanceMix));
        scene.modelProxyActionLayer.clickShellRect = InflateRect(
            hullBounds,
            hullBounds.Width * (0.04f + 0.02f * dominanceMix),
            hullBounds.Height * (0.06f + 0.03f * dominanceMix));
    }

    if (scene.actionOverlay.holdBandVisible) {
        scene.modelProxyActionLayer.holdShellVisible = true;
        scene.modelProxyActionLayer.holdShellAlpha =
            ClampAlpha(scene.actionOverlay.holdBandAlpha * (0.72f + 0.14f * dominanceMix));
        scene.modelProxyActionLayer.holdShellRect = Gdiplus::RectF(
            hullBounds.X + hullBounds.Width * 0.10f,
            hullBounds.Y + hullBounds.Height * (0.54f - 0.04f * dominanceMix),
            hullBounds.Width * 0.80f,
            std::max(10.0f, hullBounds.Height * (0.18f + 0.02f * dominanceMix)));
    }

    if (scene.actionOverlay.scrollArcVisible) {
        scene.modelProxyActionLayer.scrollShellVisible = true;
        scene.modelProxyActionLayer.scrollShellStrokeWidth =
            scene.actionOverlay.scrollArcStrokeWidth * (1.06f + 0.16f * dominanceMix);
        scene.modelProxyActionLayer.scrollShellAlpha =
            ClampAlpha(scene.actionOverlay.scrollArcAlpha * (0.74f + 0.14f * dominanceMix));
        scene.modelProxyActionLayer.scrollShellRect = InflateRect(
            hullBounds,
            hullBounds.Width * 0.10f,
            hullBounds.Height * 0.08f);
        scene.modelProxyActionLayer.scrollShellStartDeg = scene.actionOverlay.scrollArcStartDeg;
        scene.modelProxyActionLayer.scrollShellSweepDeg = scene.actionOverlay.scrollArcSweepDeg;
    }

    if (scene.actionOverlay.dragLineVisible) {
        scene.modelProxyActionLayer.dragShellVisible = true;
        scene.modelProxyActionLayer.dragShellStrokeWidth =
            scene.actionOverlay.dragLineStrokeWidth * (1.08f + 0.14f * dominanceMix);
        scene.modelProxyActionLayer.dragShellAlpha =
            ClampAlpha(scene.actionOverlay.dragLineAlpha * (0.72f + 0.12f * dominanceMix));
        scene.modelProxyActionLayer.dragShellStart = Gdiplus::PointF(
            hullBounds.X + hullBounds.Width * 0.18f,
            hullBounds.Y + hullBounds.Height * 0.36f);
        scene.modelProxyActionLayer.dragShellEnd = Gdiplus::PointF(
            hullBounds.GetRight() - hullBounds.Width * 0.10f,
            hullBounds.Y + hullBounds.Height * 0.22f);
    }

    if (scene.actionOverlay.followTrailVisible) {
        scene.modelProxyActionLayer.followShellVisible = true;
        scene.modelProxyActionLayer.followShellBaseAlpha =
            ClampAlpha(scene.actionOverlay.followTrailBaseAlpha * (0.70f + 0.14f * dominanceMix));
        const float baseX = hullBounds.X + hullBounds.Width * (0.16f - 0.06f * scene.facingSign);
        const float baseY = hullBounds.Y + hullBounds.Height * 0.64f;
        for (size_t i = 0; i < scene.modelProxyActionLayer.followShellRects.size(); ++i) {
            const float step = static_cast<float>(i);
            const float scale = 1.0f - step * 0.14f;
            scene.modelProxyActionLayer.followShellRects[i] = MakeCenteredRect(
                baseX - scene.facingSign * hullBounds.Width * 0.14f * step,
                baseY + hullBounds.Height * 0.08f * step,
                hullBounds.Width * 0.26f * scale,
                hullBounds.Height * 0.18f * scale);
        }
    }
}

} // namespace mousefx::windows
