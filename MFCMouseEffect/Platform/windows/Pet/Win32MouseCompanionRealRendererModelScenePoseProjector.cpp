#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelScenePoseProjector.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

void TranslatePoint(Gdiplus::PointF& point, float dx, float dy) {
    point.X += dx;
    point.Y += dy;
}

void TranslateRect(Gdiplus::RectF& rect, float dx, float dy) {
    rect.X += dx;
    rect.Y += dy;
}

template <size_t N>
void TranslatePointArray(std::array<Gdiplus::PointF, N>& points, float dx, float dy) {
    for (auto& point : points) {
        TranslatePoint(point, dx, dy);
    }
}

void TranslateBodyCluster(Win32MouseCompanionRealRendererScene& scene, float dx, float dy) {
    TranslatePoint(scene.bodyAnchor, dx, dy);
    TranslateRect(scene.bodyRect, dx, dy);
    TranslateRect(scene.chestRect, dx, dy);
    TranslateRect(scene.neckBridgeRect, dx, dy);
    TranslateRect(scene.leftHeadShoulderBridgeRect, dx, dy);
    TranslateRect(scene.rightHeadShoulderBridgeRect, dx, dy);
    TranslateRect(scene.leftShoulderPatchRect, dx, dy);
    TranslateRect(scene.rightShoulderPatchRect, dx, dy);
    TranslateRect(scene.leftHipPatchRect, dx, dy);
    TranslateRect(scene.rightHipPatchRect, dx, dy);
    TranslateRect(scene.bellyContourRect, dx, dy);
    TranslateRect(scene.sternumContourRect, dx, dy);
    TranslateRect(scene.upperTorsoContourRect, dx, dy);
    TranslateRect(scene.leftTorsoCadenceBridgeRect, dx, dy);
    TranslateRect(scene.rightTorsoCadenceBridgeRect, dx, dy);
    TranslateRect(scene.leftBackContourRect, dx, dy);
    TranslateRect(scene.rightBackContourRect, dx, dy);
    TranslateRect(scene.leftFlankContourRect, dx, dy);
    TranslateRect(scene.rightFlankContourRect, dx, dy);
    TranslateRect(scene.leftTailHaunchBridgeRect, dx, dy);
    TranslateRect(scene.rightTailHaunchBridgeRect, dx, dy);
}

void TranslateHeadCluster(Win32MouseCompanionRealRendererScene& scene, float dx, float dy) {
    TranslatePoint(scene.headAnchor, dx, dy);
    TranslateRect(scene.headRect, dx, dy);
    TranslateRect(scene.leftEarRootCuffRect, dx, dy);
    TranslateRect(scene.rightEarRootCuffRect, dx, dy);
    TranslateRect(scene.leftEarOcclusionCapRect, dx, dy);
    TranslateRect(scene.rightEarOcclusionCapRect, dx, dy);
    TranslatePointArray(scene.leftEar, dx, dy);
    TranslatePointArray(scene.rightEar, dx, dy);
    TranslateRect(scene.leftEyeRect, dx, dy);
    TranslateRect(scene.rightEyeRect, dx, dy);
    TranslateRect(scene.leftPupilRect, dx, dy);
    TranslateRect(scene.rightPupilRect, dx, dy);
    TranslateRect(scene.leftEyeHighlightRect, dx, dy);
    TranslateRect(scene.rightEyeHighlightRect, dx, dy);
    TranslatePointArray(scene.leftWhiskerStart, dx, dy);
    TranslatePointArray(scene.leftWhiskerEnd, dx, dy);
    TranslatePointArray(scene.rightWhiskerStart, dx, dy);
    TranslatePointArray(scene.rightWhiskerEnd, dx, dy);
    TranslatePoint(scene.leftBrowStart, dx, dy);
    TranslatePoint(scene.leftBrowEnd, dx, dy);
    TranslatePoint(scene.rightBrowStart, dx, dy);
    TranslatePoint(scene.rightBrowEnd, dx, dy);
    TranslateRect(scene.mouthRect, dx, dy);
    TranslateRect(scene.noseRect, dx, dy);
    TranslateRect(scene.leftBlushRect, dx, dy);
    TranslateRect(scene.rightBlushRect, dx, dy);
    TranslateRect(scene.leftCheekContourRect, dx, dy);
    TranslateRect(scene.rightCheekContourRect, dx, dy);
    TranslateRect(scene.jawContourRect, dx, dy);
    TranslateRect(scene.muzzlePadRect, dx, dy);
    TranslateRect(scene.foreheadPadRect, dx, dy);
    TranslateRect(scene.crownPadRect, dx, dy);
    TranslateRect(scene.leftParietalBridgeRect, dx, dy);
    TranslateRect(scene.rightParietalBridgeRect, dx, dy);
    TranslateRect(scene.leftEarSkullBridgeRect, dx, dy);
    TranslateRect(scene.rightEarSkullBridgeRect, dx, dy);
    TranslateRect(scene.leftOccipitalContourRect, dx, dy);
    TranslateRect(scene.rightOccipitalContourRect, dx, dy);
    TranslateRect(scene.leftTempleContourRect, dx, dy);
    TranslateRect(scene.rightTempleContourRect, dx, dy);
    TranslateRect(scene.leftUnderEyeContourRect, dx, dy);
    TranslateRect(scene.rightUnderEyeContourRect, dx, dy);
    TranslateRect(scene.noseBridgeRect, dx, dy);
    TranslateRect(scene.poseBadgeRect, dx, dy);
}

