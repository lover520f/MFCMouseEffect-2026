#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyAppendageActionProjector.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

float ResolveAppendageActionBoost(const Win32MouseCompanionRealRendererScene& scene) {
    float boost = 0.0f;
    if (scene.actionOverlay.holdBandVisible) {
        boost += 1.0f;
    }
    if (scene.actionOverlay.dragLineVisible) {
        boost += 0.94f;
    }
    if (scene.actionOverlay.followTrailVisible) {
        boost += 0.72f;
    }
    if (scene.actionOverlay.scrollArcVisible) {
        boost += 0.36f;
    }
    if (scene.actionOverlay.clickRingVisible) {
        boost += 0.20f;
    }
    return std::clamp(boost, 0.0f, 2.2f);
}

void ScaleRectAboutCenter(Gdiplus::RectF* rect, float scale) {
    if (rect == nullptr || scale <= 0.0f) {
        return;
    }
    const float centerX = rect->X + rect->Width * 0.5f;
    const float centerY = rect->Y + rect->Height * 0.5f;
    rect->Width *= scale;
    rect->Height *= scale;
    rect->X = centerX - rect->Width * 0.5f;
    rect->Y = centerY - rect->Height * 0.5f;
}

} // namespace

void ApplyWin32MouseCompanionRealRendererModelProxyAppendageActionProjector(
    Win32MouseCompanionRealRendererScene& scene) {
    if (!scene.modelProxyAppendageLayer.visible) {
        return;
    }

    const float boost = ResolveAppendageActionBoost(scene);
    if (boost <= 0.0f) {
        return;
    }

    const float dominanceMix = 0.62f + scene.proxyDominance * 0.38f;
    const float shapeScale = 1.0f + 0.040f * std::min(2.0f, boost) * dominanceMix;
    const float padScale = 1.0f + 0.062f * std::min(2.0f, boost) * dominanceMix;
    auto& layer = scene.modelProxyAppendageLayer;

    ScaleRectAboutCenter(&layer.tailRootCuffRect, shapeScale);
    ScaleRectAboutCenter(&layer.tailBridgeRect, shapeScale);
    ScaleRectAboutCenter(&layer.tailMidContourRect, shapeScale);
    ScaleRectAboutCenter(&layer.tailTipBridgeRect, shapeScale);
    ScaleRectAboutCenter(&layer.tailTipRect, shapeScale * 1.03f);

    ScaleRectAboutCenter(&layer.leftLegSilhouetteBridgeRect, shapeScale);
    ScaleRectAboutCenter(&layer.rightLegSilhouetteBridgeRect, shapeScale);
    ScaleRectAboutCenter(&layer.leftLegCadenceBridgeRect, shapeScale * 1.04f);
    ScaleRectAboutCenter(&layer.rightLegCadenceBridgeRect, shapeScale * 1.04f);
    ScaleRectAboutCenter(&layer.leftLegRootCuffRect, shapeScale);
    ScaleRectAboutCenter(&layer.rightLegRootCuffRect, shapeScale);
    ScaleRectAboutCenter(&layer.leftLegPadRect, padScale);
    ScaleRectAboutCenter(&layer.rightLegPadRect, padScale);

    ScaleRectAboutCenter(&layer.leftHandSilhouetteBridgeRect, shapeScale);
    ScaleRectAboutCenter(&layer.rightHandSilhouetteBridgeRect, shapeScale);
    ScaleRectAboutCenter(&layer.leftHandCadenceBridgeRect, shapeScale * 1.04f);
    ScaleRectAboutCenter(&layer.rightHandCadenceBridgeRect, shapeScale * 1.04f);
    ScaleRectAboutCenter(&layer.leftHandRootCuffRect, shapeScale);
    ScaleRectAboutCenter(&layer.rightHandRootCuffRect, shapeScale);
    ScaleRectAboutCenter(&layer.leftHandPadRect, padScale);
    ScaleRectAboutCenter(&layer.rightHandPadRect, padScale);

    layer.rearAlphaScale = std::clamp(
        layer.rearAlphaScale + 0.24f * boost * dominanceMix,
        0.92f,
        1.42f);
    layer.accentAlphaScale = std::clamp(
        layer.accentAlphaScale + 0.30f * boost * dominanceMix,
        0.90f,
        1.54f);
    layer.strokeAlphaScale = std::clamp(
        layer.strokeAlphaScale + 0.22f * boost * dominanceMix,
        0.88f,
        1.36f);

    scene.previewAppendageAlphaScale *= std::max(
        0.10f,
        0.28f - 0.08f * std::min(1.8f, boost) * dominanceMix);
}

} // namespace mousefx::windows
