#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyPaletteProjector.h"

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

BYTE BlendChannel(BYTE from, BYTE to, float mix) {
    const float blended =
        static_cast<float>(from) + (static_cast<float>(to) - static_cast<float>(from)) * mix;
    return static_cast<BYTE>(std::clamp(std::lround(blended), 0L, 255L));
}

Gdiplus::Color BlendToward(
    const Gdiplus::Color& base,
    const Gdiplus::Color& tint,
    float mix) {
    const float clampedMix = std::clamp(mix, 0.0f, 1.0f);
    return Gdiplus::Color(
        BlendChannel(base.GetA(), tint.GetA(), clampedMix),
        BlendChannel(base.GetR(), tint.GetR(), clampedMix),
        BlendChannel(base.GetG(), tint.GetG(), clampedMix),
        BlendChannel(base.GetB(), tint.GetB(), clampedMix));
}

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

float ResolveCoverageSignal(const Win32MouseCompanionRealRendererScene& scene) {
    if (!scene.modelProxyVisible || scene.modelProxyHull.size() < 3) {
        return 0.0f;
    }
    const Gdiplus::RectF hullBounds = BuildHullBounds(scene.modelProxyHull);
    const float hullArea = hullBounds.Width * hullBounds.Height;
    const float previewArea =
        std::max(1.0f, scene.bodyRect.Width * scene.bodyRect.Height + scene.headRect.Width * scene.headRect.Height);
    return std::clamp(hullArea / previewArea, 0.18f, 1.35f);
}

float BlendScalar(float current, float target, float mix) {
    return current + (target - current) * mix;
}

} // namespace

