#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginContractLabels.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererBackend.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeAnchorProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseChannelProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseSolveProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeJointHintProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeArticulationProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeRigChannelProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeRigDriverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseBusProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerTableProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeDriverBusProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseResolverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeTargetProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeResolverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererCapabilities.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetBindingTableProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetCatalogProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetDecodeProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetInstanceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetLoadProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetManifestProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetResidencyProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSessionProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetActivationProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetBindReadyProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetHandleProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeAttachProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeBindProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDriveProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDispatchProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeCommandProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeLiftProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeMountProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeExecuteProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeControllerProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDriverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDriverRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeConsumerProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeConsumerRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeMaterializationProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeMaterializationRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodePresentationProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeVisibilityProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeVisibilityRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodePresenceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodePresenceRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeOccupancyProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeOccupancyRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeRealizationProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeRealizationRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeResolveProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeRouteProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSceneBindingProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSceneHookProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSourceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelSceneAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyLayerBuilder.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyFootprintProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyActionOverlayProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyActionLayerProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyActionProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyAppendageActionProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyMotionProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyAdornmentLayerProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyDetailLayerProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyContourLayerProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyOverlaySuppressionProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyPresenceProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyAdornmentProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyAppendageProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyAppendageLayerProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyEarProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyExpressionProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyFaceProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyFrameLayerProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyFrameProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxyPaletteProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxySurfaceBuilder.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelProxySilhouetteLayerBuilder.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelSceneTopologyProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelScenePoseProjector.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPainter.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPoseAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginHost.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneBuilder.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererBackendRegistry.h"

#include <chrono>

namespace mousefx::windows {
namespace {

uint64_t ReadRendererRuntimeTickMs() {
    using Clock = std::chrono::steady_clock;
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now().time_since_epoch())
            .count());
}

} // namespace

bool Win32MouseCompanionRealRendererBackend::Start() {
    lastErrorReason_ = DescribeWin32MouseCompanionRealRendererStartFailure();
    ready_ = lastErrorReason_.empty();
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    runtimeDiagnostics_ = Win32MouseCompanionRendererBackendRuntimeDiagnostics{};
    runtimeDiagnostics_.backendName = "real";
    runtimeDiagnostics_.ready = ready_;
    return ready_;
}

void Win32MouseCompanionRealRendererBackend::Shutdown() {
    ready_ = false;
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    runtimeDiagnostics_.ready = false;
    runtimeDiagnostics_.renderedFrame = false;
}

bool Win32MouseCompanionRealRendererBackend::IsReady() const {
    return ready_;
}

std::string Win32MouseCompanionRealRendererBackend::LastErrorReason() const {
    return lastErrorReason_;
}

