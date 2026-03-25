#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeRigChannelEntry final {
    std::string logicalNode;
    std::string rigHintName;
    std::string rigChannelName;
    float channelWeight{0.0f};
    float amplitudeBias{0.0f};
    float responseBias{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeRigChannelProfile final {
    std::string channelState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeRigChannelEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeRigChannelEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeRigChannelEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeRigChannelEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeRigChannelEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string channelBrief{
        "body:rig.channel.body.spine|head:rig.channel.head.look|appendage:rig.channel.appendage.reach|overlay:rig.channel.overlay.fx|grounding:rig.channel.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeRigChannelProfile
BuildWin32MouseCompanionRealRendererAssetNodeRigChannelProfile(
    const Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile& controlRigHintProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeRigChannelProfile(
    const Win32MouseCompanionRealRendererAssetNodeRigChannelProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
