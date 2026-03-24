#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelScenePoseProjector.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelSceneClusterTransform.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

namespace mousefx::windows {
namespace {

void ApplyEntryProjection(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry& entry,
    float baseAnchorX,
    float baseAnchorY,
    float projectionMix,
    Win32MouseCompanionRealRendererScene& scene) {
    if (!entry.resolved) {
        return;
    }
    const float dx = (entry.worldX - baseAnchorX) * projectionMix;
    const float dy = (entry.worldY - baseAnchorY) * projectionMix;
    if (dx == 0.0f && dy == 0.0f) {
        return;
    }
    if (entry.logicalNode == "body") {
        TranslateWin32MouseCompanionRealRendererBodyCluster(scene, dx, dy);
        UpdateWin32MouseCompanionRealRendererGraphLinkStart("body", scene.bodyAnchor, scene);
        return;
    }
    if (entry.logicalNode == "head") {
        TranslateWin32MouseCompanionRealRendererHeadCluster(scene, dx, dy);
        UpdateWin32MouseCompanionRealRendererGraphLinkStart("head", scene.headAnchor, scene);
        return;
    }
    if (entry.logicalNode == "appendage") {
        TranslateWin32MouseCompanionRealRendererAppendageCluster(scene, dx, dy);
        UpdateWin32MouseCompanionRealRendererGraphLinkStart(
            "appendage",
            scene.appendageAnchor,
            scene);
        return;
    }
    if (entry.logicalNode == "overlay") {
        TranslateWin32MouseCompanionRealRendererOverlayCluster(scene, dx, dy);
        UpdateWin32MouseCompanionRealRendererGraphLinkStart(
            "overlay",
            scene.overlayAnchor,
            scene);
        return;
    }
    if (entry.logicalNode == "grounding") {
        TranslateWin32MouseCompanionRealRendererGroundingCluster(scene, dx, dy);
        UpdateWin32MouseCompanionRealRendererGraphLinkStart(
            "grounding",
            scene.groundingAnchor,
            scene);
    }
}

} // namespace

void ApplyWin32MouseCompanionRealRendererModelScenePoseProjector(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile& worldSpaceProfile,
    Win32MouseCompanionRealRendererScene& scene) {
    const Gdiplus::PointF bodyAnchor = scene.bodyAnchor;
    const Gdiplus::PointF headAnchor = scene.headAnchor;
    const Gdiplus::PointF appendageAnchor = scene.appendageAnchor;
    const Gdiplus::PointF overlayAnchor = scene.overlayAnchor;
    const Gdiplus::PointF groundingAnchor = scene.groundingAnchor;

    ApplyEntryProjection(worldSpaceProfile.bodyEntry, bodyAnchor.X, bodyAnchor.Y, 0.18f, scene);
    ApplyEntryProjection(worldSpaceProfile.headEntry, headAnchor.X, headAnchor.Y, 0.34f, scene);
    ApplyEntryProjection(
        worldSpaceProfile.appendageEntry,
        appendageAnchor.X,
        appendageAnchor.Y,
        0.28f,
        scene);
    ApplyEntryProjection(
        worldSpaceProfile.overlayEntry,
        overlayAnchor.X,
        overlayAnchor.Y,
        0.32f,
        scene);
    ApplyEntryProjection(
        worldSpaceProfile.groundingEntry,
        groundingAnchor.X,
        groundingAnchor.Y,
        0.22f,
        scene);
}

} // namespace mousefx::windows
