#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyLayerBuilder.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

Gdiplus::Color ResolveProxyColor(const std::string& logicalNode) {
    if (logicalNode == "body") {
        return Gdiplus::Color(160, 130, 220, 184);
    }
    if (logicalNode == "head") {
        return Gdiplus::Color(170, 246, 204, 140);
    }
    if (logicalNode == "appendage") {
        return Gdiplus::Color(164, 255, 182, 128);
    }
    if (logicalNode == "overlay") {
        return Gdiplus::Color(176, 128, 196, 255);
    }
    if (logicalNode == "grounding") {
        return Gdiplus::Color(158, 220, 188, 120);
    }
    return Gdiplus::Color(148, 170, 170, 170);
}

float ResolveProxyRadius(
    const std::string& logicalNode,
    const Win32MouseCompanionRealRendererScene& scene,
    float scale) {
    const float baseRadius = logicalNode == "body"   ? scene.bodyRect.Width * 0.16f
        : logicalNode == "head"                      ? scene.headRect.Width * 0.14f
        : logicalNode == "appendage"                 ? scene.bodyRect.Width * 0.11f
        : logicalNode == "overlay"                   ? scene.bodyRect.Width * 0.10f
                                                     : scene.bodyRect.Width * 0.12f;
    return std::max(6.0f, baseRadius * std::max(0.82f, scale));
}

void AppendProxyNode(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry& entry,
    const Win32MouseCompanionRealRendererScene& scene,
    std::vector<Win32MouseCompanionRealRendererModelProxyNode>* nodes) {
    if (nodes == nullptr || !entry.resolved) {
        return;
    }
    const float radius = ResolveProxyRadius(entry.logicalNode, scene, entry.worldScale);
    nodes->push_back(Win32MouseCompanionRealRendererModelProxyNode{
        entry.logicalNode,
        Gdiplus::RectF(
            entry.worldX - radius,
            entry.worldY - radius,
            radius * 2.0f,
            radius * 2.0f),
        ResolveProxyColor(entry.logicalNode),
        std::clamp(96.0f + entry.matchConfidence * 92.0f, 96.0f, 216.0f),
        true,
    });
}

void AppendProxyLink(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry& from,
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry& to,
    std::vector<Win32MouseCompanionRealRendererModelProxyLink>* links) {
    if (links == nullptr || !from.resolved || !to.resolved) {
        return;
    }
    links->push_back(Win32MouseCompanionRealRendererModelProxyLink{
        to.logicalNode,
        Gdiplus::PointF(from.worldX, from.worldY),
        Gdiplus::PointF(to.worldX, to.worldY),
        ResolveProxyColor(to.logicalNode),
        std::clamp(80.0f + to.matchConfidence * 88.0f, 80.0f, 196.0f),
    });
}

} // namespace

void BuildWin32MouseCompanionRealRendererModelProxyLayer(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile& worldSpaceProfile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.modelProxyVisible = false;
    scene.modelProxyNodes.clear();
    scene.modelProxyLinks.clear();

    scene.modelProxyNodes.reserve(5);
    AppendProxyNode(worldSpaceProfile.bodyEntry, scene, &scene.modelProxyNodes);
    AppendProxyNode(worldSpaceProfile.headEntry, scene, &scene.modelProxyNodes);
    AppendProxyNode(worldSpaceProfile.appendageEntry, scene, &scene.modelProxyNodes);
    AppendProxyNode(worldSpaceProfile.overlayEntry, scene, &scene.modelProxyNodes);
    AppendProxyNode(worldSpaceProfile.groundingEntry, scene, &scene.modelProxyNodes);

    scene.modelProxyLinks.reserve(4);
    AppendProxyLink(worldSpaceProfile.bodyEntry, worldSpaceProfile.headEntry, &scene.modelProxyLinks);
    AppendProxyLink(worldSpaceProfile.bodyEntry, worldSpaceProfile.appendageEntry, &scene.modelProxyLinks);
    AppendProxyLink(worldSpaceProfile.bodyEntry, worldSpaceProfile.overlayEntry, &scene.modelProxyLinks);
    AppendProxyLink(worldSpaceProfile.bodyEntry, worldSpaceProfile.groundingEntry, &scene.modelProxyLinks);

    scene.modelProxyVisible = !scene.modelProxyNodes.empty();
}

} // namespace mousefx::windows
