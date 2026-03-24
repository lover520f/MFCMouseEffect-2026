#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyAdornmentLayerProjector.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

const Win32MouseCompanionRealRendererModelProxySilhouette* FindSilhouette(
    const Win32MouseCompanionRealRendererScene& scene,
    const char* logicalNode) {
    for (const auto& silhouette : scene.modelProxySilhouettes) {
        if (silhouette.logicalNode == (logicalNode ? logicalNode : "")) {
            return &silhouette;
        }
    }
    return nullptr;
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

Gdiplus::RectF BlendRect(
    const Gdiplus::RectF& current,
    const Gdiplus::RectF& target,
    float mix) {
    return Gdiplus::RectF(
        Blend(current.X, target.X, mix),
        Blend(current.Y, target.Y, mix),
        Blend(current.Width, target.Width, mix),
        Blend(current.Height, target.Height, mix));
}

Gdiplus::PointF ScalePointAround(
    const Gdiplus::PointF& point,
    const Gdiplus::PointF& center,
    float scale) {
    return Gdiplus::PointF(
        center.X + (point.X - center.X) * scale,
        center.Y + (point.Y - center.Y) * scale);
}

template <size_t N>
void CopyAndScalePoints(
    const std::array<Gdiplus::PointF, N>& source,
    std::array<Gdiplus::PointF, N>* target,
    const Gdiplus::PointF& center,
    float scale) {
    if (target == nullptr) {
        return;
    }
    for (size_t i = 0; i < N; ++i) {
        (*target)[i] = ScalePointAround(source[i], center, scale);
    }
}

void ProjectBadgeLane(
    const Gdiplus::RectF& hullBounds,
    std::array<Gdiplus::RectF, 3>* badges) {
    if (badges == nullptr || badges->empty()) {
        return;
    }
    const float width = std::max(12.0f, hullBounds.Width * 0.10f);
    const float height = std::max(9.0f, hullBounds.Height * 0.08f);
    const float spacing = width * 0.18f;
    const float totalWidth = width * 3.0f + spacing * 2.0f;
    const float startX = hullBounds.X + (hullBounds.Width - totalWidth) * 0.5f;
    const float y = hullBounds.GetBottom() + hullBounds.Height * 0.10f;
    for (size_t i = 0; i < badges->size(); ++i) {
        (*badges)[i] = Gdiplus::RectF(
            startX + static_cast<float>(i) * (width + spacing),
            y,
            width,
            height);
    }
}

} // namespace