void Win32MouseCompanionRealRendererBackend::Render(
    const Win32MouseCompanionRendererInput& input,
    Gdiplus::Graphics* graphics,
    int width,
    int height) const {
    if (!ready_ || !graphics || width <= 0 || height <= 0) {
        return;
    }
    graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    const auto resources = BuildWin32MouseCompanionRealRendererAssetResources(input);
    const auto sceneRuntime = BuildWin32MouseCompanionRealRendererSceneRuntime(input, resources);
    auto scene = BuildWin32MouseCompanionRealRendererScene(sceneRuntime, width, height);
    const auto modelAssetSourceProfile = sceneRuntime.modelAssetSourceProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetSourceProfile(
        modelAssetSourceProfile,
        scene);
    const auto modelAssetManifestProfile = sceneRuntime.modelAssetManifestProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetManifestProfile(
        modelAssetManifestProfile,
        scene);
    const auto modelAssetCatalogProfile = sceneRuntime.modelAssetCatalogProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetCatalogProfile(
        modelAssetCatalogProfile,
        scene);
    const auto modelAssetBindingTableProfile =
        sceneRuntime.modelAssetBindingTableProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetBindingTableProfile(
        modelAssetBindingTableProfile,
        scene);
    const auto modelAssetRegistryProfile =
        sceneRuntime.modelAssetRegistryProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetRegistryProfile(
        modelAssetRegistryProfile,
        scene);
    const auto modelAssetLoadProfile =
        sceneRuntime.modelAssetLoadProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetLoadProfile(
        modelAssetLoadProfile,
        scene);
    const auto modelAssetDecodeProfile =
        sceneRuntime.modelAssetDecodeProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetDecodeProfile(
        modelAssetDecodeProfile,
        scene);
    const auto modelAssetResidencyProfile =
        sceneRuntime.modelAssetResidencyProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetResidencyProfile(
        modelAssetResidencyProfile,
        scene);
    const auto modelAssetInstanceProfile =
        sceneRuntime.modelAssetInstanceProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetInstanceProfile(
        modelAssetInstanceProfile,
        scene);
    const auto modelAssetActivationProfile =
        sceneRuntime.modelAssetActivationProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetActivationProfile(
        modelAssetActivationProfile,
        scene);
    const auto modelAssetSessionProfile =
        sceneRuntime.modelAssetSessionProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetSessionProfile(
        modelAssetSessionProfile,
        scene);
    const auto modelAssetBindReadyProfile =
        sceneRuntime.modelAssetBindReadyProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetBindReadyProfile(
        modelAssetBindReadyProfile,
        scene);
    const auto modelAssetHandleProfile =
        sceneRuntime.modelAssetHandleProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetHandleProfile(
        modelAssetHandleProfile,
        scene);
    const auto modelAssetSceneHookProfile =
        sceneRuntime.modelAssetSceneHookProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetSceneHookProfile(
        modelAssetSceneHookProfile,
        scene);
    const auto modelAssetSceneBindingProfile =
        sceneRuntime.modelAssetSceneBindingProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetSceneBindingProfile(
        modelAssetSceneBindingProfile,
        scene);
    const auto modelAssetNodeAttachProfile =
        sceneRuntime.modelAssetNodeAttachProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeAttachProfile(
        modelAssetNodeAttachProfile,
        scene);
    const auto modelAssetNodeLiftProfile =
        sceneRuntime.modelAssetNodeLiftProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeLiftProfile(
        modelAssetNodeLiftProfile,
        scene);
    const auto modelAssetNodeBindProfile =
        sceneRuntime.modelAssetNodeBindProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeBindProfile(
        modelAssetNodeBindProfile,
        scene);
    const auto modelAssetNodeResolveProfile =
        sceneRuntime.modelAssetNodeResolveProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeResolveProfile(
        modelAssetNodeResolveProfile,
        scene);
    const auto resolverProfile = sceneRuntime.assetNodeResolverProfile;
    const auto parentSpaceProfile = sceneRuntime.assetNodeParentSpaceProfile;
    const auto targetProfile = sceneRuntime.assetNodeTargetProfile;
    const auto targetResolverProfile = sceneRuntime.assetNodeTargetResolverProfile;
    const auto anchorProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeAnchorProfile(sceneRuntime, scene);
    const auto worldSpaceProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeWorldSpaceProfile(sceneRuntime, scene);
    ApplyWin32MouseCompanionRealRendererAssetNodeWorldSpaceProfile(worldSpaceProfile, scene);
    ApplyWin32MouseCompanionRealRendererModelScenePoseProjector(worldSpaceProfile, scene);
    ApplyWin32MouseCompanionRealRendererModelSceneTopologyProjector(worldSpaceProfile, scene);
    BuildWin32MouseCompanionRealRendererModelProxyLayer(worldSpaceProfile, scene);
    BuildWin32MouseCompanionRealRendererModelProxySilhouetteLayer(worldSpaceProfile, scene);
    BuildWin32MouseCompanionRealRendererModelProxySurfaces(worldSpaceProfile, scene);
    ApplyWin32MouseCompanionRealRendererModelProxyFrameProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyFaceProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyEarProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyExpressionProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyFrameLayerProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyContourLayerProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyDetailLayerProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyAppendageLayerProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyAppendageProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyFootprintProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyAdornmentProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyAdornmentLayerProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyActionOverlayProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyActionLayerProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyPaletteProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyPresenceProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyActionProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyAppendageActionProjector(scene);
    ApplyWin32MouseCompanionRealRendererModelProxyMotionProjector(scene);
    const auto poseProfile =
        BuildWin32MouseCompanionRealRendererAssetNodePoseProfile(
            sceneRuntime,
            scene,
            worldSpaceProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodePoseProfile(poseProfile, scene);
    const auto modelAssetNodeDriveProfile =
        sceneRuntime.modelAssetNodeDriveProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeDriveProfile(
        modelAssetNodeDriveProfile,
        scene);
    const auto modelAssetNodeMountProfile =
        sceneRuntime.modelAssetNodeMountProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeMountProfile(
        modelAssetNodeMountProfile,
        scene);
    const auto modelAssetNodeRouteProfile =
        sceneRuntime.modelAssetNodeRouteProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeRouteProfile(
        modelAssetNodeRouteProfile,
        scene);
    const auto modelAssetNodeDispatchProfile =
        sceneRuntime.modelAssetNodeDispatchProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeDispatchProfile(
        modelAssetNodeDispatchProfile,
        scene);
    const auto modelAssetNodeExecuteProfile =
        sceneRuntime.modelAssetNodeExecuteProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeExecuteProfile(
        modelAssetNodeExecuteProfile,
        scene);
    const auto modelAssetNodeCommandProfile =
        sceneRuntime.modelAssetNodeCommandProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeCommandProfile(
        modelAssetNodeCommandProfile,
        scene);
    const auto modelAssetNodeControllerProfile =
        sceneRuntime.modelAssetNodeControllerProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeControllerProfile(
        modelAssetNodeControllerProfile,
        scene);
    const auto modelAssetNodeDriverProfile =
        sceneRuntime.modelAssetNodeDriverProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeDriverProfile(
        modelAssetNodeDriverProfile,
        scene);
    const auto modelAssetNodeDriverRegistryProfile =
        sceneRuntime.modelAssetNodeDriverRegistryProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeDriverRegistryProfile(
        modelAssetNodeDriverRegistryProfile,
        scene);
    const auto modelAssetNodeConsumerProfile =
        sceneRuntime.modelAssetNodeConsumerProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeConsumerProfile(
        modelAssetNodeConsumerProfile,
        scene);
    const auto modelAssetNodeConsumerRegistryProfile =
        sceneRuntime.modelAssetNodeConsumerRegistryProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeConsumerRegistryProfile(
        modelAssetNodeConsumerRegistryProfile,
        scene);
    const auto modelAssetNodeProjectionProfile =
        sceneRuntime.modelAssetNodeProjectionProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeProjectionProfile(
        modelAssetNodeProjectionProfile,
        scene);
    const auto modelAssetNodeProjectionRegistryProfile =
        sceneRuntime.modelAssetNodeProjectionRegistryProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile(
        modelAssetNodeProjectionRegistryProfile,
        scene);
    const auto modelAssetNodeRealizationProfile =
        sceneRuntime.modelAssetNodeRealizationProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeRealizationProfile(
        modelAssetNodeRealizationProfile,
        scene);
    const auto modelAssetNodeRealizationRegistryProfile =
        sceneRuntime.modelAssetNodeRealizationRegistryProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeRealizationRegistryProfile(
        modelAssetNodeRealizationRegistryProfile,
        scene);
    const auto modelAssetNodeMaterializationProfile =
        sceneRuntime.modelAssetNodeMaterializationProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeMaterializationProfile(
        modelAssetNodeMaterializationProfile,
        scene);
    const auto modelAssetNodeMaterializationRegistryProfile =
        sceneRuntime.modelAssetNodeMaterializationRegistryProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeMaterializationRegistryProfile(
        modelAssetNodeMaterializationRegistryProfile,
        scene);
    const auto modelAssetNodePresentationProfile =
        sceneRuntime.modelAssetNodePresentationProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodePresentationProfile(
        modelAssetNodePresentationProfile,
        scene);
    const auto modelAssetNodePresentationRegistryProfile =
        sceneRuntime.modelAssetNodePresentationRegistryProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile(
        modelAssetNodePresentationRegistryProfile,
        scene);
    const auto modelAssetNodeVisibilityProfile =
        sceneRuntime.modelAssetNodeVisibilityProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeVisibilityProfile(
        modelAssetNodeVisibilityProfile,
        scene);
    const auto modelAssetNodeVisibilityRegistryProfile =
        sceneRuntime.modelAssetNodeVisibilityRegistryProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeVisibilityRegistryProfile(
        modelAssetNodeVisibilityRegistryProfile,
        scene);
    const auto modelAssetNodePresenceProfile =
        sceneRuntime.modelAssetNodePresenceProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodePresenceProfile(
        modelAssetNodePresenceProfile,
        scene);
    const auto modelAssetNodePresenceRegistryProfile =
        sceneRuntime.modelAssetNodePresenceRegistryProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodePresenceRegistryProfile(
        modelAssetNodePresenceRegistryProfile,
        scene);
    const auto modelAssetNodeOccupancyProfile =
        sceneRuntime.modelAssetNodeOccupancyProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeOccupancyProfile(
        modelAssetNodeOccupancyProfile,
        scene);
    const auto modelAssetNodeOccupancyRegistryProfile =
        sceneRuntime.modelAssetNodeOccupancyRegistryProfile;
    ApplyWin32MouseCompanionRealRendererModelAssetNodeOccupancyRegistryProfile(
        modelAssetNodeOccupancyRegistryProfile,
        scene);
    const auto poseResolverProfile =
        BuildWin32MouseCompanionRealRendererAssetNodePoseResolverProfile(
            poseProfile);
    const auto poseRegistryProfile =
        BuildWin32MouseCompanionRealRendererAssetNodePoseRegistryProfile(
            poseResolverProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodePoseRegistryProfile(poseRegistryProfile, scene);
    const auto poseChannelProfile =
        BuildWin32MouseCompanionRealRendererAssetNodePoseChannelProfile(
            poseRegistryProfile);
    const auto poseConstraintProfile =
        BuildWin32MouseCompanionRealRendererAssetNodePoseConstraintProfile(
            poseChannelProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodePoseConstraintProfile(
        poseConstraintProfile,
        scene);
    const auto poseSolveProfile =
        BuildWin32MouseCompanionRealRendererAssetNodePoseSolveProfile(
            poseConstraintProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodePoseSolveProfile(
        poseSolveProfile,
        scene);
    const auto jointHintProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeJointHintProfile(
            poseSolveProfile,
            sceneRuntime.assetNodeMatchCatalogProfile,
            sceneRuntime.assetNodeMatchGraphProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeJointHintProfile(
        jointHintProfile,
        scene);
    const auto articulationProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeArticulationProfile(
            jointHintProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeArticulationProfile(
        articulationProfile,
        scene);
    const auto localJointRegistryProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile(
            articulationProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile(
        localJointRegistryProfile,
        scene);
    const auto articulationMapProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeArticulationMapProfile(
            localJointRegistryProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeArticulationMapProfile(
        articulationMapProfile,
        scene);
    const auto controlRigHintProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeControlRigHintProfile(
            articulationMapProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeControlRigHintProfile(
        controlRigHintProfile,
        scene);
    const auto rigChannelProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeRigChannelProfile(
            controlRigHintProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeRigChannelProfile(
        rigChannelProfile,
        scene);
    const auto controlSurfaceProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeControlSurfaceProfile(
            rigChannelProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeControlSurfaceProfile(
        controlSurfaceProfile,
        scene);
    const auto rigDriverProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeRigDriverProfile(
            controlSurfaceProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeRigDriverProfile(
        rigDriverProfile,
        scene);
    const auto surfaceDriverProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile(
            rigDriverProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile(
        surfaceDriverProfile,
        scene);
    const auto poseBusProfile =
        BuildWin32MouseCompanionRealRendererAssetNodePoseBusProfile(
            surfaceDriverProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodePoseBusProfile(
        poseBusProfile,
        scene);
    const auto controllerTableProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeControllerTableProfile(
            poseBusProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeControllerTableProfile(
        controllerTableProfile,
        scene);
    const auto controllerRegistryProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeControllerRegistryProfile(
            controllerTableProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeControllerRegistryProfile(
        controllerRegistryProfile,
        scene);
    const auto driverBusProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeDriverBusProfile(
            controllerRegistryProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeDriverBusProfile(
        driverBusProfile,
        scene);
    const auto controllerDriverRegistryProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile(
            driverBusProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile(
        controllerDriverRegistryProfile,
        scene);
    const auto executionLaneProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeExecutionLaneProfile(
            controllerDriverRegistryProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeExecutionLaneProfile(
        executionLaneProfile,
        scene);
    const auto controllerPhaseProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeControllerPhaseProfile(
            executionLaneProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeControllerPhaseProfile(
        controllerPhaseProfile,
        scene);
    const auto executionSurfaceProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile(
            controllerPhaseProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile(
        executionSurfaceProfile,
        scene);
    const auto controllerPhaseRegistryProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile(
            controllerPhaseProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile(
        controllerPhaseRegistryProfile,
        scene);
    const auto surfaceCompositionBusProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile(
            executionSurfaceProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile(
        surfaceCompositionBusProfile,
        scene);
    const auto executionStackProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeExecutionStackProfile(
            surfaceCompositionBusProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeExecutionStackProfile(
        executionStackProfile,
        scene);
    const auto executionStackRouterProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile(
            executionStackProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile(
        executionStackRouterProfile,
        scene);
    const auto executionStackRouterRegistryProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile(
            executionStackRouterProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile(
        executionStackRouterRegistryProfile,
        scene);
    const auto compositionRegistryProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile(
            surfaceCompositionBusProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile(
        compositionRegistryProfile,
        scene);
    const auto surfaceRouteProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile(
            compositionRegistryProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile(
        surfaceRouteProfile,
        scene);
    const auto surfaceRouteRegistryProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile(
            surfaceRouteProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile(
        surfaceRouteRegistryProfile,
        scene);
    const auto surfaceRouteRouterBusProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile(
            surfaceRouteRegistryProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile(
        surfaceRouteRouterBusProfile,
        scene);
    const auto surfaceRouteBusRegistryProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryProfile(
            surfaceRouteRouterBusProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryProfile(
        surfaceRouteBusRegistryProfile,
        scene);
    const auto surfaceRouteBusDriverProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile(
            surfaceRouteBusRegistryProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile(
        surfaceRouteBusDriverProfile,
        scene);
    const auto surfaceRouteBusDriverRegistryProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile(
            surfaceRouteBusDriverProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile(
        surfaceRouteBusDriverRegistryProfile,
        scene);
    const auto surfaceRouteBusDriverRegistryRouterProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile(
            surfaceRouteBusDriverRegistryProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile(
        surfaceRouteBusDriverRegistryRouterProfile,
        scene);
    const auto executionDriverTableProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile(
            surfaceRouteProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile(
        executionDriverTableProfile,
        scene);
    const auto executionDriverRouterTableProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile(
            executionDriverTableProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile(
        executionDriverRouterTableProfile,
        scene);
    const auto executionDriverRouterRegistryProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile(
            executionDriverRouterTableProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile(
        executionDriverRouterRegistryProfile,
        scene);
    const auto executionDriverRouterRegistryBusProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile(
            executionDriverRouterRegistryProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile(
        executionDriverRouterRegistryBusProfile,
        scene);
    const auto executionDriverRouterRegistryBusRegistryProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile(
            executionDriverRouterRegistryBusProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile(
        executionDriverRouterRegistryBusRegistryProfile,
        scene);
    const auto pluginSelection = ResolveWin32MouseCompanionRenderPluginSelection();
    ApplyWin32MouseCompanionRealRendererModelProxyOverlaySuppressionProjector(scene);
    const Win32MouseCompanionRealRendererPainter painter{};
    painter.Paint(scene, graphics, width, height);

    Win32MouseCompanionRendererBackendRuntimeDiagnostics diagnostics{};
    diagnostics.backendName = "real";
    diagnostics.ready = ready_;
    diagnostics.renderedFrame = true;
    diagnostics.renderedFrameCount = 1;
    diagnostics.lastRenderTickMs = ReadRendererRuntimeTickMs();
    diagnostics.actionName = sceneRuntime.actionName;
    diagnostics.reactiveActionName = sceneRuntime.reactiveActionName;
    diagnostics.actionIntensity = sceneRuntime.actionIntensity;
    diagnostics.reactiveActionIntensity = sceneRuntime.reactiveActionIntensity;
    diagnostics.modelReady = resources.modelReady;
    diagnostics.actionLibraryReady = resources.actionLibraryReady;
    diagnostics.appearanceProfileReady = resources.appearanceProfileReady;
    diagnostics.poseFrameAvailable = sceneRuntime.poseFrameAvailable;
    diagnostics.poseBindingConfigured = sceneRuntime.poseBindingConfigured;
    #if !defined(MFX_SHIPPING_BUILD)
    diagnostics.sceneRuntimeAdapterMode = sceneRuntime.sceneRuntimeAdapterMode;
    diagnostics.sceneRuntimePoseSampleCount = sceneRuntime.sceneRuntimePoseSampleCount;
    diagnostics.sceneRuntimeBoundPoseSampleCount =
        sceneRuntime.sceneRuntimeBoundPoseSampleCount;
    diagnostics.sceneRuntimeModelAssetSourceState =
        modelAssetSourceProfile.sourceState;
    diagnostics.sceneRuntimeModelAssetSourceReadiness =
        modelAssetSourceProfile.sourceReadiness;
    diagnostics.sceneRuntimeModelAssetSourceBrief =
        modelAssetSourceProfile.brief;
    diagnostics.sceneRuntimeModelAssetSourcePathBrief =
        modelAssetSourceProfile.pathBrief;
    diagnostics.sceneRuntimeModelAssetSourceValueBrief =
        modelAssetSourceProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetManifestState =
        modelAssetManifestProfile.manifestState;
    diagnostics.sceneRuntimeModelAssetManifestEntryCount =
        modelAssetManifestProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetManifestResolvedEntryCount =
        modelAssetManifestProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetManifestBrief =
        modelAssetManifestProfile.brief;
    diagnostics.sceneRuntimeModelAssetManifestEntryBrief =
        modelAssetManifestProfile.entryBrief;
    diagnostics.sceneRuntimeModelAssetManifestValueBrief =
        modelAssetManifestProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetCatalogState =
        modelAssetCatalogProfile.catalogState;
    diagnostics.sceneRuntimeModelAssetCatalogEntryCount =
        modelAssetCatalogProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetCatalogResolvedEntryCount =
        modelAssetCatalogProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetCatalogBrief =
        modelAssetCatalogProfile.brief;
    diagnostics.sceneRuntimeModelAssetCatalogEntryBrief =
        modelAssetCatalogProfile.entryBrief;
    diagnostics.sceneRuntimeModelAssetCatalogValueBrief =
        modelAssetCatalogProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetBindingTableState =
        modelAssetBindingTableProfile.bindingState;
    diagnostics.sceneRuntimeModelAssetBindingTableEntryCount =
        modelAssetBindingTableProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetBindingTableResolvedEntryCount =
        modelAssetBindingTableProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetBindingTableBrief =
        modelAssetBindingTableProfile.brief;
    diagnostics.sceneRuntimeModelAssetBindingTableSlotBrief =
        modelAssetBindingTableProfile.slotBrief;
    diagnostics.sceneRuntimeModelAssetBindingTableValueBrief =
        modelAssetBindingTableProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetRegistryState =
        modelAssetRegistryProfile.registryState;
    diagnostics.sceneRuntimeModelAssetRegistryEntryCount =
        modelAssetRegistryProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetRegistryResolvedEntryCount =
        modelAssetRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetRegistryBrief =
        modelAssetRegistryProfile.brief;
    diagnostics.sceneRuntimeModelAssetRegistryAssetBrief =
        modelAssetRegistryProfile.assetBrief;
    diagnostics.sceneRuntimeModelAssetRegistryValueBrief =
        modelAssetRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetLoadState =
        modelAssetLoadProfile.loadState;
    diagnostics.sceneRuntimeModelAssetLoadEntryCount =
        modelAssetLoadProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetLoadResolvedEntryCount =
        modelAssetLoadProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetLoadBrief =
        modelAssetLoadProfile.brief;
    diagnostics.sceneRuntimeModelAssetLoadPlanBrief =
        modelAssetLoadProfile.planBrief;
    diagnostics.sceneRuntimeModelAssetLoadValueBrief =
        modelAssetLoadProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetDecodeState =
        modelAssetDecodeProfile.decodeState;
    diagnostics.sceneRuntimeModelAssetDecodeEntryCount =
        modelAssetDecodeProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetDecodeResolvedEntryCount =
        modelAssetDecodeProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetDecodeBrief =
        modelAssetDecodeProfile.brief;
    diagnostics.sceneRuntimeModelAssetDecodePipelineBrief =
        modelAssetDecodeProfile.pipelineBrief;
    diagnostics.sceneRuntimeModelAssetDecodeValueBrief =
        modelAssetDecodeProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetResidencyState =
        modelAssetResidencyProfile.residencyState;
    diagnostics.sceneRuntimeModelAssetResidencyEntryCount =
        modelAssetResidencyProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetResidencyResolvedEntryCount =
        modelAssetResidencyProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetResidencyBrief =
        modelAssetResidencyProfile.brief;
    diagnostics.sceneRuntimeModelAssetResidencyCacheBrief =
        modelAssetResidencyProfile.cacheBrief;
    diagnostics.sceneRuntimeModelAssetResidencyValueBrief =
        modelAssetResidencyProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetInstanceState =
        modelAssetInstanceProfile.instanceState;
    diagnostics.sceneRuntimeModelAssetInstanceEntryCount =
        modelAssetInstanceProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetInstanceResolvedEntryCount =
        modelAssetInstanceProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetInstanceBrief =
        modelAssetInstanceProfile.brief;
    diagnostics.sceneRuntimeModelAssetInstanceSlotBrief =
        modelAssetInstanceProfile.slotBrief;
    diagnostics.sceneRuntimeModelAssetInstanceValueBrief =
        modelAssetInstanceProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetActivationState =
        modelAssetActivationProfile.activationState;
    diagnostics.sceneRuntimeModelAssetActivationEntryCount =
        modelAssetActivationProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetActivationResolvedEntryCount =
        modelAssetActivationProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetActivationBrief =
        modelAssetActivationProfile.brief;
    diagnostics.sceneRuntimeModelAssetActivationRouteBrief =
        modelAssetActivationProfile.routeBrief;
    diagnostics.sceneRuntimeModelAssetActivationValueBrief =
        modelAssetActivationProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetSessionState =
        modelAssetSessionProfile.sessionState;
    diagnostics.sceneRuntimeModelAssetSessionEntryCount =
        modelAssetSessionProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetSessionResolvedEntryCount =
        modelAssetSessionProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetSessionBrief =
        modelAssetSessionProfile.brief;
    diagnostics.sceneRuntimeModelAssetSessionSessionBrief =
        modelAssetSessionProfile.sessionBrief;
    diagnostics.sceneRuntimeModelAssetSessionValueBrief =
        modelAssetSessionProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetBindReadyState =
        modelAssetBindReadyProfile.bindReadyState;
    diagnostics.sceneRuntimeModelAssetBindReadyEntryCount =
        modelAssetBindReadyProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetBindReadyResolvedEntryCount =
        modelAssetBindReadyProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetBindReadyBrief =
        modelAssetBindReadyProfile.brief;
    diagnostics.sceneRuntimeModelAssetBindReadyBindingBrief =
        modelAssetBindReadyProfile.bindingBrief;
    diagnostics.sceneRuntimeModelAssetBindReadyValueBrief =
        modelAssetBindReadyProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetHandleState =
        modelAssetHandleProfile.handleState;
    diagnostics.sceneRuntimeModelAssetHandleEntryCount =
        modelAssetHandleProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetHandleResolvedEntryCount =
        modelAssetHandleProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetHandleBrief =
        modelAssetHandleProfile.brief;
    diagnostics.sceneRuntimeModelAssetHandleHandleBrief =
        modelAssetHandleProfile.handleBrief;
    diagnostics.sceneRuntimeModelAssetHandleValueBrief =
        modelAssetHandleProfile.valueBrief;
    diagnostics.sceneRuntimeModelSceneAdapterState =
        sceneRuntime.modelSceneAdapterProfile.seamState;
    diagnostics.sceneRuntimeModelSceneSeamReadiness =
        sceneRuntime.modelSceneAdapterProfile.seamReadiness;
    diagnostics.sceneRuntimeModelSceneAdapterBrief =
        sceneRuntime.modelSceneAdapterProfile.brief;
    diagnostics.sceneRuntimeModelAssetSceneHookState =
        modelAssetSceneHookProfile.hookState;
    diagnostics.sceneRuntimeModelAssetSceneHookEntryCount =
        modelAssetSceneHookProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetSceneHookResolvedEntryCount =
        modelAssetSceneHookProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetSceneHookBrief =
        modelAssetSceneHookProfile.brief;
    diagnostics.sceneRuntimeModelAssetSceneHookHookBrief =
        modelAssetSceneHookProfile.hookBrief;
    diagnostics.sceneRuntimeModelAssetSceneHookValueBrief =
        modelAssetSceneHookProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetSceneBindingState =
        modelAssetSceneBindingProfile.bindingState;
    diagnostics.sceneRuntimeModelAssetSceneBindingEntryCount =
        modelAssetSceneBindingProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetSceneBindingResolvedEntryCount =
        modelAssetSceneBindingProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetSceneBindingBrief =
        modelAssetSceneBindingProfile.brief;
    diagnostics.sceneRuntimeModelAssetSceneBindingBindingBrief =
        modelAssetSceneBindingProfile.bindingBrief;
    diagnostics.sceneRuntimeModelAssetSceneBindingValueBrief =
        modelAssetSceneBindingProfile.valueBrief;
    diagnostics.sceneRuntimeModelNodeAdapterInfluence =
        sceneRuntime.modelNodeAdapterProfile.influence;
    diagnostics.sceneRuntimeModelNodeAdapterBrief =
        sceneRuntime.modelNodeAdapterProfile.brief;
    diagnostics.sceneRuntimeModelNodeChannelBrief =
        sceneRuntime.modelNodeAdapterProfile.channelBrief;
    diagnostics.sceneRuntimeModelAssetNodeAttachState =
        modelAssetNodeAttachProfile.attachState;
    diagnostics.sceneRuntimeModelAssetNodeAttachEntryCount =
        modelAssetNodeAttachProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeAttachResolvedEntryCount =
        modelAssetNodeAttachProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeAttachBrief =
        modelAssetNodeAttachProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeAttachAttachBrief =
        modelAssetNodeAttachProfile.attachBrief;
    diagnostics.sceneRuntimeModelAssetNodeAttachValueBrief =
        modelAssetNodeAttachProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeLiftState =
        modelAssetNodeLiftProfile.liftState;
    diagnostics.sceneRuntimeModelAssetNodeLiftEntryCount =
        modelAssetNodeLiftProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeLiftResolvedEntryCount =
        modelAssetNodeLiftProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeLiftBrief =
        modelAssetNodeLiftProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeLiftLiftBrief =
        modelAssetNodeLiftProfile.liftBrief;
    diagnostics.sceneRuntimeModelAssetNodeLiftValueBrief =
        modelAssetNodeLiftProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeBindState =
        modelAssetNodeBindProfile.bindState;
    diagnostics.sceneRuntimeModelAssetNodeBindEntryCount =
        modelAssetNodeBindProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeBindResolvedEntryCount =
        modelAssetNodeBindProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeBindBrief =
        modelAssetNodeBindProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeBindBindBrief =
        modelAssetNodeBindProfile.bindBrief;
    diagnostics.sceneRuntimeModelAssetNodeBindValueBrief =
        modelAssetNodeBindProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeResolveState =
        modelAssetNodeResolveProfile.resolveState;
    diagnostics.sceneRuntimeModelAssetNodeResolveEntryCount =
        modelAssetNodeResolveProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeResolveResolvedEntryCount =
        modelAssetNodeResolveProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeResolveBrief =
        modelAssetNodeResolveProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeResolveResolveBrief =
        modelAssetNodeResolveProfile.resolveBrief;
    diagnostics.sceneRuntimeModelAssetNodeResolveValueBrief =
        modelAssetNodeResolveProfile.valueBrief;
    diagnostics.sceneRuntimeModelNodeGraphState =
        sceneRuntime.modelNodeGraphProfile.graphState;
    diagnostics.sceneRuntimeModelNodeGraphNodeCount =
        sceneRuntime.modelNodeGraphProfile.nodeCount;
    diagnostics.sceneRuntimeModelNodeGraphBoundNodeCount =
        sceneRuntime.modelNodeGraphProfile.boundNodeCount;
    diagnostics.sceneRuntimeModelNodeGraphBrief =
        sceneRuntime.modelNodeGraphProfile.brief;
    diagnostics.sceneRuntimeModelNodeBindingState =
        sceneRuntime.modelNodeBindingProfile.bindingState;
    diagnostics.sceneRuntimeModelNodeBindingEntryCount =
        sceneRuntime.modelNodeBindingProfile.entryCount;
    diagnostics.sceneRuntimeModelNodeBindingBoundEntryCount =
        sceneRuntime.modelNodeBindingProfile.boundEntryCount;
    diagnostics.sceneRuntimeModelNodeBindingBrief =
        sceneRuntime.modelNodeBindingProfile.brief;
    diagnostics.sceneRuntimeModelNodeBindingWeightBrief =
        sceneRuntime.modelNodeBindingProfile.weightBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriveState =
        modelAssetNodeDriveProfile.driveState;
    diagnostics.sceneRuntimeModelAssetNodeDriveEntryCount =
        modelAssetNodeDriveProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeDriveResolvedEntryCount =
        modelAssetNodeDriveProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeDriveBrief =
        modelAssetNodeDriveProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeDriveDriveBrief =
        modelAssetNodeDriveProfile.driveBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriveValueBrief =
        modelAssetNodeDriveProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeMountState =
        modelAssetNodeMountProfile.mountState;
    diagnostics.sceneRuntimeModelAssetNodeMountEntryCount =
        modelAssetNodeMountProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeMountResolvedEntryCount =
        modelAssetNodeMountProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeMountBrief =
        modelAssetNodeMountProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeMountMountBrief =
        modelAssetNodeMountProfile.mountBrief;
    diagnostics.sceneRuntimeModelAssetNodeMountValueBrief =
        modelAssetNodeMountProfile.valueBrief;
    diagnostics.sceneRuntimeModelNodeSlotState =
        sceneRuntime.modelNodeSlotProfile.slotState;
    diagnostics.sceneRuntimeModelNodeSlotCount =
        sceneRuntime.modelNodeSlotProfile.slotCount;
    diagnostics.sceneRuntimeModelNodeReadySlotCount =
        sceneRuntime.modelNodeSlotProfile.readySlotCount;
    diagnostics.sceneRuntimeModelNodeSlotBrief =
        sceneRuntime.modelNodeSlotProfile.brief;
    diagnostics.sceneRuntimeModelNodeSlotNameBrief =
        sceneRuntime.modelNodeSlotProfile.slotBrief;
    diagnostics.sceneRuntimeModelNodeRegistryState =
        sceneRuntime.modelNodeRegistryProfile.registryState;
    diagnostics.sceneRuntimeModelNodeRegistryEntryCount =
        sceneRuntime.modelNodeRegistryProfile.entryCount;
    diagnostics.sceneRuntimeModelNodeRegistryResolvedEntryCount =
        sceneRuntime.modelNodeRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelNodeRegistryBrief =
        sceneRuntime.modelNodeRegistryProfile.brief;
    diagnostics.sceneRuntimeModelNodeRegistryAssetNodeBrief =
        sceneRuntime.modelNodeRegistryProfile.assetNodeBrief;
    diagnostics.sceneRuntimeModelNodeRegistryWeightBrief =
        sceneRuntime.modelNodeRegistryProfile.weightBrief;
    diagnostics.sceneRuntimeModelAssetNodeRouteState =
        modelAssetNodeRouteProfile.routeState;
    diagnostics.sceneRuntimeModelAssetNodeRouteEntryCount =
        modelAssetNodeRouteProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeRouteResolvedEntryCount =
        modelAssetNodeRouteProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeRouteBrief =
        modelAssetNodeRouteProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeRouteRouteBrief =
        modelAssetNodeRouteProfile.routeBrief;
    diagnostics.sceneRuntimeModelAssetNodeRouteValueBrief =
        modelAssetNodeRouteProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeDispatchState =
        modelAssetNodeDispatchProfile.dispatchState;
    diagnostics.sceneRuntimeModelAssetNodeDispatchEntryCount =
        modelAssetNodeDispatchProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeDispatchResolvedEntryCount =
        modelAssetNodeDispatchProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeDispatchBrief =
        modelAssetNodeDispatchProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeDispatchDispatchBrief =
        modelAssetNodeDispatchProfile.dispatchBrief;
    diagnostics.sceneRuntimeModelAssetNodeDispatchValueBrief =
        modelAssetNodeDispatchProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeExecuteState =
        modelAssetNodeExecuteProfile.executeState;
    diagnostics.sceneRuntimeModelAssetNodeExecuteEntryCount =
        modelAssetNodeExecuteProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeExecuteResolvedEntryCount =
        modelAssetNodeExecuteProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeExecuteBrief =
        modelAssetNodeExecuteProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeExecuteExecuteBrief =
        modelAssetNodeExecuteProfile.executeBrief;
    diagnostics.sceneRuntimeModelAssetNodeExecuteValueBrief =
        modelAssetNodeExecuteProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeCommandState =
        sceneRuntime.modelAssetNodeCommandProfile.commandState;
    diagnostics.sceneRuntimeModelAssetNodeCommandEntryCount =
        sceneRuntime.modelAssetNodeCommandProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeCommandResolvedEntryCount =
        sceneRuntime.modelAssetNodeCommandProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeCommandBrief =
        sceneRuntime.modelAssetNodeCommandProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeCommandCommandBrief =
        sceneRuntime.modelAssetNodeCommandProfile.commandBrief;
    diagnostics.sceneRuntimeModelAssetNodeCommandValueBrief =
        sceneRuntime.modelAssetNodeCommandProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeControllerState =
        sceneRuntime.modelAssetNodeControllerProfile.controllerState;
    diagnostics.sceneRuntimeModelAssetNodeControllerEntryCount =
        sceneRuntime.modelAssetNodeControllerProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeControllerResolvedEntryCount =
        sceneRuntime.modelAssetNodeControllerProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeControllerBrief =
        sceneRuntime.modelAssetNodeControllerProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeControllerControllerBrief =
        sceneRuntime.modelAssetNodeControllerProfile.controllerBrief;
    diagnostics.sceneRuntimeModelAssetNodeControllerValueBrief =
        sceneRuntime.modelAssetNodeControllerProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriverState =
        sceneRuntime.modelAssetNodeDriverProfile.driverState;
    diagnostics.sceneRuntimeModelAssetNodeDriverEntryCount =
        sceneRuntime.modelAssetNodeDriverProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeDriverResolvedEntryCount =
        sceneRuntime.modelAssetNodeDriverProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeDriverBrief =
        sceneRuntime.modelAssetNodeDriverProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeDriverDriverBrief =
        sceneRuntime.modelAssetNodeDriverProfile.driverBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriverValueBrief =
        sceneRuntime.modelAssetNodeDriverProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriverRegistryState =
        sceneRuntime.modelAssetNodeDriverRegistryProfile.driverRegistryState;
    diagnostics.sceneRuntimeModelAssetNodeDriverRegistryEntryCount =
        sceneRuntime.modelAssetNodeDriverRegistryProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeDriverRegistryResolvedEntryCount =
        sceneRuntime.modelAssetNodeDriverRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeDriverRegistryBrief =
        sceneRuntime.modelAssetNodeDriverRegistryProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeDriverRegistryRegistryBrief =
        sceneRuntime.modelAssetNodeDriverRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriverRegistryValueBrief =
        sceneRuntime.modelAssetNodeDriverRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeConsumerState =
        sceneRuntime.modelAssetNodeConsumerProfile.consumerState;
    diagnostics.sceneRuntimeModelAssetNodeConsumerEntryCount =
        sceneRuntime.modelAssetNodeConsumerProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeConsumerResolvedEntryCount =
        sceneRuntime.modelAssetNodeConsumerProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeConsumerBrief =
        sceneRuntime.modelAssetNodeConsumerProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeConsumerConsumerBrief =
        sceneRuntime.modelAssetNodeConsumerProfile.consumerBrief;
    diagnostics.sceneRuntimeModelAssetNodeConsumerValueBrief =
        sceneRuntime.modelAssetNodeConsumerProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeConsumerRegistryState =
        sceneRuntime.modelAssetNodeConsumerRegistryProfile.consumerRegistryState;
    diagnostics.sceneRuntimeModelAssetNodeConsumerRegistryEntryCount =
        sceneRuntime.modelAssetNodeConsumerRegistryProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeConsumerRegistryResolvedEntryCount =
        sceneRuntime.modelAssetNodeConsumerRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeConsumerRegistryBrief =
        sceneRuntime.modelAssetNodeConsumerRegistryProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeConsumerRegistryRegistryBrief =
        sceneRuntime.modelAssetNodeConsumerRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeModelAssetNodeConsumerRegistryValueBrief =
        sceneRuntime.modelAssetNodeConsumerRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeProjectionState =
        sceneRuntime.modelAssetNodeProjectionProfile.projectionState;
    diagnostics.sceneRuntimeModelAssetNodeProjectionEntryCount =
        sceneRuntime.modelAssetNodeProjectionProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeProjectionResolvedEntryCount =
        sceneRuntime.modelAssetNodeProjectionProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeProjectionBrief =
        sceneRuntime.modelAssetNodeProjectionProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeProjectionProjectionBrief =
        sceneRuntime.modelAssetNodeProjectionProfile.projectionBrief;
    diagnostics.sceneRuntimeModelAssetNodeProjectionValueBrief =
        sceneRuntime.modelAssetNodeProjectionProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeProjectionRegistryState =
        sceneRuntime.modelAssetNodeProjectionRegistryProfile.projectionRegistryState;
    diagnostics.sceneRuntimeModelAssetNodeProjectionRegistryEntryCount =
        sceneRuntime.modelAssetNodeProjectionRegistryProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeProjectionRegistryResolvedEntryCount =
        sceneRuntime.modelAssetNodeProjectionRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeProjectionRegistryBrief =
        sceneRuntime.modelAssetNodeProjectionRegistryProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeProjectionRegistryRegistryBrief =
        sceneRuntime.modelAssetNodeProjectionRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeModelAssetNodeProjectionRegistryValueBrief =
        sceneRuntime.modelAssetNodeProjectionRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeRealizationState =
        sceneRuntime.modelAssetNodeRealizationProfile.realizationState;
    diagnostics.sceneRuntimeModelAssetNodeRealizationEntryCount =
        sceneRuntime.modelAssetNodeRealizationProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeRealizationResolvedEntryCount =
        sceneRuntime.modelAssetNodeRealizationProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeRealizationBrief =
        sceneRuntime.modelAssetNodeRealizationProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeRealizationRealizationBrief =
        sceneRuntime.modelAssetNodeRealizationProfile.realizationBrief;
    diagnostics.sceneRuntimeModelAssetNodeRealizationValueBrief =
        sceneRuntime.modelAssetNodeRealizationProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeRealizationRegistryState =
        sceneRuntime.modelAssetNodeRealizationRegistryProfile.realizationRegistryState;
    diagnostics.sceneRuntimeModelAssetNodeRealizationRegistryEntryCount =
        sceneRuntime.modelAssetNodeRealizationRegistryProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeRealizationRegistryResolvedEntryCount =
        sceneRuntime.modelAssetNodeRealizationRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeRealizationRegistryBrief =
        sceneRuntime.modelAssetNodeRealizationRegistryProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeRealizationRegistryRegistryBrief =
        sceneRuntime.modelAssetNodeRealizationRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeModelAssetNodeRealizationRegistryValueBrief =
        sceneRuntime.modelAssetNodeRealizationRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationState =
        sceneRuntime.modelAssetNodeMaterializationProfile.materializationState;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationEntryCount =
        sceneRuntime.modelAssetNodeMaterializationProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationResolvedEntryCount =
        sceneRuntime.modelAssetNodeMaterializationProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationBrief =
        sceneRuntime.modelAssetNodeMaterializationProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationMaterializationBrief =
        sceneRuntime.modelAssetNodeMaterializationProfile.materializationBrief;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationValueBrief =
        sceneRuntime.modelAssetNodeMaterializationProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationRegistryState =
        sceneRuntime.modelAssetNodeMaterializationRegistryProfile
            .materializationRegistryState;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationRegistryEntryCount =
        sceneRuntime.modelAssetNodeMaterializationRegistryProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationRegistryResolvedEntryCount =
        sceneRuntime.modelAssetNodeMaterializationRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationRegistryBrief =
        sceneRuntime.modelAssetNodeMaterializationRegistryProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationRegistryRegistryBrief =
        sceneRuntime.modelAssetNodeMaterializationRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationRegistryValueBrief =
        sceneRuntime.modelAssetNodeMaterializationRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodePresentationState =
        sceneRuntime.modelAssetNodePresentationProfile.presentationState;
    diagnostics.sceneRuntimeModelAssetNodePresentationEntryCount =
        sceneRuntime.modelAssetNodePresentationProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodePresentationResolvedEntryCount =
        sceneRuntime.modelAssetNodePresentationProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodePresentationBrief =
        sceneRuntime.modelAssetNodePresentationProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodePresentationPresentationBrief =
        sceneRuntime.modelAssetNodePresentationProfile.presentationBrief;
    diagnostics.sceneRuntimeModelAssetNodePresentationValueBrief =
        sceneRuntime.modelAssetNodePresentationProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodePresentationRegistryState =
        sceneRuntime.modelAssetNodePresentationRegistryProfile.presentationRegistryState;
    diagnostics.sceneRuntimeModelAssetNodePresentationRegistryEntryCount =
        sceneRuntime.modelAssetNodePresentationRegistryProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodePresentationRegistryResolvedEntryCount =
        sceneRuntime.modelAssetNodePresentationRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodePresentationRegistryBrief =
        sceneRuntime.modelAssetNodePresentationRegistryProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodePresentationRegistryRegistryBrief =
        sceneRuntime.modelAssetNodePresentationRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeModelAssetNodePresentationRegistryValueBrief =
        sceneRuntime.modelAssetNodePresentationRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeVisibilityState =
        sceneRuntime.modelAssetNodeVisibilityProfile.visibilityState;
    diagnostics.sceneRuntimeModelAssetNodeVisibilityEntryCount =
        sceneRuntime.modelAssetNodeVisibilityProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeVisibilityResolvedEntryCount =
        sceneRuntime.modelAssetNodeVisibilityProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeVisibilityBrief =
        sceneRuntime.modelAssetNodeVisibilityProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeVisibilityVisibilityBrief =
        sceneRuntime.modelAssetNodeVisibilityProfile.visibilityBrief;
    diagnostics.sceneRuntimeModelAssetNodeVisibilityValueBrief =
        sceneRuntime.modelAssetNodeVisibilityProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeVisibilityRegistryState =
        sceneRuntime.modelAssetNodeVisibilityRegistryProfile.visibilityRegistryState;
    diagnostics.sceneRuntimeModelAssetNodeVisibilityRegistryEntryCount =
        sceneRuntime.modelAssetNodeVisibilityRegistryProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeVisibilityRegistryResolvedEntryCount =
        sceneRuntime.modelAssetNodeVisibilityRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeVisibilityRegistryBrief =
        sceneRuntime.modelAssetNodeVisibilityRegistryProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeVisibilityRegistryRegistryBrief =
        sceneRuntime.modelAssetNodeVisibilityRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeModelAssetNodeVisibilityRegistryValueBrief =
        sceneRuntime.modelAssetNodeVisibilityRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodePresenceState =
        sceneRuntime.modelAssetNodePresenceProfile.presenceState;
    diagnostics.sceneRuntimeModelAssetNodePresenceEntryCount =
        sceneRuntime.modelAssetNodePresenceProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodePresenceResolvedEntryCount =
        sceneRuntime.modelAssetNodePresenceProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodePresenceBrief =
        sceneRuntime.modelAssetNodePresenceProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodePresencePresenceBrief =
        sceneRuntime.modelAssetNodePresenceProfile.presenceBrief;
    diagnostics.sceneRuntimeModelAssetNodePresenceValueBrief =
        sceneRuntime.modelAssetNodePresenceProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodePresenceRegistryState =
        sceneRuntime.modelAssetNodePresenceRegistryProfile.presenceRegistryState;
    diagnostics.sceneRuntimeModelAssetNodePresenceRegistryEntryCount =
        sceneRuntime.modelAssetNodePresenceRegistryProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodePresenceRegistryResolvedEntryCount =
        sceneRuntime.modelAssetNodePresenceRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodePresenceRegistryBrief =
        sceneRuntime.modelAssetNodePresenceRegistryProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodePresenceRegistryRegistryBrief =
        sceneRuntime.modelAssetNodePresenceRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeModelAssetNodePresenceRegistryValueBrief =
        sceneRuntime.modelAssetNodePresenceRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeOccupancyState =
        sceneRuntime.modelAssetNodeOccupancyProfile.occupancyState;
    diagnostics.sceneRuntimeModelAssetNodeOccupancyEntryCount =
        sceneRuntime.modelAssetNodeOccupancyProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeOccupancyResolvedEntryCount =
        sceneRuntime.modelAssetNodeOccupancyProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeOccupancyBrief =
        sceneRuntime.modelAssetNodeOccupancyProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeOccupancyOccupancyBrief =
        sceneRuntime.modelAssetNodeOccupancyProfile.occupancyBrief;
    diagnostics.sceneRuntimeModelAssetNodeOccupancyValueBrief =
        sceneRuntime.modelAssetNodeOccupancyProfile.valueBrief;
    diagnostics.sceneRuntimeModelAssetNodeOccupancyRegistryState =
        sceneRuntime.modelAssetNodeOccupancyRegistryProfile.occupancyRegistryState;
    diagnostics.sceneRuntimeModelAssetNodeOccupancyRegistryEntryCount =
        sceneRuntime.modelAssetNodeOccupancyRegistryProfile.entryCount;
    diagnostics.sceneRuntimeModelAssetNodeOccupancyRegistryResolvedEntryCount =
        sceneRuntime.modelAssetNodeOccupancyRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeOccupancyRegistryBrief =
        sceneRuntime.modelAssetNodeOccupancyRegistryProfile.brief;
    diagnostics.sceneRuntimeModelAssetNodeOccupancyRegistryRegistryBrief =
        sceneRuntime.modelAssetNodeOccupancyRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeModelAssetNodeOccupancyRegistryValueBrief =
        sceneRuntime.modelAssetNodeOccupancyRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeBindingState =
        sceneRuntime.assetNodeBindingProfile.bindingState;
    diagnostics.sceneRuntimeAssetNodeBindingEntryCount =
        sceneRuntime.assetNodeBindingProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeBindingResolvedEntryCount =
        sceneRuntime.assetNodeBindingProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeBindingBrief =
        sceneRuntime.assetNodeBindingProfile.brief;
    diagnostics.sceneRuntimeAssetNodeBindingPathBrief =
        sceneRuntime.assetNodeBindingProfile.pathBrief;
    diagnostics.sceneRuntimeAssetNodeBindingWeightBrief =
        sceneRuntime.assetNodeBindingProfile.weightBrief;
    diagnostics.sceneRuntimeAssetNodeTransformState =
        sceneRuntime.assetNodeTransformProfile.transformState;
    diagnostics.sceneRuntimeAssetNodeTransformEntryCount =
        sceneRuntime.assetNodeTransformProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeTransformResolvedEntryCount =
        sceneRuntime.assetNodeTransformProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeTransformBrief =
        sceneRuntime.assetNodeTransformProfile.brief;
    diagnostics.sceneRuntimeAssetNodeTransformPathBrief =
        sceneRuntime.assetNodeTransformProfile.pathBrief;
    diagnostics.sceneRuntimeAssetNodeTransformValueBrief =
        sceneRuntime.assetNodeTransformProfile.transformBrief;
    diagnostics.sceneRuntimeAssetNodeAnchorState = anchorProfile.anchorState;
    diagnostics.sceneRuntimeAssetNodeAnchorEntryCount = anchorProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeAnchorResolvedEntryCount =
        anchorProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeAnchorBrief = anchorProfile.brief;
    diagnostics.sceneRuntimeAssetNodeAnchorPointBrief = anchorProfile.pointBrief;
    diagnostics.sceneRuntimeAssetNodeAnchorScaleBrief = anchorProfile.scaleBrief;
    diagnostics.sceneRuntimeAssetNodeResolverState = resolverProfile.resolverState;
    diagnostics.sceneRuntimeAssetNodeResolverEntryCount = resolverProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeResolverResolvedEntryCount =
        resolverProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeResolverBrief = resolverProfile.brief;
    diagnostics.sceneRuntimeAssetNodeResolverParentBrief = resolverProfile.parentBrief;
    diagnostics.sceneRuntimeAssetNodeResolverValueBrief =
        resolverProfile.localTransformBrief;
    diagnostics.sceneRuntimeAssetNodeParentSpaceState = parentSpaceProfile.parentSpaceState;
    diagnostics.sceneRuntimeAssetNodeParentSpaceEntryCount = parentSpaceProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeParentSpaceResolvedEntryCount =
        parentSpaceProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeParentSpaceBrief = parentSpaceProfile.brief;
    diagnostics.sceneRuntimeAssetNodeParentSpaceParentBrief = parentSpaceProfile.parentBrief;
    diagnostics.sceneRuntimeAssetNodeParentSpaceValueBrief = parentSpaceProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeTargetState = targetProfile.targetState;
    diagnostics.sceneRuntimeAssetNodeTargetEntryCount = targetProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeTargetResolvedEntryCount = targetProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeTargetBrief = targetProfile.brief;
    diagnostics.sceneRuntimeAssetNodeTargetKindBrief = targetProfile.kindBrief;
    diagnostics.sceneRuntimeAssetNodeTargetValueBrief = targetProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeTargetResolverState = targetResolverProfile.resolverState;
    diagnostics.sceneRuntimeAssetNodeTargetResolverEntryCount = targetResolverProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeTargetResolverResolvedEntryCount =
        targetResolverProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeTargetResolverBrief = targetResolverProfile.brief;
    diagnostics.sceneRuntimeAssetNodeTargetResolverPathBrief =
        targetResolverProfile.pathBrief;
    diagnostics.sceneRuntimeAssetNodeTargetResolverValueBrief =
        targetResolverProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceState = worldSpaceProfile.worldSpaceState;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceEntryCount = worldSpaceProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceResolvedEntryCount =
        worldSpaceProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceBrief = worldSpaceProfile.brief;
    diagnostics.sceneRuntimeAssetNodeWorldSpacePathBrief =
        worldSpaceProfile.pathBrief;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceValueBrief =
        worldSpaceProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodePoseState = poseProfile.poseState;
    diagnostics.sceneRuntimeAssetNodePoseEntryCount = poseProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodePoseResolvedEntryCount =
        poseProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseBrief = poseProfile.brief;
    diagnostics.sceneRuntimeAssetNodePosePathBrief = poseProfile.pathBrief;
    diagnostics.sceneRuntimeAssetNodePoseValueBrief = poseProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodePoseResolverState =
        poseResolverProfile.resolverState;
    diagnostics.sceneRuntimeAssetNodePoseResolverEntryCount =
        poseResolverProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodePoseResolverResolvedEntryCount =
        poseResolverProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseResolverBrief =
        poseResolverProfile.brief;
    diagnostics.sceneRuntimeAssetNodePoseResolverPathBrief =
        poseResolverProfile.pathBrief;
    diagnostics.sceneRuntimeAssetNodePoseResolverValueBrief =
        poseResolverProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodePoseRegistryState =
        poseRegistryProfile.registryState;
    diagnostics.sceneRuntimeAssetNodePoseRegistryEntryCount =
        poseRegistryProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodePoseRegistryResolvedEntryCount =
        poseRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseRegistryBrief =
        poseRegistryProfile.brief;
    diagnostics.sceneRuntimeAssetNodePoseRegistryNodeBrief =
        poseRegistryProfile.poseNodeBrief;
    diagnostics.sceneRuntimeAssetNodePoseRegistryWeightBrief =
        poseRegistryProfile.weightBrief;
    diagnostics.sceneRuntimeAssetNodePoseChannelState =
        poseChannelProfile.channelState;
    diagnostics.sceneRuntimeAssetNodePoseChannelEntryCount =
        poseChannelProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodePoseChannelResolvedEntryCount =
        poseChannelProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseChannelBrief =
        poseChannelProfile.brief;
    diagnostics.sceneRuntimeAssetNodePoseChannelNameBrief =
        poseChannelProfile.channelBrief;
    diagnostics.sceneRuntimeAssetNodePoseChannelWeightBrief =
        poseChannelProfile.weightBrief;
    diagnostics.sceneRuntimeAssetNodePoseConstraintState =
        poseConstraintProfile.constraintState;
    diagnostics.sceneRuntimeAssetNodePoseConstraintEntryCount =
        poseConstraintProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodePoseConstraintResolvedEntryCount =
        poseConstraintProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseConstraintBrief =
        poseConstraintProfile.brief;
    diagnostics.sceneRuntimeAssetNodePoseConstraintNameBrief =
        poseConstraintProfile.constraintBrief;
    diagnostics.sceneRuntimeAssetNodePoseConstraintValueBrief =
        poseConstraintProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodePoseSolveState =
        poseSolveProfile.solveState;
    diagnostics.sceneRuntimeAssetNodePoseSolveEntryCount =
        poseSolveProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodePoseSolveResolvedEntryCount =
        poseSolveProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseSolveBrief =
        poseSolveProfile.brief;
    diagnostics.sceneRuntimeAssetNodePoseSolvePathBrief =
        poseSolveProfile.pathBrief;
    diagnostics.sceneRuntimeAssetNodePoseSolveValueBrief =
        poseSolveProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeJointHintState =
        jointHintProfile.hintState;
    diagnostics.sceneRuntimeAssetNodeJointHintEntryCount =
        jointHintProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeJointHintResolvedEntryCount =
        jointHintProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeJointHintBrief =
        jointHintProfile.brief;
    diagnostics.sceneRuntimeAssetNodeJointHintNameBrief =
        jointHintProfile.jointHintBrief;
    diagnostics.sceneRuntimeAssetNodeJointHintValueBrief =
        jointHintProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeArticulationState =
        articulationProfile.articulationState;
    diagnostics.sceneRuntimeAssetNodeArticulationEntryCount =
        articulationProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeArticulationResolvedEntryCount =
        articulationProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeArticulationBrief =
        articulationProfile.brief;
    diagnostics.sceneRuntimeAssetNodeArticulationNameBrief =
        articulationProfile.articulationBrief;
    diagnostics.sceneRuntimeAssetNodeArticulationValueBrief =
        articulationProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryState =
        localJointRegistryProfile.registryState;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryEntryCount =
        localJointRegistryProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryResolvedEntryCount =
        localJointRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryBrief =
        localJointRegistryProfile.brief;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryJointBrief =
        localJointRegistryProfile.localJointBrief;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryWeightBrief =
        localJointRegistryProfile.weightBrief;
    diagnostics.sceneRuntimeAssetNodeArticulationMapState =
        articulationMapProfile.mapState;
    diagnostics.sceneRuntimeAssetNodeArticulationMapEntryCount =
        articulationMapProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeArticulationMapResolvedEntryCount =
        articulationMapProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeArticulationMapBrief =
        articulationMapProfile.brief;
    diagnostics.sceneRuntimeAssetNodeArticulationMapNameBrief =
        articulationMapProfile.mapBrief;
    diagnostics.sceneRuntimeAssetNodeArticulationMapValueBrief =
        articulationMapProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeControlRigHintState =
        controlRigHintProfile.hintState;
    diagnostics.sceneRuntimeAssetNodeControlRigHintEntryCount =
        controlRigHintProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeControlRigHintResolvedEntryCount =
        controlRigHintProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControlRigHintBrief =
        controlRigHintProfile.brief;
    diagnostics.sceneRuntimeAssetNodeControlRigHintNameBrief =
        controlRigHintProfile.rigHintBrief;
    diagnostics.sceneRuntimeAssetNodeControlRigHintValueBrief =
        controlRigHintProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeRigChannelState =
        rigChannelProfile.channelState;
    diagnostics.sceneRuntimeAssetNodeRigChannelEntryCount =
        rigChannelProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeRigChannelResolvedEntryCount =
        rigChannelProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeRigChannelBrief =
        rigChannelProfile.brief;
    diagnostics.sceneRuntimeAssetNodeRigChannelNameBrief =
        rigChannelProfile.channelBrief;
    diagnostics.sceneRuntimeAssetNodeRigChannelValueBrief =
        rigChannelProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceState =
        controlSurfaceProfile.surfaceState;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceEntryCount =
        controlSurfaceProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceResolvedEntryCount =
        controlSurfaceProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceBrief =
        controlSurfaceProfile.brief;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceNameBrief =
        controlSurfaceProfile.surfaceBrief;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceValueBrief =
        controlSurfaceProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeRigDriverState =
        rigDriverProfile.driverState;
    diagnostics.sceneRuntimeAssetNodeRigDriverEntryCount =
        rigDriverProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeRigDriverResolvedEntryCount =
        rigDriverProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeRigDriverBrief =
        rigDriverProfile.brief;
    diagnostics.sceneRuntimeAssetNodeRigDriverNameBrief =
        rigDriverProfile.driverBrief;
    diagnostics.sceneRuntimeAssetNodeRigDriverValueBrief =
        rigDriverProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverState =
        surfaceDriverProfile.driverState;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverEntryCount =
        surfaceDriverProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverResolvedEntryCount =
        surfaceDriverProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverBrief =
        surfaceDriverProfile.brief;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverNameBrief =
        surfaceDriverProfile.driverBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverValueBrief =
        surfaceDriverProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodePoseBusState = poseBusProfile.busState;
    diagnostics.sceneRuntimeAssetNodePoseBusEntryCount = poseBusProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodePoseBusResolvedEntryCount =
        poseBusProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseBusBrief = poseBusProfile.brief;
    diagnostics.sceneRuntimeAssetNodePoseBusNameBrief = poseBusProfile.busBrief;
    diagnostics.sceneRuntimeAssetNodePoseBusValueBrief = poseBusProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeControllerTableState =
        controllerTableProfile.tableState;
    diagnostics.sceneRuntimeAssetNodeControllerTableEntryCount =
        controllerTableProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeControllerTableResolvedEntryCount =
        controllerTableProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerTableBrief =
        controllerTableProfile.brief;
    diagnostics.sceneRuntimeAssetNodeControllerTableNameBrief =
        controllerTableProfile.controllerBrief;
    diagnostics.sceneRuntimeAssetNodeControllerTableValueBrief =
        controllerTableProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeControllerRegistryState =
        controllerRegistryProfile.registryState;
    diagnostics.sceneRuntimeAssetNodeControllerRegistryEntryCount =
        controllerRegistryProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeControllerRegistryResolvedEntryCount =
        controllerRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerRegistryBrief =
        controllerRegistryProfile.brief;
    diagnostics.sceneRuntimeAssetNodeControllerRegistryNameBrief =
        controllerRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeAssetNodeControllerRegistryValueBrief =
        controllerRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeDriverBusState = driverBusProfile.busState;
    diagnostics.sceneRuntimeAssetNodeDriverBusEntryCount =
        driverBusProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeDriverBusResolvedEntryCount =
        driverBusProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeDriverBusBrief = driverBusProfile.brief;
    diagnostics.sceneRuntimeAssetNodeDriverBusNameBrief =
        driverBusProfile.driverBusBrief;
    diagnostics.sceneRuntimeAssetNodeDriverBusValueBrief =
        driverBusProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryState =
        controllerDriverRegistryProfile.registryState;
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryEntryCount =
        controllerDriverRegistryProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryResolvedEntryCount =
        controllerDriverRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryBrief =
        controllerDriverRegistryProfile.brief;
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryNameBrief =
        controllerDriverRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryValueBrief =
        controllerDriverRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionLaneState =
        executionLaneProfile.laneState;
    diagnostics.sceneRuntimeAssetNodeExecutionLaneEntryCount =
        executionLaneProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionLaneResolvedEntryCount =
        executionLaneProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionLaneBrief =
        executionLaneProfile.brief;
    diagnostics.sceneRuntimeAssetNodeExecutionLaneNameBrief =
        executionLaneProfile.laneBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionLaneValueBrief =
        executionLaneProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseState =
        controllerPhaseProfile.phaseState;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseEntryCount =
        controllerPhaseProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseResolvedEntryCount =
        controllerPhaseProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseBrief =
        controllerPhaseProfile.brief;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseNameBrief =
        controllerPhaseProfile.phaseBrief;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseValueBrief =
        controllerPhaseProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceState =
        executionSurfaceProfile.surfaceState;
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceEntryCount =
        executionSurfaceProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceResolvedEntryCount =
        executionSurfaceProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceBrief =
        executionSurfaceProfile.brief;
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceNameBrief =
        executionSurfaceProfile.surfaceBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceValueBrief =
        executionSurfaceProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryState =
        controllerPhaseRegistryProfile.registryState;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryEntryCount =
        controllerPhaseRegistryProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryResolvedEntryCount =
        controllerPhaseRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryBrief =
        controllerPhaseRegistryProfile.brief;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryNameBrief =
        controllerPhaseRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryValueBrief =
        controllerPhaseRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusState =
        surfaceCompositionBusProfile.busState;
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusEntryCount =
        surfaceCompositionBusProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusResolvedEntryCount =
        surfaceCompositionBusProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusBrief =
        surfaceCompositionBusProfile.brief;
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusNameBrief =
        surfaceCompositionBusProfile.busBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusValueBrief =
        surfaceCompositionBusProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackState =
        executionStackProfile.stackState;
    diagnostics.sceneRuntimeAssetNodeExecutionStackEntryCount =
        executionStackProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionStackResolvedEntryCount =
        executionStackProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionStackBrief =
        executionStackProfile.brief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackNameBrief =
        executionStackProfile.stackBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackValueBrief =
        executionStackProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterState =
        executionStackRouterProfile.routerState;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterEntryCount =
        executionStackRouterProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterResolvedEntryCount =
        executionStackRouterProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterBrief =
        executionStackRouterProfile.brief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterNameBrief =
        executionStackRouterProfile.routerBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterValueBrief =
        executionStackRouterProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryState =
        executionStackRouterRegistryProfile.registryState;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryEntryCount =
        executionStackRouterRegistryProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryResolvedEntryCount =
        executionStackRouterRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryBrief =
        executionStackRouterRegistryProfile.brief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryNameBrief =
        executionStackRouterRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryValueBrief =
        executionStackRouterRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryState =
        compositionRegistryProfile.registryState;
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryEntryCount =
        compositionRegistryProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryResolvedEntryCount =
        compositionRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryBrief =
        compositionRegistryProfile.brief;
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryNameBrief =
        compositionRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryValueBrief =
        compositionRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteState =
        surfaceRouteProfile.routeState;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteEntryCount =
        surfaceRouteProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteResolvedEntryCount =
        surfaceRouteProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBrief =
        surfaceRouteProfile.brief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteNameBrief =
        surfaceRouteProfile.routeBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteValueBrief =
        surfaceRouteProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryState =
        surfaceRouteRegistryProfile.registryState;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryEntryCount =
        surfaceRouteRegistryProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryResolvedEntryCount =
        surfaceRouteRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryBrief =
        surfaceRouteRegistryProfile.brief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryNameBrief =
        surfaceRouteRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryValueBrief =
        surfaceRouteRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusState =
        surfaceRouteRouterBusProfile.busState;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusEntryCount =
        surfaceRouteRouterBusProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusResolvedEntryCount =
        surfaceRouteRouterBusProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusBrief =
        surfaceRouteRouterBusProfile.brief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusNameBrief =
        surfaceRouteRouterBusProfile.busBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusValueBrief =
        surfaceRouteRouterBusProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryState =
        surfaceRouteBusRegistryProfile.registryState;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryEntryCount =
        surfaceRouteBusRegistryProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryResolvedEntryCount =
        surfaceRouteBusRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryBrief =
        surfaceRouteBusRegistryProfile.brief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryNameBrief =
        surfaceRouteBusRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryValueBrief =
        surfaceRouteBusRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverState =
        surfaceRouteBusDriverProfile.driverState;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverEntryCount =
        surfaceRouteBusDriverProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverResolvedEntryCount =
        surfaceRouteBusDriverProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverBrief =
        surfaceRouteBusDriverProfile.brief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverNameBrief =
        surfaceRouteBusDriverProfile.driverBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverValueBrief =
        surfaceRouteBusDriverProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryState =
        surfaceRouteBusDriverRegistryProfile.registryState;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryEntryCount =
        surfaceRouteBusDriverRegistryProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryResolvedEntryCount =
        surfaceRouteBusDriverRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryBrief =
        surfaceRouteBusDriverRegistryProfile.brief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryNameBrief =
        surfaceRouteBusDriverRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryValueBrief =
        surfaceRouteBusDriverRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterState =
        surfaceRouteBusDriverRegistryRouterProfile.routerState;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterEntryCount =
        surfaceRouteBusDriverRegistryRouterProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterResolvedEntryCount =
        surfaceRouteBusDriverRegistryRouterProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterBrief =
        surfaceRouteBusDriverRegistryRouterProfile.brief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterNameBrief =
        surfaceRouteBusDriverRegistryRouterProfile.routerBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterValueBrief =
        surfaceRouteBusDriverRegistryRouterProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableState =
        executionDriverTableProfile.tableState;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableEntryCount =
        executionDriverTableProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableResolvedEntryCount =
        executionDriverTableProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableBrief =
        executionDriverTableProfile.brief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableNameBrief =
        executionDriverTableProfile.driverBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableValueBrief =
        executionDriverTableProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableState =
        executionDriverRouterTableProfile.tableState;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableEntryCount =
        executionDriverRouterTableProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableResolvedEntryCount =
        executionDriverRouterTableProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableBrief =
        executionDriverRouterTableProfile.brief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableNameBrief =
        executionDriverRouterTableProfile.routerBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableValueBrief =
        executionDriverRouterTableProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryState =
        executionDriverRouterRegistryProfile.registryState;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryEntryCount =
        executionDriverRouterRegistryProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryResolvedEntryCount =
        executionDriverRouterRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBrief =
        executionDriverRouterRegistryProfile.brief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryNameBrief =
        executionDriverRouterRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryValueBrief =
        executionDriverRouterRegistryProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusState =
        executionDriverRouterRegistryBusProfile.busState;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusEntryCount =
        executionDriverRouterRegistryBusProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusResolvedEntryCount =
        executionDriverRouterRegistryBusProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusBrief =
        executionDriverRouterRegistryBusProfile.brief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusNameBrief =
        executionDriverRouterRegistryBusProfile.busBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusValueBrief =
        executionDriverRouterRegistryBusProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryState =
        executionDriverRouterRegistryBusRegistryProfile.registryState;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryEntryCount =
        executionDriverRouterRegistryBusRegistryProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryResolvedEntryCount =
        executionDriverRouterRegistryBusRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryBrief =
        executionDriverRouterRegistryBusRegistryProfile.brief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryNameBrief =
        executionDriverRouterRegistryBusRegistryProfile.registryBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryValueBrief =
        executionDriverRouterRegistryBusRegistryProfile.valueBrief;
    const auto& poseAdapterProfile = sceneRuntime.poseAdapterProfile;
    diagnostics.sceneRuntimePoseAdapterInfluence = poseAdapterProfile.influence;
    diagnostics.sceneRuntimePoseReadabilityBias = poseAdapterProfile.readabilityBias;
    diagnostics.sceneRuntimePoseAdapterBrief = poseAdapterProfile.brief;
    diagnostics.facingDirection = sceneRuntime.facingSign >= 0.0f ? 1 : -1;
    diagnostics.surfaceWidth = width;
    diagnostics.surfaceHeight = height;
    diagnostics.modelSourceFormat = resources.modelSourceFormat;
    diagnostics.appearanceSkinVariantId = resources.appearanceProfileSkinVariantId;
    diagnostics.appearanceAccessoryIds = resources.appearanceAccessoryIds;
    const auto accessoryFamily =
        ResolveWin32MouseCompanionRealRendererAppearanceAccessoryFamily(
            resources.appearanceAccessoryIds);
    diagnostics.appearanceAccessoryFamily =
        ToStringWin32MouseCompanionRealRendererAppearanceAccessoryFamily(accessoryFamily);
    diagnostics.appearanceComboPreset =
        ToStringWin32MouseCompanionRealRendererAppearanceComboPreset(
            ResolveWin32MouseCompanionRealRendererAppearanceComboPreset(
                resources.appearanceProfileSkinVariantId,
                accessoryFamily));
    if (pluginSelection.comboPresetOverride !=
        Win32MouseCompanionRealRendererAppearanceComboPreset::None) {
        diagnostics.appearanceComboPreset =
            ToStringWin32MouseCompanionRealRendererAppearanceComboPreset(
                pluginSelection.comboPresetOverride);
    }
    diagnostics.appearanceRequestedPresetId = resources.appearanceRequestedPresetId;
    diagnostics.appearanceResolvedPresetId = resources.appearanceResolvedPresetId;
    diagnostics.appearancePluginId = pluginSelection.pluginId;
    diagnostics.appearancePluginKind = pluginSelection.pluginKind;
    diagnostics.appearancePluginSource = pluginSelection.pluginSource;
    diagnostics.appearancePluginSelectionReason = pluginSelection.selectionReason;
    diagnostics.appearancePluginFailureReason = pluginSelection.failureReason;
    diagnostics.appearancePluginManifestPath = pluginSelection.manifestPath;
    diagnostics.appearancePluginRuntimeBackend = pluginSelection.runtimeBackend;
    diagnostics.appearancePluginMetadataPath = pluginSelection.metadataPath;
    diagnostics.appearancePluginMetadataSchemaVersion =
        pluginSelection.metadataSchemaVersion;
    diagnostics.appearancePluginAppearanceSemanticsMode =
        pluginSelection.appearanceSemanticsMode;
    diagnostics.appearancePluginSampleTier = pluginSelection.declaredSampleTier;
    diagnostics.defaultLaneCandidate = pluginSelection.defaultLaneCandidate;
    diagnostics.defaultLaneSource = pluginSelection.defaultLaneSource;
    diagnostics.defaultLaneRolloutStatus = pluginSelection.defaultLaneRolloutStatus;
    diagnostics.defaultLaneStyleIntent = pluginSelection.defaultLaneStyleIntent;
    diagnostics.defaultLaneCandidateTier = pluginSelection.defaultLaneCandidateTier;
    diagnostics.appearancePluginContractBrief =
        BuildWin32MouseCompanionRenderPluginContractBrief(
            diagnostics.appearancePluginAppearanceSemanticsMode,
            diagnostics.defaultLaneStyleIntent,
            diagnostics.appearancePluginSampleTier);
    #endif
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    diagnostics.renderedFrameCount = runtimeDiagnostics_.renderedFrameCount + 1;
    runtimeDiagnostics_ = std::move(diagnostics);
}

Win32MouseCompanionRendererBackendRuntimeDiagnostics
Win32MouseCompanionRealRendererBackend::ReadRuntimeDiagnostics() const {
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    return runtimeDiagnostics_;
}

void RegisterWin32MouseCompanionRealRendererBackend() {
    static Win32MouseCompanionRendererBackendRegistrar<Win32MouseCompanionRealRendererBackend> registrar(
        "real",
        200,
        &EvaluateWin32MouseCompanionRealRendererAvailability);
    (void)registrar;
}

} // namespace mousefx::windows
