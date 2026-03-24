#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyFrameLayerProjector.h"

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

Gdiplus::RectF BlendRect(
    const Gdiplus::RectF& current,
    const Gdiplus::RectF& target,
    float mix) {
    return Gdiplus::RectF(
        current.X + (target.X - current.X) * mix,
        current.Y + (target.Y - current.Y) * mix,
        current.Width + (target.Width - current.Width) * mix,
        current.Height + (target.Height - current.Height) * mix);
}

Gdiplus::RectF ExpandRect(const Gdiplus::RectF& rect, float dx, float dy) {
    return Gdiplus::RectF(
        rect.X - dx,
        rect.Y - dy,
        rect.Width + dx * 2.0f,
        rect.Height + dy * 2.0f);
}

} // namespace

void ApplyWin32MouseCompanionRealRendererModelProxyFrameLayerProjector(
    Win32MouseCompanionRealRendererScene& scene) {
    scene.modelProxyFrameLayer = Win32MouseCompanionRealRendererModelProxyFrameLayer{};

    const auto* bodySilhouette = FindSilhouette(scene, "body");
    const auto* headSilhouette = FindSilhouette(scene, "head");
    const auto* groundingSilhouette = FindSilhouette(scene, "grounding");
    if (bodySilhouette == nullptr || headSilhouette == nullptr ||
        bodySilhouette->bounds.Width <= 0.0f || headSilhouette->bounds.Width <= 0.0f) {
        return;
    }

    const float dominanceMix = 0.60f + scene.proxyDominance * 0.40f;
    scene.modelProxyFrameLayer.visible = true;
    scene.modelProxyFrameLayer.bodyRect = BlendRect(
        scene.bodyRect,
        ExpandRect(bodySilhouette->bounds, bodySilhouette->bounds.Width * 0.04f, bodySilhouette->bounds.Height * 0.03f),
        0.58f);
    scene.modelProxyFrameLayer.headRect = BlendRect(
        scene.headRect,
        ExpandRect(headSilhouette->bounds, headSilhouette->bounds.Width * 0.03f, headSilhouette->bounds.Height * 0.03f),
        0.60f);
    scene.modelProxyFrameLayer.tailRect = BlendRect(
        scene.tailRect,
        Gdiplus::RectF(
            scene.bodyRect.GetRight() - scene.tailRect.Width * (0.88f + 0.06f * dominanceMix),
            scene.bodyRect.Y + scene.bodyRect.Height * 0.42f,
            scene.tailRect.Width * (0.92f + 0.08f * dominanceMix),
            scene.tailRect.Height * (0.94f + 0.06f * dominanceMix)),
        0.48f);
    scene.modelProxyFrameLayer.chestRect = BlendRect(scene.chestRect, scene.bodyRect, 0.18f);
    scene.modelProxyFrameLayer.neckBridgeRect = BlendRect(
        scene.neckBridgeRect,
        Gdiplus::RectF(
            scene.modelProxyFrameLayer.bodyRect.X + scene.modelProxyFrameLayer.bodyRect.Width * 0.36f,
            scene.modelProxyFrameLayer.bodyRect.Y - scene.modelProxyFrameLayer.bodyRect.Height * 0.06f,
            scene.modelProxyFrameLayer.bodyRect.Width * 0.28f,
            scene.modelProxyFrameLayer.bodyRect.Height * 0.24f),
        0.44f);

    const Gdiplus::RectF appendageTarget = groundingSilhouette != nullptr && groundingSilhouette->bounds.Width > 0.0f
        ? groundingSilhouette->bounds
        : scene.modelProxyFrameLayer.bodyRect;
    scene.modelProxyFrameLayer.leftHandRect = BlendRect(scene.leftHandRect, appendageTarget, 0.22f);
    scene.modelProxyFrameLayer.rightHandRect = BlendRect(scene.rightHandRect, appendageTarget, 0.22f);
    scene.modelProxyFrameLayer.leftLegRect = BlendRect(scene.leftLegRect, appendageTarget, 0.26f);
    scene.modelProxyFrameLayer.rightLegRect = BlendRect(scene.rightLegRect, appendageTarget, 0.26f);

    scene.modelProxyFrameLayer.fillAlphaScale = 0.90f + 0.24f * dominanceMix;
    scene.modelProxyFrameLayer.strokeAlphaScale = 0.88f + 0.20f * dominanceMix;
    scene.modelProxyFrameLayer.appendageAlphaScale = 0.86f + 0.18f * dominanceMix;
    scene.modelProxyFrameLayer.strokeWidthScale = 1.02f + 0.10f * dominanceMix;

    scene.previewBodyAlphaScale *= std::max(0.18f, 0.46f - scene.proxyDominance * 0.16f);
    scene.previewHeadAlphaScale *= std::max(0.22f, 0.52f - scene.proxyDominance * 0.14f);
    scene.previewAppendageAlphaScale *= std::max(0.18f, 0.42f - scene.proxyDominance * 0.14f);
}

} // namespace mousefx::windows