void ApplyWin32MouseCompanionRealRendererModelProxyPaletteProjector(
    Win32MouseCompanionRealRendererScene& scene) {
    if (!scene.modelProxyVisible) {
        return;
    }

    const auto* bodySilhouette = FindSilhouette(scene, "body");
    const auto* headSilhouette = FindSilhouette(scene, "head");
    const auto* appendageSilhouette = FindSilhouette(scene, "appendage");
    const auto* overlaySilhouette = FindSilhouette(scene, "overlay");
    const auto* groundingSilhouette = FindSilhouette(scene, "grounding");
    const float coverageSignal = ResolveCoverageSignal(scene);
    const float baseMix = std::clamp(0.12f + coverageSignal * 0.14f, 0.12f, 0.28f);

    if (bodySilhouette != nullptr) {
        scene.bodyFill = BlendToward(scene.bodyFill, bodySilhouette->fill, baseMix);
        scene.bodyFillRear = BlendToward(scene.bodyFillRear, bodySilhouette->fill, baseMix * 0.84f);
        scene.bodyStroke = BlendToward(scene.bodyStroke, bodySilhouette->fill, baseMix * 0.74f);
        scene.shadowFill = BlendToward(scene.shadowFill, bodySilhouette->fill, baseMix * 0.42f);
    }

    if (headSilhouette != nullptr) {
        scene.headFill = BlendToward(scene.headFill, headSilhouette->fill, baseMix);
        scene.headFillRear = BlendToward(scene.headFillRear, headSilhouette->fill, baseMix * 0.88f);
        scene.earFill = BlendToward(scene.earFill, headSilhouette->fill, baseMix * 0.74f);
        scene.earFillRear = BlendToward(scene.earFillRear, headSilhouette->fill, baseMix * 0.70f);
        scene.earRootCuffFill = BlendToward(scene.earRootCuffFill, headSilhouette->fill, baseMix * 0.56f);
        scene.earRootCuffFillRear = BlendToward(scene.earRootCuffFillRear, headSilhouette->fill, baseMix * 0.52f);
    }

    if (appendageSilhouette != nullptr) {
        scene.tailFill = BlendToward(scene.tailFill, appendageSilhouette->fill, baseMix * 0.90f);
        scene.tailFillRear = BlendToward(scene.tailFillRear, appendageSilhouette->fill, baseMix * 0.82f);
        scene.tailMidFill = BlendToward(scene.tailMidFill, appendageSilhouette->fill, baseMix * 0.72f);
        scene.tailTipFill = BlendToward(scene.tailTipFill, appendageSilhouette->fill, baseMix * 0.54f);
        scene.tailStroke = BlendToward(scene.tailStroke, appendageSilhouette->fill, baseMix * 0.46f);
    }

    if (overlaySilhouette != nullptr) {
        scene.accentFill = BlendToward(scene.accentFill, overlaySilhouette->fill, baseMix);
        scene.accessoryFill = BlendToward(scene.accessoryFill, overlaySilhouette->fill, baseMix * 0.84f);
        scene.accessoryStroke = BlendToward(scene.accessoryStroke, overlaySilhouette->fill, baseMix * 0.76f);
        scene.actionOverlay.accentColor =
            BlendToward(scene.actionOverlay.accentColor, overlaySilhouette->fill, baseMix * 0.92f);
    }

    if (groundingSilhouette != nullptr) {
        scene.pedestalFill = BlendToward(scene.pedestalFill, groundingSilhouette->fill, baseMix * 0.90f);
        scene.badgeReadyFill = BlendToward(scene.badgeReadyFill, groundingSilhouette->fill, baseMix * 0.48f);
        scene.badgePendingFill = BlendToward(scene.badgePendingFill, groundingSilhouette->fill, baseMix * 0.38f);
        scene.glowColor = BlendToward(scene.glowColor, groundingSilhouette->fill, baseMix * 0.34f);
    }

    scene.bodyStrokeWidth = BlendScalar(scene.bodyStrokeWidth, scene.bodyStrokeWidth * (1.06f + coverageSignal * 0.04f), 0.52f);
    scene.headStrokeWidth = BlendScalar(scene.headStrokeWidth, scene.headStrokeWidth * (1.06f + coverageSignal * 0.05f), 0.54f);
    scene.earStrokeWidth = BlendScalar(scene.earStrokeWidth, scene.earStrokeWidth * (1.08f + coverageSignal * 0.04f), 0.56f);
    scene.earStrokeWidthRear = BlendScalar(scene.earStrokeWidthRear, scene.earStrokeWidthRear * (1.06f + coverageSignal * 0.04f), 0.56f);
    scene.limbStrokeWidth = BlendScalar(scene.limbStrokeWidth, scene.limbStrokeWidth * (1.10f + coverageSignal * 0.04f), 0.58f);
    scene.tailStrokeWidth = BlendScalar(scene.tailStrokeWidth, scene.tailStrokeWidth * (1.08f + coverageSignal * 0.04f), 0.56f);
    scene.chestStrokeWidth = BlendScalar(scene.chestStrokeWidth, scene.chestStrokeWidth * (1.08f + coverageSignal * 0.04f), 0.54f);
    scene.accessoryStrokeWidth = BlendScalar(scene.accessoryStrokeWidth, scene.accessoryStrokeWidth * (1.10f + coverageSignal * 0.03f), 0.58f);
    scene.whiskerStrokeWidth = BlendScalar(scene.whiskerStrokeWidth, scene.whiskerStrokeWidth * (1.08f + coverageSignal * 0.03f), 0.50f);
    scene.mouthStrokeWidth = BlendScalar(scene.mouthStrokeWidth, scene.mouthStrokeWidth * (1.06f + coverageSignal * 0.03f), 0.48f);
    scene.shadowAlphaScale = std::min(1.26f, BlendScalar(scene.shadowAlphaScale, scene.shadowAlphaScale * 1.06f, 0.46f));
    scene.pedestalAlphaScale = std::min(1.24f, BlendScalar(scene.pedestalAlphaScale, scene.pedestalAlphaScale * 1.05f, 0.44f));
    scene.poseBadgeAlpha = std::min(255.0f, BlendScalar(scene.poseBadgeAlpha, scene.poseBadgeAlpha + 10.0f + coverageSignal * 8.0f, 0.40f));
    scene.accessoryAlphaScale = std::min(1.34f, BlendScalar(scene.accessoryAlphaScale, scene.accessoryAlphaScale * (1.06f + coverageSignal * 0.02f), 0.48f));
    scene.glowAlpha = std::min(255.0f, BlendScalar(scene.glowAlpha, scene.glowAlpha + 8.0f + coverageSignal * 12.0f, 0.44f));
}

} // namespace mousefx::windows
