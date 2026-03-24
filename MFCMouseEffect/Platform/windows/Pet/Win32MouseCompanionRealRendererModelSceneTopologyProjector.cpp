#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelSceneTopologyProjector.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelSceneClusterTransform.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cmath>

namespace mousefx::windows {
namespace {

bool HasResolvedAnchor(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry& entry) {
    return entry.resolved && entry.matchConfidence > 0.0f;
}

float ResolveProjectionMix(const Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry& entry) {
    return std::clamp(0.10f + entry.matchConfidence * 0.18f, 0.0f, 0.32f);
}

void ApplyRelativeProjection(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry& bodyEntry,
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry& targetEntry,
    const Gdiplus::PointF& baseBodyAnchor,
    const Gdiplus::PointF& currentAnchor,
    void (*translateCluster)(Win32MouseCompanionRealRendererScene&, float, float),
    const char* logicalNode,
    Win32MouseCompanionRealRendererScene& scene) {
    if (!HasResolvedAnchor(bodyEntry) || !HasResolvedAnchor(targetEntry)) {
        return;
    }

    const Gdiplus::PointF desiredAnchor(
        baseBodyAnchor.X + (targetEntry.worldX - bodyEntry.worldX),
        baseBodyAnchor.Y + (targetEntry.worldY - bodyEntry.worldY));
    const float mix = ResolveProjectionMix(targetEntry);
    const float dx = (desiredAnchor.X - currentAnchor.X) * mix;
    const float dy = (desiredAnchor.Y - currentAnchor.Y) * mix;
    if (dx == 0.0f && dy == 0.0f) {
        return;
    }

    translateCluster(scene, dx, dy);
    if (logicalNode != nullptr) {
        if (std::string(logicalNode) == "head") {
            UpdateWin32MouseCompanionRealRendererGraphLinkStart("head", scene.headAnchor, scene);
        } else if (std::string(logicalNode) == "appendage") {
            UpdateWin32MouseCompanionRealRendererGraphLinkStart(
                "appendage",
                scene.appendageAnchor,
                scene);
        } else if (std::string(logicalNode) == "overlay") {
            UpdateWin32MouseCompanionRealRendererGraphLinkStart("overlay", scene.overlayAnchor, scene);
        } else if (std::string(logicalNode) == "grounding") {
            UpdateWin32MouseCompanionRealRendererGraphLinkStart(
                "grounding",
                scene.groundingAnchor,
                scene);
        }
    }
}

} // namespace

void ApplyWin32MouseCompanionRealRendererModelSceneTopologyProjector(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile& worldSpaceProfile,
    Win32MouseCompanionRealRendererScene& scene) {
    const auto& body = worldSpaceProfile.bodyEntry;
    if (!HasResolvedAnchor(body)) {
        return;
    }

    const Gdiplus::PointF baseBodyAnchor = scene.bodyAnchor;
    ApplyRelativeProjection(
        body,
        worldSpaceProfile.headEntry,
        baseBodyAnchor,
        scene.headAnchor,
        &TranslateWin32MouseCompanionRealRendererHeadCluster,
        "head",
        scene);
    ApplyRelativeProjection(
        body,
        worldSpaceProfile.appendageEntry,
        baseBodyAnchor,
        scene.appendageAnchor,
        &TranslateWin32MouseCompanionRealRendererAppendageCluster,
        "appendage",
        scene);
    ApplyRelativeProjection(
        body,
        worldSpaceProfile.overlayEntry,
        baseBodyAnchor,
        scene.overlayAnchor,
        &TranslateWin32MouseCompanionRealRendererOverlayCluster,
        "overlay",
        scene);
    ApplyRelativeProjection(
        body,
        worldSpaceProfile.groundingEntry,
        baseBodyAnchor,
        scene.groundingAnchor,
        &TranslateWin32MouseCompanionRealRendererGroundingCluster,
        "grounding",
        scene);

    if (HasResolvedAnchor(worldSpaceProfile.headEntry)) {
        const float dx = worldSpaceProfile.headEntry.worldX - body.worldX;
        const float dy = worldSpaceProfile.headEntry.worldY - body.worldY;
        const float desiredTiltDeg =
            std::clamp(std::atan2(dy, std::max(1.0f, std::fabs(dx))) * 180.0f / 3.1415926f, -20.0f, 20.0f);
        scene.bodyTiltDeg = scene.bodyTiltDeg * 0.82f + desiredTiltDeg * 0.18f;
    }
}

} // namespace mousefx::windows
