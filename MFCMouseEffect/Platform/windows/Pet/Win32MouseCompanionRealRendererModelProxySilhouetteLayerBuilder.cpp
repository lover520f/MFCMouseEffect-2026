#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxySilhouetteLayerBuilder.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

using WorldSpaceEntry = Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry;

Gdiplus::Color ResolveSilhouetteColor(const std::string& logicalNode) {
    if (logicalNode == "body") {
        return Gdiplus::Color(118, 142, 214, 198);
    }
    if (logicalNode == "head") {
        return Gdiplus::Color(124, 226, 208, 166);
    }
    if (logicalNode == "appendage") {
        return Gdiplus::Color(112, 240, 180, 156);
    }
    if (logicalNode == "overlay") {
        return Gdiplus::Color(126, 154, 204, 255);
    }
    if (logicalNode == "grounding") {
        return Gdiplus::Color(110, 208, 188, 146);
    }
    return Gdiplus::Color(108, 184, 198, 226);
}

Gdiplus::RectF ResolveBaseBounds(
    const std::string& logicalNode,
    const Win32MouseCompanionRealRendererScene& scene) {
    if (logicalNode == "body") {
        return scene.bodyRect;
    }
    if (logicalNode == "head") {
        return scene.headRect;
    }
    if (logicalNode == "appendage") {
        return scene.tailRect;
    }
    if (logicalNode == "overlay") {
        return Gdiplus::RectF(
            scene.overlayAnchor.X - scene.bodyRect.Width * 0.16f,
            scene.overlayAnchor.Y - scene.bodyRect.Height * 0.12f,
            scene.bodyRect.Width * 0.32f,
            scene.bodyRect.Height * 0.24f);
    }
    return scene.pedestalRect;
}

Gdiplus::RectF BuildSilhouetteBounds(
    const WorldSpaceEntry& entry,
    const Win32MouseCompanionRealRendererScene& scene) {
    const Gdiplus::RectF base = ResolveBaseBounds(entry.logicalNode, scene);
    const float widthScale = entry.logicalNode == "body"   ? 0.92f
        : entry.logicalNode == "head"                      ? 0.86f
        : entry.logicalNode == "appendage"                 ? 0.72f
        : entry.logicalNode == "overlay"                   ? 0.64f
                                                           : 0.82f;
    const float heightScale = entry.logicalNode == "grounding" ? 0.44f : widthScale;
    const float width = std::max(14.0f, base.Width * widthScale * std::max(0.84f, entry.worldScale));
    const float height = std::max(10.0f, base.Height * heightScale * std::max(0.82f, entry.worldScale));
    return Gdiplus::RectF(
        entry.worldX - width * 0.5f,
        entry.worldY - height * 0.5f,
        width,
        height);
}

void AppendSilhouette(
    const WorldSpaceEntry& entry,
    const Win32MouseCompanionRealRendererScene& scene,
    std::vector<Win32MouseCompanionRealRendererModelProxySilhouette>* silhouettes) {
    if (silhouettes == nullptr || !entry.resolved) {
        return;
    }

    silhouettes->push_back(Win32MouseCompanionRealRendererModelProxySilhouette{
        entry.logicalNode,
        BuildSilhouetteBounds(entry, scene),
        ResolveSilhouetteColor(entry.logicalNode),
        std::clamp(82.0f + entry.matchConfidence * 96.0f, 82.0f, 188.0f),
    });
}

} // namespace

void BuildWin32MouseCompanionRealRendererModelProxySilhouetteLayer(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile& worldSpaceProfile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.modelProxySilhouettes.clear();
    scene.modelProxySilhouettes.reserve(5);

    AppendSilhouette(worldSpaceProfile.bodyEntry, scene, &scene.modelProxySilhouettes);
    AppendSilhouette(worldSpaceProfile.headEntry, scene, &scene.modelProxySilhouettes);
    AppendSilhouette(worldSpaceProfile.appendageEntry, scene, &scene.modelProxySilhouettes);
    AppendSilhouette(worldSpaceProfile.overlayEntry, scene, &scene.modelProxySilhouettes);
    AppendSilhouette(worldSpaceProfile.groundingEntry, scene, &scene.modelProxySilhouettes);
}

} // namespace mousefx::windows
