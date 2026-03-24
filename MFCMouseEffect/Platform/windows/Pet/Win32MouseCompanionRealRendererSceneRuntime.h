#pragma once

#include <cstdint>
#include <string>

#include "MouseFx/Core/Control/MouseCompanionPluginV1Types.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelSceneAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeBindingProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeBindingProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeAnchorProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeResolverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeTargetProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeTargetResolverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeTransformProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetBindingTableProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetCatalogProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetDecodeProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetInstanceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetLoadProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetResidencyProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSessionProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetActivationProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetBindReadyProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetHandleProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeBindProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDriveProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDispatchProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeExecuteProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeCommandProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeControllerProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDriverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDriverRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeConsumerProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeConsumerRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeRealizationProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeRealizationRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeMaterializationProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeMaterializationRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodePresentationProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeLiftProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeMountProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeResolveProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeRouteProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSceneBindingProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSceneHookProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeAttachProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetManifestProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSourceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeGraphProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeSlotProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPoseAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererInput.h"

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime final {
    const MouseCompanionPetRuntimeConfig* config{nullptr};
    const Win32MouseCompanionRealRendererAssetResources* assets{nullptr};
    const MouseCompanionPetPoseFrame* poseFrame{nullptr};
    const MouseCompanionPetPoseSample* leftEarPose{nullptr};
    const MouseCompanionPetPoseSample* rightEarPose{nullptr};
    const MouseCompanionPetPoseSample* leftHandPose{nullptr};
    const MouseCompanionPetPoseSample* rightHandPose{nullptr};
    const MouseCompanionPetPoseSample* leftLegPose{nullptr};
    const MouseCompanionPetPoseSample* rightLegPose{nullptr};
    std::string actionName{"idle"};
    std::string reactiveActionName{"idle"};
    float actionIntensity{0.0f};
    float reactiveActionIntensity{0.0f};
    float headTintAmount{0.0f};
    float facingSign{1.0f};
    float facingMomentumPx{0.0f};
    float scrollSignedIntensity{0.0f};
    uint64_t poseSampleTickMs{0};
    uint64_t clickTriggerTickMs{0};
    uint64_t holdTriggerTickMs{0};
    uint64_t scrollTriggerTickMs{0};
    bool follow{false};
    bool drag{false};
    bool hold{false};
    bool scroll{false};
    bool click{false};
    bool poseFrameAvailable{false};
    bool poseBindingConfigured{false};
    std::string sceneRuntimeAdapterMode{"runtime_only"};
    uint32_t sceneRuntimePoseSampleCount{0};
    uint32_t sceneRuntimeBoundPoseSampleCount{0};
    Win32MouseCompanionRealRendererModelAssetSourceProfile modelAssetSourceProfile{};
    Win32MouseCompanionRealRendererModelAssetManifestProfile modelAssetManifestProfile{};
    Win32MouseCompanionRealRendererModelAssetCatalogProfile modelAssetCatalogProfile{};
    Win32MouseCompanionRealRendererModelAssetBindingTableProfile
        modelAssetBindingTableProfile{};
    Win32MouseCompanionRealRendererModelAssetRegistryProfile
        modelAssetRegistryProfile{};
    Win32MouseCompanionRealRendererModelAssetLoadProfile modelAssetLoadProfile{};
    Win32MouseCompanionRealRendererModelAssetDecodeProfile modelAssetDecodeProfile{};
    Win32MouseCompanionRealRendererModelAssetResidencyProfile
        modelAssetResidencyProfile{};
    Win32MouseCompanionRealRendererModelAssetInstanceProfile
        modelAssetInstanceProfile{};
    Win32MouseCompanionRealRendererModelAssetActivationProfile
        modelAssetActivationProfile{};
    Win32MouseCompanionRealRendererModelAssetSessionProfile
        modelAssetSessionProfile{};
    Win32MouseCompanionRealRendererModelAssetBindReadyProfile
        modelAssetBindReadyProfile{};
    Win32MouseCompanionRealRendererModelAssetHandleProfile
        modelAssetHandleProfile{};
    Win32MouseCompanionRealRendererModelSceneAdapterProfile modelSceneAdapterProfile{};
    Win32MouseCompanionRealRendererModelAssetSceneHookProfile
        modelAssetSceneHookProfile{};
    Win32MouseCompanionRealRendererModelAssetSceneBindingProfile
        modelAssetSceneBindingProfile{};
    Win32MouseCompanionRealRendererModelNodeAdapterProfile modelNodeAdapterProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeAttachProfile
        modelAssetNodeAttachProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeLiftProfile
        modelAssetNodeLiftProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeBindProfile
        modelAssetNodeBindProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeResolveProfile
        modelAssetNodeResolveProfile{};
    Win32MouseCompanionRealRendererModelNodeGraphProfile modelNodeGraphProfile{};
    Win32MouseCompanionRealRendererModelNodeBindingProfile modelNodeBindingProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeDriveProfile
        modelAssetNodeDriveProfile{};
    Win32MouseCompanionRealRendererModelNodeSlotProfile modelNodeSlotProfile{};
    Win32MouseCompanionRealRendererModelNodeRegistryProfile modelNodeRegistryProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeMountProfile
        modelAssetNodeMountProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeRouteProfile
        modelAssetNodeRouteProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeDispatchProfile
        modelAssetNodeDispatchProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeExecuteProfile
        modelAssetNodeExecuteProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeCommandProfile
        modelAssetNodeCommandProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeControllerProfile
        modelAssetNodeControllerProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeDriverProfile
        modelAssetNodeDriverProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeDriverRegistryProfile
        modelAssetNodeDriverRegistryProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeConsumerProfile
        modelAssetNodeConsumerProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeConsumerRegistryProfile
        modelAssetNodeConsumerRegistryProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile
        modelAssetNodeProjectionProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile
        modelAssetNodeProjectionRegistryProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeRealizationProfile
        modelAssetNodeRealizationProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeRealizationRegistryProfile
        modelAssetNodeRealizationRegistryProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeMaterializationProfile
        modelAssetNodeMaterializationProfile{};
    Win32MouseCompanionRealRendererModelAssetNodeMaterializationRegistryProfile
        modelAssetNodeMaterializationRegistryProfile{};
    Win32MouseCompanionRealRendererModelAssetNodePresentationProfile
        modelAssetNodePresentationProfile{};
    Win32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile
        modelAssetNodePresentationRegistryProfile{};
    Win32MouseCompanionRealRendererAssetNodeBindingProfile assetNodeBindingProfile{};
    Win32MouseCompanionRealRendererAssetNodeTransformProfile assetNodeTransformProfile{};
    Win32MouseCompanionRealRendererAssetNodeResolverProfile assetNodeResolverProfile{};
    Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile assetNodeParentSpaceProfile{};
    Win32MouseCompanionRealRendererAssetNodeTargetProfile assetNodeTargetProfile{};
    Win32MouseCompanionRealRendererAssetNodeTargetResolverProfile
        assetNodeTargetResolverProfile{};
    Win32MouseCompanionRealRendererAssetNodeAnchorProfile assetNodeAnchorProfile{};
    Win32MouseCompanionRealRendererPoseAdapterProfile poseAdapterProfile{};
};

Win32MouseCompanionRealRendererSceneRuntime BuildWin32MouseCompanionRealRendererSceneRuntime(
    const Win32MouseCompanionRendererInput& input,
    const Win32MouseCompanionRealRendererAssetResources& assets);
float ResolveWin32MouseCompanionRealRendererPoseSampleCoverage(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);
float ResolveWin32MouseCompanionRealRendererPoseAdapterInfluence(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);
float ResolveWin32MouseCompanionRealRendererPoseAdapterReadabilityBias(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);
std::string BuildWin32MouseCompanionRealRendererPoseAdapterBrief(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
