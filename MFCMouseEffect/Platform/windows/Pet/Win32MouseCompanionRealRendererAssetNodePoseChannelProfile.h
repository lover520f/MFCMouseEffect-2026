#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile;

struct Win32MouseCompanionRealRendererAssetNodePoseChannelEntry final {
    std::string logicalNode;
    std::string poseNodeName;
    std::string channelName;
    float channelWeight{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodePoseChannelProfile final {
    std::string channelState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodePoseChannelEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseChannelEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseChannelEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseChannelEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodePoseChannelEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string channelBrief{
        "body:channel.body.posture|head:channel.head.expression|appendage:channel.appendage.motion|overlay:channel.overlay.fx|grounding:channel.grounding.shadow"};
    std::string weightBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
};

Win32MouseCompanionRealRendererAssetNodePoseChannelProfile
BuildWin32MouseCompanionRealRendererAssetNodePoseChannelProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile& registryProfile);

} // namespace mousefx::windows
