#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyAppendageGeometryProjector.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

float Blend(float current, float target, float mix) {
    return current + (target - current) * mix;
}

Gdiplus::PointF RectCenter(const Gdiplus::RectF& rect) {
    return Gdiplus::PointF(
        rect.X + rect.Width * 0.5f,
        rect.Y + rect.Height * 0.5f);
}

void BlendRect(
    Gdiplus::RectF* rect,
    const Gdiplus::RectF& target,
    float mix) {
    if (rect == nullptr) {
        return;
    }
    rect->X = Blend(rect->X, target.X, mix);
    rect->Y = Blend(rect->Y, target.Y, mix);
    rect->Width = Blend(rect->Width, target.Width, mix);
    rect->Height = Blend(rect->Height, target.Height, mix);
}

Gdiplus::PointF BlendPoint(
    const Gdiplus::PointF& current,
    const Gdiplus::PointF& target,
    float mix) {
    return Gdiplus::PointF(
        Blend(current.X, target.X, mix),
        Blend(current.Y, target.Y, mix));
}

float ResolveGeometryMix(const Win32MouseCompanionRealRendererScene& scene) {
    float mix = std::clamp(scene.proxyDominance, 0.0f, 1.0f) * 0.42f;
    if (scene.actionOverlay.holdBandVisible) {
        mix += 0.16f;
    }
    if (scene.actionOverlay.dragLineVisible) {
        mix += 0.14f;
    }
    if (scene.actionOverlay.followTrailVisible) {
        mix += 0.10f;
    }
    return std::clamp(mix, 0.0f, 0.78f);
}

float ResolveShellMix(const Win32MouseCompanionRealRendererScene& scene) {
    float mix = std::clamp(scene.proxyDominance, 0.0f, 1.0f) * 0.34f;
    if (scene.modelProxyActionLayer.holdShellVisible) {
        mix += 0.10f;
    }
    if (scene.modelProxyActionLayer.dragShellVisible) {
        mix += 0.10f;
    }
    if (scene.modelProxyActionLayer.followShellVisible) {
        mix += 0.08f;
    }
    return std::clamp(mix, 0.0f, 0.64f);
}

} // namespace

void ApplyWin32MouseCompanionRealRendererModelProxyAppendageGeometryProjector(
    Win32MouseCompanionRealRendererScene& scene) {
    if (!scene.modelProxyAppendageLayer.visible) {
        return;
    }

    const float mix = ResolveGeometryMix(scene);
    if (mix <= 0.0f) {
        return;
    }

    const auto& layer = scene.modelProxyAppendageLayer;
    const auto& frameLayer = scene.modelProxyFrameLayer;
    const float shellMix =
        frameLayer.visible ? ResolveShellMix(scene) : mix * 0.72f;

    if (frameLayer.visible) {
        BlendRect(&scene.tailRect, frameLayer.tailRect, shellMix);
        BlendRect(&scene.leftHandRect, frameLayer.leftHandRect, shellMix);
        BlendRect(&scene.rightHandRect, frameLayer.rightHandRect, shellMix);
        BlendRect(&scene.leftLegRect, frameLayer.leftLegRect, shellMix);
        BlendRect(&scene.rightLegRect, frameLayer.rightLegRect, shellMix);
        scene.appendageAnchorScale = Blend(
            scene.appendageAnchorScale,
            1.04f + scene.proxyDominance * 0.10f,
            shellMix);
    }

    BlendRect(&scene.tailRootCuffRect, layer.tailRootCuffRect, mix);
    BlendRect(&scene.tailBridgeRect, layer.tailBridgeRect, mix);
    BlendRect(&scene.tailMidContourRect, layer.tailMidContourRect, mix);
    BlendRect(&scene.tailTipBridgeRect, layer.tailTipBridgeRect, mix);
    BlendRect(&scene.tailTipRect, layer.tailTipRect, mix);

    BlendRect(&scene.leftHandSilhouetteBridgeRect, layer.leftHandSilhouetteBridgeRect, mix);
    BlendRect(&scene.rightHandSilhouetteBridgeRect, layer.rightHandSilhouetteBridgeRect, mix);
    BlendRect(&scene.leftHandCadenceBridgeRect, layer.leftHandCadenceBridgeRect, mix);
    BlendRect(&scene.rightHandCadenceBridgeRect, layer.rightHandCadenceBridgeRect, mix);
    BlendRect(&scene.leftHandRootCuffRect, layer.leftHandRootCuffRect, mix);
    BlendRect(&scene.rightHandRootCuffRect, layer.rightHandRootCuffRect, mix);
    BlendRect(&scene.leftHandPadRect, layer.leftHandPadRect, mix);
    BlendRect(&scene.rightHandPadRect, layer.rightHandPadRect, mix);

    BlendRect(&scene.leftLegSilhouetteBridgeRect, layer.leftLegSilhouetteBridgeRect, mix);
    BlendRect(&scene.rightLegSilhouetteBridgeRect, layer.rightLegSilhouetteBridgeRect, mix);
    BlendRect(&scene.leftLegCadenceBridgeRect, layer.leftLegCadenceBridgeRect, mix);
    BlendRect(&scene.rightLegCadenceBridgeRect, layer.rightLegCadenceBridgeRect, mix);
    BlendRect(&scene.leftLegRootCuffRect, layer.leftLegRootCuffRect, mix);
    BlendRect(&scene.rightLegRootCuffRect, layer.rightLegRootCuffRect, mix);
    BlendRect(&scene.leftLegPadRect, layer.leftLegPadRect, mix);
    BlendRect(&scene.rightLegPadRect, layer.rightLegPadRect, mix);

    const Gdiplus::PointF tailCenter = RectCenter(layer.tailBridgeRect);
    const Gdiplus::PointF limbCenter = Gdiplus::PointF(
        (RectCenter(layer.leftHandCadenceBridgeRect).X +
         RectCenter(layer.rightHandCadenceBridgeRect).X +
         RectCenter(layer.leftLegCadenceBridgeRect).X +
         RectCenter(layer.rightLegCadenceBridgeRect).X) /
            4.0f,
        (RectCenter(layer.leftHandCadenceBridgeRect).Y +
         RectCenter(layer.rightHandCadenceBridgeRect).Y +
         RectCenter(layer.leftLegCadenceBridgeRect).Y +
         RectCenter(layer.rightLegCadenceBridgeRect).Y) /
            4.0f);
    const Gdiplus::PointF targetAnchor = Gdiplus::PointF(
        Blend(tailCenter.X, limbCenter.X, 0.64f),
        Blend(tailCenter.Y, limbCenter.Y, 0.64f));
    scene.appendageAnchor = BlendPoint(scene.appendageAnchor, targetAnchor, mix);
}

} // namespace mousefx::windows
