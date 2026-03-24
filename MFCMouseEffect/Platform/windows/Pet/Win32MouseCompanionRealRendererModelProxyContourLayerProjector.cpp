#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyContourLayerProjector.h"

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

Gdiplus::PointF MapPoint(
    const Gdiplus::RectF& from,
    const Gdiplus::RectF& to,
    const Gdiplus::PointF& point) {
    const float xNorm = from.Width > 0.0f ? (point.X - from.X) / from.Width : 0.5f;
    const float yNorm = from.Height > 0.0f ? (point.Y - from.Y) / from.Height : 0.5f;
    return Gdiplus::PointF(to.X + to.Width * xNorm, to.Y + to.Height * yNorm);
}

Gdiplus::RectF MapRect(
    const Gdiplus::RectF& from,
    const Gdiplus::RectF& to,
    const Gdiplus::RectF& rect) {
    const Gdiplus::PointF topLeft = MapPoint(from, to, Gdiplus::PointF(rect.X, rect.Y));
    const Gdiplus::PointF bottomRight =
        MapPoint(from, to, Gdiplus::PointF(rect.GetRight(), rect.GetBottom()));
    return Gdiplus::RectF(
        topLeft.X,
        topLeft.Y,
        bottomRight.X - topLeft.X,
        bottomRight.Y - topLeft.Y);
}

template <size_t N>
void MapPointArray(
    const std::array<Gdiplus::PointF, N>& source,
    const Gdiplus::RectF& from,
    const Gdiplus::RectF& to,
    std::array<Gdiplus::PointF, N>* target) {
    if (target == nullptr) {
        return;
    }
    for (size_t i = 0; i < N; ++i) {
        (*target)[i] = MapPoint(from, to, source[i]);
    }
}

} // namespace

void ApplyWin32MouseCompanionRealRendererModelProxyContourLayerProjector(
    Win32MouseCompanionRealRendererScene& scene) {
    scene.modelProxyContourLayer = Win32MouseCompanionRealRendererModelProxyContourLayer{};

    const auto* bodySilhouette = FindSilhouette(scene, "body");
    const auto* headSilhouette = FindSilhouette(scene, "head");
    if (bodySilhouette == nullptr || headSilhouette == nullptr ||
        bodySilhouette->bounds.Width <= 0.0f || headSilhouette->bounds.Width <= 0.0f) {
        return;
    }

    const Gdiplus::RectF bodyTo = scene.modelProxyFrameLayer.visible
        ? scene.modelProxyFrameLayer.bodyRect
        : bodySilhouette->bounds;
    const Gdiplus::RectF headTo = scene.modelProxyFrameLayer.visible
        ? scene.modelProxyFrameLayer.headRect
        : headSilhouette->bounds;

    scene.modelProxyContourLayer.visible = true;
    MapPointArray(scene.leftEar, scene.headRect, headTo, &scene.modelProxyContourLayer.leftEar);
    MapPointArray(scene.rightEar, scene.headRect, headTo, &scene.modelProxyContourLayer.rightEar);
    scene.modelProxyContourLayer.leftEarRootCuffRect =
        MapRect(scene.headRect, headTo, scene.leftEarRootCuffRect);
    scene.modelProxyContourLayer.rightEarRootCuffRect =
        MapRect(scene.headRect, headTo, scene.rightEarRootCuffRect);
    scene.modelProxyContourLayer.leftEarOcclusionCapRect =
        MapRect(scene.headRect, headTo, scene.leftEarOcclusionCapRect);
    scene.modelProxyContourLayer.rightEarOcclusionCapRect =
        MapRect(scene.headRect, headTo, scene.rightEarOcclusionCapRect);
    scene.modelProxyContourLayer.leftHeadShoulderBridgeRect =
        MapRect(scene.bodyRect, bodyTo, scene.leftHeadShoulderBridgeRect);
    scene.modelProxyContourLayer.rightHeadShoulderBridgeRect =
        MapRect(scene.bodyRect, bodyTo, scene.rightHeadShoulderBridgeRect);
    scene.modelProxyContourLayer.leftShoulderPatchRect =
        MapRect(scene.bodyRect, bodyTo, scene.leftShoulderPatchRect);
    scene.modelProxyContourLayer.rightShoulderPatchRect =
        MapRect(scene.bodyRect, bodyTo, scene.rightShoulderPatchRect);
    scene.modelProxyContourLayer.leftHipPatchRect =
        MapRect(scene.bodyRect, bodyTo, scene.leftHipPatchRect);
    scene.modelProxyContourLayer.rightHipPatchRect =
        MapRect(scene.bodyRect, bodyTo, scene.rightHipPatchRect);
    scene.modelProxyContourLayer.bellyContourRect =
        MapRect(scene.bodyRect, bodyTo, scene.bellyContourRect);
    scene.modelProxyContourLayer.sternumContourRect =
        MapRect(scene.bodyRect, bodyTo, scene.sternumContourRect);
    scene.modelProxyContourLayer.upperTorsoContourRect =
        MapRect(scene.bodyRect, bodyTo, scene.upperTorsoContourRect);
    scene.modelProxyContourLayer.leftTorsoCadenceBridgeRect =
        MapRect(scene.bodyRect, bodyTo, scene.leftTorsoCadenceBridgeRect);
    scene.modelProxyContourLayer.rightTorsoCadenceBridgeRect =
        MapRect(scene.bodyRect, bodyTo, scene.rightTorsoCadenceBridgeRect);
    scene.modelProxyContourLayer.leftBackContourRect =
        MapRect(scene.bodyRect, bodyTo, scene.leftBackContourRect);
    scene.modelProxyContourLayer.rightBackContourRect =
        MapRect(scene.bodyRect, bodyTo, scene.rightBackContourRect);
    scene.modelProxyContourLayer.leftFlankContourRect =
        MapRect(scene.bodyRect, bodyTo, scene.leftFlankContourRect);
    scene.modelProxyContourLayer.rightFlankContourRect =
        MapRect(scene.bodyRect, bodyTo, scene.rightFlankContourRect);

    const float dominanceMix = 0.62f + scene.proxyDominance * 0.38f;
    scene.modelProxyContourLayer.rearAlphaScale = 0.92f + 0.18f * dominanceMix;
    scene.modelProxyContourLayer.strokeAlphaScale = 0.90f + 0.16f * dominanceMix;

    scene.previewBodyAlphaScale *= std::max(0.16f, 0.42f - scene.proxyDominance * 0.12f);
    scene.previewHeadAlphaScale *= std::max(0.20f, 0.48f - scene.proxyDominance * 0.12f);
}

} // namespace mousefx::windows