void TranslateAppendageCluster(Win32MouseCompanionRealRendererScene& scene, float dx, float dy) {
    TranslatePoint(scene.appendageAnchor, dx, dy);
    TranslateRect(scene.tailRect, dx, dy);
    TranslateRect(scene.tailRootCuffRect, dx, dy);
    TranslateRect(scene.tailBridgeRect, dx, dy);
    TranslateRect(scene.tailMidContourRect, dx, dy);
    TranslateRect(scene.tailTipBridgeRect, dx, dy);
    TranslateRect(scene.tailTipRect, dx, dy);
    TranslateRect(scene.leftHandRect, dx, dy);
    TranslateRect(scene.rightHandRect, dx, dy);
    TranslateRect(scene.leftLegRect, dx, dy);
    TranslateRect(scene.rightLegRect, dx, dy);
    TranslateRect(scene.leftHandRootCuffRect, dx, dy);
    TranslateRect(scene.rightHandRootCuffRect, dx, dy);
    TranslateRect(scene.leftLegRootCuffRect, dx, dy);
    TranslateRect(scene.rightLegRootCuffRect, dx, dy);
    TranslateRect(scene.leftHandSilhouetteBridgeRect, dx, dy);
    TranslateRect(scene.rightHandSilhouetteBridgeRect, dx, dy);
    TranslateRect(scene.leftLegSilhouetteBridgeRect, dx, dy);
    TranslateRect(scene.rightLegSilhouetteBridgeRect, dx, dy);
    TranslateRect(scene.leftHandCadenceBridgeRect, dx, dy);
    TranslateRect(scene.rightHandCadenceBridgeRect, dx, dy);
    TranslateRect(scene.leftLegCadenceBridgeRect, dx, dy);
    TranslateRect(scene.rightLegCadenceBridgeRect, dx, dy);
    TranslateRect(scene.leftHandPadRect, dx, dy);
    TranslateRect(scene.rightHandPadRect, dx, dy);
    TranslateRect(scene.leftLegPadRect, dx, dy);
    TranslateRect(scene.rightLegPadRect, dx, dy);
    TranslateRect(scene.accessoryBounds, dx, dy);
    TranslatePointArray(scene.accessoryStar, dx, dy);
    TranslatePointArray(scene.accessoryMoon, dx, dy);
    TranslateRect(scene.accessoryMoonInsetRect, dx, dy);
    TranslatePointArray(scene.accessoryLeaf, dx, dy);
    TranslatePoint(scene.accessoryLeafVeinStart, dx, dy);
    TranslatePoint(scene.accessoryLeafVeinEnd, dx, dy);
    TranslatePointArray(scene.accessoryRibbonLeft, dx, dy);
    TranslatePointArray(scene.accessoryRibbonRight, dx, dy);
    TranslateRect(scene.accessoryRibbonCenter, dx, dy);
    TranslatePoint(scene.accessoryRibbonLeftFoldStart, dx, dy);
    TranslatePoint(scene.accessoryRibbonLeftFoldEnd, dx, dy);
    TranslatePoint(scene.accessoryRibbonRightFoldStart, dx, dy);
    TranslatePoint(scene.accessoryRibbonRightFoldEnd, dx, dy);
}

void TranslateOverlayCluster(Win32MouseCompanionRealRendererScene& scene, float dx, float dy) {
    TranslatePoint(scene.overlayAnchor, dx, dy);
    TranslateRect(scene.actionOverlay.clickRingRect, dx, dy);
    TranslateRect(scene.actionOverlay.holdBandRect, dx, dy);
    TranslateRect(scene.actionOverlay.scrollArcRect, dx, dy);
    TranslatePoint(scene.actionOverlay.dragLineStart, dx, dy);
    TranslatePoint(scene.actionOverlay.dragLineEnd, dx, dy);
    for (auto& rect : scene.actionOverlay.followTrailRects) {
        TranslateRect(rect, dx, dy);
    }
}

void TranslateGroundingCluster(Win32MouseCompanionRealRendererScene& scene, float dx, float dy) {
    TranslatePoint(scene.groundingAnchor, dx, dy);
    TranslateRect(scene.shadowRect, dx, dy);
    TranslateRect(scene.pedestalRect, dx, dy);
    TranslateRect(scene.glowRect, dx, dy);
    for (auto& rect : scene.laneBadgeRects) {
        TranslateRect(rect, dx, dy);
    }
}

void UpdateGraphLinkStart(
    const char* logicalNode,
    const Gdiplus::PointF& anchor,
    std::vector<Win32MouseCompanionRealRendererSceneGraphLink>& links) {
    for (auto& link : links) {
        if (link.logicalNode == logicalNode) {
            link.start = anchor;
        }
    }
}

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
        TranslateBodyCluster(scene, dx, dy);
        UpdateGraphLinkStart("body", scene.bodyAnchor, scene.modelSceneGraphLinks);
        return;
    }
    if (entry.logicalNode == "head") {
        TranslateHeadCluster(scene, dx, dy);
        UpdateGraphLinkStart("head", scene.headAnchor, scene.modelSceneGraphLinks);
        return;
    }
    if (entry.logicalNode == "appendage") {
        TranslateAppendageCluster(scene, dx, dy);
        UpdateGraphLinkStart("appendage", scene.appendageAnchor, scene.modelSceneGraphLinks);
        return;
    }
    if (entry.logicalNode == "overlay") {
        TranslateOverlayCluster(scene, dx, dy);
        UpdateGraphLinkStart("overlay", scene.overlayAnchor, scene.modelSceneGraphLinks);
        return;
    }
    if (entry.logicalNode == "grounding") {
        TranslateGroundingCluster(scene, dx, dy);
        UpdateGraphLinkStart("grounding", scene.groundingAnchor, scene.modelSceneGraphLinks);
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