void ApplyWin32MouseCompanionRealRendererModelProxyAdornmentLayerProjector(
    Win32MouseCompanionRealRendererScene& scene) {
    scene.modelProxyAdornmentLayer = Win32MouseCompanionRealRendererModelProxyAdornmentLayer{};

    if (!scene.modelProxyVisible || scene.modelProxyHull.size() < 3) {
        return;
    }

    float minX = scene.modelProxyHull.front().X;
    float maxX = scene.modelProxyHull.front().X;
    float minY = scene.modelProxyHull.front().Y;
    float maxY = scene.modelProxyHull.front().Y;
    for (const auto& point : scene.modelProxyHull) {
        minX = std::min(minX, point.X);
        maxX = std::max(maxX, point.X);
        minY = std::min(minY, point.Y);
        maxY = std::max(maxY, point.Y);
    }
    const Gdiplus::RectF hullBounds(minX, minY, maxX - minX, maxY - minY);
    if (hullBounds.Width <= 0.0f || hullBounds.Height <= 0.0f) {
        return;
    }

    const float dominanceMix = 0.56f + scene.proxyDominance * 0.44f;
    scene.modelProxyAdornmentLayer.visible = true;
    scene.modelProxyAdornmentLayer.laneReady = scene.laneReady;
    scene.modelProxyAdornmentLayer.laneAlphaScale = 0.86f + 0.26f * dominanceMix;
    ProjectBadgeLane(hullBounds, &scene.modelProxyAdornmentLayer.laneBadgeRects);

    if (scene.poseBadgeVisible) {
        const auto* headSilhouette = FindSilhouette(scene, "head");
        const Gdiplus::RectF targetRect = headSilhouette != nullptr && headSilhouette->bounds.Width > 0.0f
            ? Gdiplus::RectF(
                  headSilhouette->bounds.X + headSilhouette->bounds.Width * 0.64f,
                  headSilhouette->bounds.Y - headSilhouette->bounds.Height * 0.12f,
                  std::max(scene.poseBadgeRect.Width, headSilhouette->bounds.Width * 0.20f),
                  std::max(scene.poseBadgeRect.Height, headSilhouette->bounds.Height * 0.20f))
            : scene.poseBadgeRect;
        scene.modelProxyAdornmentLayer.poseBadgeVisible = true;
        scene.modelProxyAdornmentLayer.poseBadgeRect = BlendRect(scene.poseBadgeRect, targetRect, 0.52f);
        scene.modelProxyAdornmentLayer.poseBadgeAlphaScale = 0.92f + 0.24f * dominanceMix;
    }

    if (scene.accessoryVisible) {
        scene.modelProxyAdornmentLayer.accessoryVisible = true;
        scene.modelProxyAdornmentLayer.accessoryShape = scene.accessoryShape;
        scene.modelProxyAdornmentLayer.accessoryAlphaScale =
            scene.accessoryAlphaScale * (0.92f + 0.22f * dominanceMix);
        scene.modelProxyAdornmentLayer.accessoryStrokeScale = 1.02f + 0.12f * dominanceMix;

        const Gdiplus::PointF center(
            scene.accessoryBounds.X + scene.accessoryBounds.Width * 0.5f,
            scene.accessoryBounds.Y + scene.accessoryBounds.Height * 0.5f);
        const float scale = 1.04f + 0.08f * dominanceMix;
        scene.modelProxyAdornmentLayer.accessoryBounds = BlendRect(
            scene.accessoryBounds,
            Gdiplus::RectF(
                center.X - scene.accessoryBounds.Width * scale * 0.5f,
                center.Y - scene.accessoryBounds.Height * scale * 0.5f,
                scene.accessoryBounds.Width * scale,
                scene.accessoryBounds.Height * scale),
            0.52f);
        scene.modelProxyAdornmentLayer.accessoryMoonInsetRect = scene.accessoryMoonInsetRect;
        scene.modelProxyAdornmentLayer.accessoryRibbonCenter = scene.accessoryRibbonCenter;
        CopyAndScalePoints(scene.accessoryStar, &scene.modelProxyAdornmentLayer.accessoryStar, center, scale);
        CopyAndScalePoints(scene.accessoryMoon, &scene.modelProxyAdornmentLayer.accessoryMoon, center, scale);
        CopyAndScalePoints(scene.accessoryLeaf, &scene.modelProxyAdornmentLayer.accessoryLeaf, center, scale);
        CopyAndScalePoints(scene.accessoryRibbonLeft, &scene.modelProxyAdornmentLayer.accessoryRibbonLeft, center, scale);
        CopyAndScalePoints(scene.accessoryRibbonRight, &scene.modelProxyAdornmentLayer.accessoryRibbonRight, center, scale);
        scene.modelProxyAdornmentLayer.accessoryLeafVeinStart =
            ScalePointAround(scene.accessoryLeafVeinStart, center, scale);
        scene.modelProxyAdornmentLayer.accessoryLeafVeinEnd =
            ScalePointAround(scene.accessoryLeafVeinEnd, center, scale);
        scene.modelProxyAdornmentLayer.accessoryRibbonLeftFoldStart =
            ScalePointAround(scene.accessoryRibbonLeftFoldStart, center, scale);
        scene.modelProxyAdornmentLayer.accessoryRibbonLeftFoldEnd =
            ScalePointAround(scene.accessoryRibbonLeftFoldEnd, center, scale);
        scene.modelProxyAdornmentLayer.accessoryRibbonRightFoldStart =
            ScalePointAround(scene.accessoryRibbonRightFoldStart, center, scale);
        scene.modelProxyAdornmentLayer.accessoryRibbonRightFoldEnd =
            ScalePointAround(scene.accessoryRibbonRightFoldEnd, center, scale);
    }

    scene.previewAdornmentAlphaScale *= std::max(0.22f, 0.54f - scene.proxyDominance * 0.20f);
}

} // namespace mousefx::windows
